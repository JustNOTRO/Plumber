#include "Server.hpp"
#include "../managers/Job.hpp"

constexpr int HTTP_CREATED = 201;

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

std::string require_url_scheme(const char *name) {
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

std::optional<std::string> get_env(const char *name) {
    const char *v = std::getenv(name);
    if (!v) {
        return std::nullopt;
    }

    return std::make_optional(v);
}

void handle_exit_signal(int __attribute__((unused)) signal) {
    spdlog::info("Stopping the server..");
    std::exit(0);
}

Server::Server()
    : ip(require_env("SERVER_IP")),
      port(require_port("SERVER_PORT")),
      gitlab_client(require_url_scheme("GITLAB_INSTANCE")) {}

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
        spdlog::error("failed to bind address {} to port {}.", ip, port);
        std::exit(1);
    }

    setup_gitlab_client();

    Post("/webhook", [this](const httplib::Request &req, httplib::Response &) {
        if (!nlohmann::json::accept(req.body)) {
            spdlog::error("failed to parse request body.");
            return;
        }

        const std::string bot_username = require_env("BOT_USERNAME");
        const std::string job_name = require_env("JOB_NAME");
        const nlohmann::json req_body = nlohmann::json::parse(req.body);

        if (const std::string object_kind = req_body["object_kind"].get<std::string>(); object_kind == "build")
            handle_job_webhook(req_body, job_name);
        else if (object_kind == "note")
            handle_comment_webhook(req_body, bot_username, job_name);
        else
            spdlog::error("unsupported object kind: {}", object_kind);
    });

    spdlog::info("Server is now running on: {}:{}", ip, port);
    std::signal(SIGTERM, handle_exit_signal);

    listen_after_bind();
}

void Server::retry_job(Job &job) {
    job.increase_retry_amount();

    const std::string path = std::format("/api/v4/projects/{}/jobs/{}/retry", job.get_project_id(), job.get_id());
    if (const httplib::Result res = gitlab_client.Post(path); res && res->status == HTTP_CREATED)
        spdlog::info("successfully retried job {} for {}x times", job.get_name(), job.get_retry_amount());
    else
        spdlog::error("failed to retry job {} with status {}", job.get_name(), res->status);
}

std::optional<std::reference_wrapper<Job>> Server::get_job_by_name(const std::string &job_name, const nlohmann::json &req_body) {
    const int project_id = req_body["project_id"].get<int>();
    const nlohmann::json merge_request = req_body["merge_request"].get<nlohmann::json>();
    const int pipeline_id = merge_request["head_pipeline_id"].get<int>();

    const std::optional<nlohmann::json> jobs_opt = get_pipeline_jobs(project_id, pipeline_id);
    if (!jobs_opt)
        return std::nullopt;

    for (const nlohmann::json &jobs = jobs_opt.value(); const nlohmann::json &other_job: jobs) {
        if (other_job.at("name") != job_name)
            continue;

        const std::string job_status = other_job["status"].get<std::string>();

        Job &job = job_manager.create_job(pipeline_id, other_job);
        job.set_status(job_status);
        return job;
    }

    return std::nullopt;
}

void Server::react_with_emoji(const Job &job, const std::string &emoji) {
    const int project_id = job.get_project_id();
    const int merge_req_id = job.get_merge_request_id();
    const int comment_id = job.get_comment_id();

    spdlog::info("Project id: {}", project_id);
    spdlog::info("Merge req: {}", merge_req_id);
    spdlog::info("Comment {}", comment_id);

    const httplib::Result res = gitlab_client.Post(
        std::format("/api/v4/projects/{}/merge_requests/{}/notes/{}/award_emoji?name={}",
        project_id,
        merge_req_id,
        comment_id,
        emoji
    ));

    if (res && res->status == HTTP_CREATED)
        spdlog::info("successfully reacted with {} emoji to comment {} for failed job {}", emoji, comment_id, job.get_name());
    else
        spdlog::error("failed to react {} emoji to comment {} with status {}", emoji, comment_id, res ? res->status : 0);
}

void Server::handle_comment_webhook(const nlohmann::json &req_body, const std::string &bot_username, const std::string &job_name) {
    const nlohmann::json object_attributes = req_body["object_attributes"].get<nlohmann::json>();
    const nlohmann::json noteable_type = object_attributes["noteable_type"].get<std::string>();

    if (noteable_type != "MergeRequest")
        return;

    const std::string note = object_attributes["note"].get<std::string>();
    if (!note.contains(BOT_MENTION_PREFIX + bot_username))
        return;

    const std::optional<std::reference_wrapper<Job>> job_opt = get_job_by_name(job_name, req_body);
    if (!job_opt) {
        spdlog::error("requested job {} not found.", job_name);
        return;
    }

    const int merge_req_id = req_body["merge_request"]["iid"].get<int>();
    const int comment_id = object_attributes["id"].get<int>();

    Job &job = job_opt.value();
    job.set_merge_request_id(merge_req_id);
    job.set_comment_id(comment_id);
    job.set_name(job_name);

    if (job.get_status() == "success") {
        react_with_emoji(job, get_env("RETRY_REQUEST_APPROVED_REACTION").value_or("rocket"));
        retry_job(job);
    } else {
        spdlog::error("job {} is not in success state, cannot retry.", job_name);
        react_with_emoji(job, get_env("RETRY_REQUEST_DENIED_REACTION").value_or("thumbsdown"));
    }
}

void Server::handle_job_webhook(const nlohmann::json &req_body, const std::string &job_name) {
    if (const std::string build_name = req_body["build_name"].get<std::string>(); build_name != job_name)
        return;

    const int job_id = req_body["build_id"].get<int>();
    const std::string job_status = req_body["build_status"].get<std::string>();
    const int pipeline_id = req_body["pipeline_id"].get<int>();

    const std::optional<std::reference_wrapper<Job>> job_opt = job_manager.get_job(pipeline_id);
    if (!job_opt) {
        spdlog::error("failed to retrieve job, {}", job_name);
        return;
    }

    Job &job = job_opt.value();
    job.set_id(job_id);
    job.set_status(job_status);

    if (job_status == "failed") {
        job_manager.remove_job(pipeline_id);
        spdlog::error("job {} failed! terminating..", job_name);
        react_with_emoji(job, get_env("JOB_FAILED_REACTION").value_or("x"));
        return;
    }

    if (job_status != "success")
        return;

    const int required_retry_amount = require_retry_amount("RETRY_AMOUNT");
    if (required_retry_amount <= 0) {
        spdlog::error("retry amount must be greater than 0.");
        return;
    }

    if (job.get_retry_amount() >= required_retry_amount) {
        spdlog::info("job retry_amount reached! terminating with success!");
        react_with_emoji(job, get_env("JOB_SUCCESS_REACTION").value_or("white_check_mark"));
        job_manager.remove_job(pipeline_id);
    } else {
        retry_job(job);
    }
}

std::optional<nlohmann::json> Server::get_pipeline_jobs(int project_id, int pipeline_id) {
    const std::string path = std::format("/api/v4/projects/{}/pipelines/{}/jobs", project_id, pipeline_id);
    httplib::Result jobs = this->gitlab_client.Get(path);

    if (!nlohmann::json::accept(jobs->body))
        return std::nullopt;

    const nlohmann::json jobs_json = nlohmann::json::parse(jobs->body);
    return std::make_optional(jobs_json);
}
