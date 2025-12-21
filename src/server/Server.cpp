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
        spdlog::error("could not parse {}, invalid URL scheme", name);
        std::exit(1);
    }

    return env;
}

int require_retry_amount(const char *name) {
    const std::string env = require_env(name);

    try {
        return std::stoi(env);
    } catch (std::exception &e) {
        spdlog::error("could not parse {} {}", name, e.what());
        std::exit(1);
    }
}

void handle_exit_signal(int __attribute__((unused)) signal) {
    spdlog::info("Stopping the server..");
    std::exit(0);
}

Server::Server()
    : ip(require_env("SERVER_IP")),
      port(require_port("SERVER_PORT")),
      gitlab_client(require_http_format("GITLAB_INSTANCE")) {}

void Server::setup_gitlab_client() {
    std::string gitlab_access_token = require_env("GITLAB_ACCESS_TOKEN");

    gitlab_client.enable_server_certificate_verification(false);
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
    setup_gitlab_client();

    Post("/webhook", [this](const httplib::Request &req, httplib::Response &res) {
        spdlog::info("test");
        if (!nlohmann::json::accept(req.body)) {
            spdlog::error("failed to parse request body.");
            return;
        }

        const auto bot_username = require_env("BOT_USERNAME");
        const auto job_name = require_env("JOB_NAME");
        const auto req_body = nlohmann::json::parse(req.body);

        const auto object_kind = get_node<std::string>(req_body, "object_kind");
        if (!object_kind) {
            spdlog::error(object_kind.error());
            return;
        }

        if (object_kind.value() == "build")
            handle_job_webhook(req_body, job_name);
        else if (object_kind.value() == "note")
            handle_comment_webhook(req_body, bot_username, job_name);
        else
            spdlog::error("unsupported object kind: {}", object_kind.value());

        res.status = 200;
        res.set_content("test", "text/plain");
    });

    spdlog::info("Server is now running on: {}:{}", ip, port);
    std::signal(SIGTERM, handle_exit_signal);

    listen(ip, port);
}

void Server::retry_job(const Job &job) {
    const std::string path = std::format("/api/v4/projects/{}/jobs/{}/retry", job.get_project_id(), job.get_id());
    if (const httplib::Result res = gitlab_client.Post(path); res && res->status == 201)
        spdlog::info("successfully retried job {} for {}x times", job.get_name(), job.get_retry_amount());
    else
        spdlog::error("failed to retry job {} with status {}", job.get_name(), res->status);
}

std::optional<Job> Server::get_job_by_name(const std::string &job_name, const nlohmann::json &req_body) {
    const auto project_id = get_node<int>(req_body, "project_id");
    const auto merge_request = get_node<nlohmann::json>(req_body, "merge_request");

    if (!merge_request) {
        spdlog::error(merge_request.error());
        return std::nullopt;
    }

    const auto pipeline_id = get_node<int>(merge_request.value(), "head_pipeline_id");
    if (!pipeline_id) {
        spdlog::error(merge_request.error());
        return std::nullopt;
    }

    // todo use std::expected
    const auto jobs_opt = get_pipeline_jobs(project_id.value(), pipeline_id.value());
    if (!jobs_opt)
        return std::nullopt;

    for (const auto &jobs = jobs_opt.value(); const auto &other_job: jobs) {
        if (other_job.at("name") != job_name)
            continue;

        Job job = job_manager.create_job(pipeline_id.value(), other_job);

        if (const auto job_status = get_node<std::string>(other_job, "status"); job_status.has_value())
            job.set_status(job_status.value());
        else
            spdlog::error(job_status.error());

        return std::make_optional(job);
    }

    return std::nullopt;
}

void Server::handle_comment_webhook(const nlohmann::json &req_body, const std::string &bot_username, const std::string &job_name) {
    const auto obj_attributes = get_node<nlohmann::json>(req_body, "object_attributes");
    if (!obj_attributes) {
        spdlog::error(obj_attributes.error());
        return;
    }

    const auto noteable_type = get_node<std::string>(obj_attributes.value(), "noteable_type");

    if (!noteable_type) {
        spdlog::error(noteable_type.error());
        return;
    }

    if (noteable_type.value() != "MergeRequest")
        return;

    const auto note = get_node<std::string>(obj_attributes.value(), "note");
    if (!note) {
        spdlog::error(note.error());
        return;
    }

    if (!note.value().contains(BOT_MENTION_PREFIX + bot_username))
        return;

    std::optional<Job> job_opt = get_job_by_name(job_name, req_body);

    if (!job_opt) {
        spdlog::error("requested job {} not found.", job_name);
        return;
    }

    Job &job = job_opt.value();
    if (job.get_status() == "failed")
        return;

    job.set_name(job_name);

    retry_job(job);
    job.increase_retry_amount();
}

void Server::handle_job_webhook(const nlohmann::json &req_body, const std::string &job_name) {
    const auto build_name = get_node<std::string>(req_body, "build_name");
    if (!build_name) {
        spdlog::error(build_name.error());
        return;
    }

    if (build_name.value() != job_name)
        return;

    const auto job_id = get_node<int>(req_body, "build_id");
    if (!job_id) {
        spdlog::error(job_id.error());
        return;
    }

    const auto job_status = get_node<std::string>(req_body, "build_status");

    if (!job_status) {
        spdlog::error(job_status.error());
        return;
    }

    const auto pipeline_id = get_node<int>(req_body, "pipeline_id");
    if (!pipeline_id) {
        spdlog::error(pipeline_id.error());
        return;
    }

    auto expected_job = job_manager.get_job(pipeline_id.value());
    if (!expected_job) {
        spdlog::error("failed to retrieve job, {}", expected_job.error());
        return;
    }

    Job &job = expected_job.value();
    job.set_id(job_id.value());
    job.set_status(job_status.value());

    if (job_status.value() == "failed") {
        job_manager.remove_job(pipeline_id.value());
        spdlog::error("job {} failed! terminating..", job_name);
        return;
    }

    if (job_status.value() != "success")
        return;

    const int requested_retry_amount = require_retry_amount("RETRY_AMOUNT");
    if (requested_retry_amount <= 0) {
        spdlog::error("retry amount must be greater than 0.");
        return;
    }

    if (job.get_retry_amount() >= requested_retry_amount) {
        spdlog::info("job retry_amount reached! terminating with success!");
        job_manager.remove_job(pipeline_id.value());
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
