#include "Server.hpp"

#include "../Job.hpp"

#define HTTP_CREATED 201

Server::Server(std::string ip, const std::uint16_t port, Config &config, httplib::SSLClient &gitlab_client)
    : ip(std::move(ip)), port(port), config(config), gitlab_client(gitlab_client), job_manager(JobManager()) {
    Post("/retry", [&config, this](const httplib::Request &req, httplib::Response &) {
        const auto bot_username_opt = config.get_value<std::string>("bot_username");
        if (!bot_username_opt.has_value()) {
            spdlog::error("bot username not found.");
            return;
        }

        if (!json::accept(req.body)) {
            spdlog::error("failed to parse request body.");
            return;
        }

        const auto &req_body = json::parse(req.body);
        const auto &object_kind = req_body.at("object_kind").get<std::string>();

        if (object_kind == "build")
            handle_job_webhook(req_body);
        else
            handle_comment_webhook(req_body, bot_username_opt.value());
    });
}

Server::~Server() {
    stop();
    delete &ip;
    delete &port;
    delete &config;
    delete &gitlab_client;
}

void Server::start() {
    spdlog::info("Server is now running on: {}:{}", ip, port);
    listen(ip, port);
}

bool Server::retry_job(const Job &job) const {
    httplib::Result retry_job_res = this->gitlab_client.Post(
        std::format("/api/v4/projects/{}/jobs/{}/retry", job.get_project_id(), job.get_id())
    );

    return retry_job_res->status == HTTP_CREATED;
}

std::optional<Job> Server::get_job_by_name(const json &jobs, const std::string &job_name, const json &req_body) {
    for (const auto &other_job: jobs) {
        if (other_job.at("name") == job_name) {
            const int &pipeline_id = other_job.at("pipeline").at("id").get<int>();
            Job job = job_manager.get_or_create(pipeline_id, other_job, req_body);

            if (const auto &status = other_job.at("status").get<std::string>(); status == "success")
                job.set_status(Job::Status::SUCCESS);
            else if (status == "failed")
                job.set_status(Job::Status::FAILED);

            job_manager.add_job(pipeline_id, job);
            return std::make_optional(job);
        }
    }

    return std::nullopt;
}

void Server::handle_comment_webhook(const json &req_body, const std::string &bot_username) {
    const auto &note = req_body.at("object_attributes").at("note").get<std::string>();

    if (!note.contains(BOT_MENTION_PERFIX + bot_username)) {
        spdlog::error("user did not mention the bot");
        return;
    }

    const int &project_id = req_body.at("project_id");
    const int &pipeline_id = req_body.at("merge_request").at("head_pipeline_id");

    httplib::Result jobs = this->gitlab_client.Get(
        std::format("/api/v4/projects/{}/pipelines/{}/jobs", project_id, pipeline_id)
    );

    const auto &jobs_json = json::parse(jobs->body);

    const auto &job_name = config.get_value<std::string>("job_name");
    if (!job_name.has_value()) {
        spdlog::error("job name not found");
        return;
    }

    std::optional<Job> job_opt = get_job_by_name(jobs_json, job_name.value(), req_body);

    if (!job_opt.has_value()) {
        spdlog::error("requested job {} not found.", job_name.value());
        return;
    }

    Job &job = job_opt.value();
    if (job.get_status() == Job::Status::FAILED)
        return;

    if (retry_job(job))
        spdlog::info("Retried job {} for {}x times", job_name.value(), job.get_retry_amount());
    else
        spdlog::error("Failed to retry job {}", job_name.value());

    job.increase_retry_amount(1);
}

void Server::handle_job_webhook(const json &req_body) {
    const auto &job_name = config.get_value<std::string>("job_name");
    if (!job_name.has_value()) {
        spdlog::error("job name not found.");
        return;
    }

    if (req_body.at("build_name").get<std::string>() != job_name.value())
        return;

    const int &job_id = req_body.at("build_id").get<int>();

    const int &pipeline_id = req_body.at("pipeline_id").get<int>();
    Job &job = job_manager.get_job(pipeline_id);
    job.set_id(job_id);
    job.set_project_id(req_body.at("project_id").get<int>());

    const auto &status = req_body.at("build_status").get<std::string>();

    if (status == "created")
        job.set_status(Job::Status::CREATED);
    else if (status == "pending")
        job.set_status(Job::Status::PENDING);
    else if (status == "running")
        job.set_status(Job::Status::RUNNING);
    else if (status == "success")
        job.set_status(Job::Status::SUCCESS);
    else if (status == "failed")
        job.set_status(Job::Status::FAILED);

    if (job.get_status() == Job::Status::FAILED) {
        spdlog::error("job {} failed! terminating..", job_name.value());
        return;
    }

    if (job.get_status() != Job::Status::SUCCESS)
        return;

    const int requested_retry_amount = config.get_value<int>("retry_amount").value();

    if (requested_retry_amount <= 0) {
        spdlog::error("retry amount must be greater than 0.");
        return;
    }

    if (job.get_retry_amount() >= requested_retry_amount) {
        spdlog::info("job retry_amount reached! terminating with success!");
        job_manager.remove_job(pipeline_id);
        return;
    }

    job.set_name(job_name.value());

    if (retry_job(job))
        spdlog::info("Retried job {} for {}x times", job.get_name(), job.get_retry_amount());
    else
        spdlog::error("Failed to retry job {}", job.get_name());

    job.increase_retry_amount(1);
}