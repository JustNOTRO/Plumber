#include "Server.hpp"
#include "../managers/Job.hpp"

std::string require_env(const char *name) {
    const char *v = std::getenv(name);
    if (!v) {
        spdlog::error("{} is not set", name);
        std::exit(1);
    }

    return v;
}

std::uint16_t require_port(const char *name) {
    std::string v = require_env(name);

    try {
        return std::stoul(v);
    } catch (std::exception &e) {
        spdlog::error("could not parse {}, {}", v, e.what());
        std::exit(1);
    }
}

std::string require_http_format(const char *name) {
    std::string env = require_env(name);
    if (!env.starts_with("http://") && !env.starts_with("https://")) {
        spdlog::error("could not parse {}, invalid http format", name);
        std::exit(1);
    }

    return env;
}

std::expected<int, std::string> require_retry_amount(const char* name) {
    std::string env = require_env(name);

    try {
        return std::stoi(env);
    } catch (std::exception &e) {
        return std::unexpected<std::string>(e.what());
    }
}

void handle_exit_signal(int __attribute__((unused)) signal) {
    spdlog::info("Stopping the server..");
    std::exit(0);
}

Server::Server()
    : ip(require_env("SERVER_IP")),
      port(require_port("SERVER_PORT")),
      gitlab_client(httplib::Client(require_http_format("GITLAB_INSTANCE"))) {}

void Server::setup_gitlab_client() {
    std::string gitlab_access_token = require_env("GITLAB_ACCESS_TOKEN");

    gitlab_client.set_default_headers({{"PRIVATE-TOKEN", gitlab_access_token}});
    gitlab_client.set_error_logger([](const httplib::Error &err, const httplib::Request *req) {
        if (req)
            spdlog::error("{} {}", req->method, req->path);

        spdlog::error("failed: {}", httplib::to_string(err));

        switch (err) {
            case httplib::Error::Connection:
                spdlog::error("(verify server is running and reachable)");
                break;
            case httplib::Error::SSLConnection:
                spdlog::error(" (check SSL certificate and TLS configuration)");
                break;
            case httplib::Error::ConnectionTimeout:
                spdlog::error(" (increase timeout or check network latency)");
                break;
            case httplib::Error::Read:
                spdlog::error(" (server may have closed connection prematurely)");
                break;
            default:
                break;
        }
    });
}

void Server::start() {
    if (!bind_to_port(ip, port)) {
        spdlog::error("Could not bind address {} to port: {}", ip, port);
        std::exit(1);
    }

    setup_gitlab_client();

    Post("/webhook", [this](const httplib::Request &req, httplib::Response &) {
        const auto bot_username = require_env("BOT_USERNAME");

        if (!nlohmann::json::accept(req.body)) {
            spdlog::error("failed to parse request body.");
            return;
        }

        const auto job_name = require_env("JOB_NAME");
        const auto req_body = nlohmann::json::parse(req.body);

        if (const auto object_kind = req_body.at("object_kind").get<std::string>(); object_kind == "build")
            handle_job_webhook(req_body, job_name);
        else
            handle_comment_webhook(req_body, bot_username, job_name);
    });

    spdlog::info("Server is now running on: {}:{}", ip, port);
    std::signal(SIGTERM, handle_exit_signal);

    listen(ip, port);
}

void Server::retry_job(const Job &job) {
    // todo fix the path here
    const std::string path = std::format("/api/v4/projects/{}/jobs/{}/retry/t", job.get_project_id(), job.get_id());
    const httplib::Result res = this->gitlab_client.Post(path);
}

std::optional<Job> Server::get_job_by_name(const std::string &job_name, const nlohmann::json &req_body) {
    const int &project_id = req_body.at("project_id");
    const int &pipeline_id = req_body.at("merge_request").at("head_pipeline_id");

    const std::optional<nlohmann::json> jobs_opt = get_pipeline_jobs(project_id, pipeline_id);

    if (!jobs_opt.has_value())
        return std::nullopt;

    for (const auto &jobs = jobs_opt.value(); const auto &other_job: jobs) {
        if (other_job.at("name") != job_name)
            continue;

        Job job = job_manager.create_job(pipeline_id, other_job);

        if (const auto &status = other_job.at("status").get<std::string>(); status == "success")
            job.set_status(Job::Status::SUCCESS);
        else if (status == "failed")
            job.set_status(Job::Status::FAILED);

        return std::make_optional(job);
    }

    return std::nullopt;
}

nlohmann::basic_json<> require_node(const nlohmann::json &json, const std::string &name) {
    if (!json.contains(name)) {
        spdlog::error("could not find {}", name);
        std::exit(1);
    }

    return json.at(name);
}

void Server::handle_comment_webhook(const nlohmann::json &req_body, const std::string &bot_username, const std::string &job_name) {
    const auto object_attributes = require_node(req_body, "object_attributes");
    const auto noteable_type = require_node(object_attributes, "noteable_type").get<std::string>();

    if (noteable_type != "MergeRequest")
        return;

    const auto note = require_node(object_attributes, "note").get<std::string>();
    if (!note.contains(BOT_MENTION_PERFIX + bot_username))
        return;

    std::optional<Job> job_opt = get_job_by_name(job_name, req_body);

    if (!job_opt) {
        spdlog::error("requested job {} not found.", job_name);
        return;
    }

    Job &job = job_opt.value();
    if (job.get_status() == Job::Status::FAILED)
        return;

    retry_job(job);
    job.increase_retry_amount();
}

void Server::handle_job_webhook(const nlohmann::json &req_body, const std::string &job_name) {
    const auto build_name = require_node(req_body, "build_name").get<std::string>();
    if (build_name != job_name)
        return;

    const int &job_id = req_body.at("build_id").get<int>();
    const int &pipeline_id = req_body.at("pipeline_id").get<int>();

    auto expected_job = job_manager.get_job(pipeline_id);
    if (!expected_job.has_value()) {
        spdlog::error("failed to retrieve job, {}", expected_job.error());
        return;
    }

    Job &job = expected_job.value();
    job.set_id(job_id);

    if (const auto &status = req_body.at("build_status").get<std::string>(); status == "created")
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
        job_manager.remove_job(pipeline_id);
        spdlog::error("job {} failed! terminating..", job_name);
        return;
    }

    if (job.get_status() != Job::Status::SUCCESS)
        return;

    const std::expected<int, std::string> expected_retry_amount = require_retry_amount("RETRY_AMOUNT");
    if (!expected_retry_amount.has_value()) {
        spdlog::error("failed to retrieve RETRY_AMOUNT");
        return;
    }

    const int requested_retry_amount = expected_retry_amount.value();
    if (requested_retry_amount <= 0) {
        spdlog::error("retry amount must be greater than 0.");
        return;
    }

    if (job.get_retry_amount() >= requested_retry_amount) {
        spdlog::info("job retry_amount reached! terminating with success!");
        job_manager.remove_job(pipeline_id);
        return;
    }

    job.set_name(job_name);
    job.increase_retry_amount();
    retry_job(job);
}

std::optional<nlohmann::json> Server::get_pipeline_jobs(int project_id, int pipeline_id) {
    const std::string path = std::format("/api/v4/projects/{}/pipelines/{}/jobs", project_id, pipeline_id);
    httplib::Result jobs = this->gitlab_client.Get(path);

    if (!nlohmann::json::accept(jobs->body))
        return std::nullopt;

    const auto &jobs_json = nlohmann::json::parse(jobs->body);
    return std::make_optional(jobs_json);
}
