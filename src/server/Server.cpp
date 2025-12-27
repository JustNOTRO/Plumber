#include "Server.hpp"
#include "../managers/Job.hpp"

constexpr int HTTP_CREATED = 201;
constexpr int HTTP_DELETED = 204;
constexpr int HTTP_UNAUTHORIZED = 401;
constexpr int HTTP_NOT_FOUND = 404;

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
    if (!v)
        return std::nullopt;

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
            handle_job_webhook(req_body, job_name, bot_username);
        else if (object_kind == "note")
            handle_comment_webhook(req_body, bot_username, job_name);
        else
            spdlog::error("unsupported object kind: {}", object_kind);
    });

    spdlog::info("Server is now running on: {}:{}", ip, port);
    std::signal(SIGTERM, handle_exit_signal);

    listen_after_bind();
}

void Server::delete_previous_bot_reactions(Job &job, const std::string &bot_username) {
    const int project_id = job.get_project_id();
    const int merge_req_id = job.get_merge_request_id();
    const int comment_id = job.get_comment_id();

    const httplib::Result res = gitlab_client.Get(std::format("/api/v4/projects/{}/merge_requests/{}/notes/{}/award_emoji", project_id, merge_req_id, comment_id));
    const nlohmann::json json = nlohmann::json::parse(res->body);

    if (json.empty())
        return;

    for (const nlohmann::json &node : json) {
        if (const std::string username = node["user"]["username"].get<std::string>(); username != bot_username)
            continue;

        const std::string path = std::format("/api/v4/projects/{}/merge_requests/{}/notes/{}/award_emoji/{}", project_id, merge_req_id, comment_id, node["id"].get<int>());
        const httplib::Result delete_res = gitlab_client.Delete(path);
        const std::string emoji = node["name"].get<std::string>();

        if (delete_res->status != HTTP_DELETED)
            spdlog::error("failed to delete reaction {} for {} with status {}", emoji, job.get_name(), delete_res->status);
    }
}

void Server::react_with_emoji(const Job &job, const std::string &emoji) {
    const int project_id = job.get_project_id();
    const int merge_req_id = job.get_merge_request_id();
    const int comment_id = job.get_comment_id();

    const httplib::Result res = gitlab_client.Post(
        std::format("/api/v4/projects/{}/merge_requests/{}/notes/{}/award_emoji?name={}",
        project_id,
        merge_req_id,
        comment_id,
        emoji
    ));

    if (res && res->status != HTTP_CREATED)
        spdlog::error("failed to react {} emoji to comment {} with status {}", emoji, comment_id, res->status);
}

std::expected<Job, std::string> Server::create_job(const nlohmann::json &pipeline_jobs, JobInfo &job_info) {
    for (const nlohmann::json &node : pipeline_jobs) {
        if (node["name"].get<std::string>() != job_info.name)
            continue;

        const auto status = node["status"].get<std::string>();
        job_info.id = node["id"].get<int>();

        Job job = job_manager.create_job(job_info);
        job.set_merge_request_id(job_info.merge_req_id);
        job.set_comment_id(job_info.comment_id);
        job.set_name(job_info.name);
        job.set_status(status);
        return job;
    }

    return std::unexpected("job not found");
}

void Server::retry_job(Job &job) {
    const std::string path = std::format("/api/v4/projects/{}/jobs/{}/retry", job.get_project_id(), job.get_id());
    if (const httplib::Result res = gitlab_client.Post(path); res && res->status == HTTP_CREATED)
        spdlog::info("successfully retried job {} for {}x times", job.get_name(), job.get_retry_amount());
    else
        spdlog::error("failed to retry job {} with status {}", job.get_name(), res->status);
}

void Server::retry_job(JobInfo &job_info) {
    const nlohmann::json pipeline_jobs = get_pipeline_jobs(job_info.project_id, job_info.pipeline_id);
    auto job_expected = create_job(pipeline_jobs, job_info);

    if (!job_expected) {
        spdlog::error("failed: {}", job_expected.error());
        return;
    }

    Job &job = job_expected.value();
    if (job.get_status() != "success") {
        spdlog::error("job {} is not in success state, cannot retry.", job_info.name);
        react_with_emoji(job, get_env("RETRY_REQUEST_DENIED_REACTION").value_or("thumbsdown"));
        return;
    }

    react_with_emoji(job, get_env("RETRY_REQUEST_APPROVED_REACTION").value_or("rocket"));
    job.increase_retry_amount();

    job_manager.add_job(job_info.pipeline_id, job);
    retry_job(job);
}

void Server::handle_comment_webhook(const nlohmann::json &req_body, const std::string &bot_username, const std::string &job_name) {
    const nlohmann::json object_attributes = req_body["object_attributes"].get<nlohmann::json>();
    const std::string noteable_type = object_attributes["noteable_type"].get<std::string>();

    if (noteable_type != "MergeRequest")
        return;

    const std::string note = object_attributes["note"].get<std::string>();
    if (!note.contains(BOT_MENTION_PREFIX + bot_username))
        return;

    JobInfo job_info;
    job_info.name = job_name;
    job_info.merge_req_id = req_body["merge_request"]["iid"].get<int>();
    job_info.project_id = req_body["project_id"].get<int>();
    job_info.comment_id = object_attributes["id"].get<int>();
    job_info.pipeline_id = req_body["merge_request"]["head_pipeline_id"].get<int>();

    retry_job(job_info);
}

void Server::approve_merge_request(Job &job, const std::string &bot_username, const int pipeline_id) {
    delete_previous_bot_reactions(job, bot_username);
    react_with_emoji(job, get_env("JOB_SUCCESS_REACTION").value_or("white_check_mark"));

    const int merge_request_id = job.get_merge_request_id();
    const std::string path = std::format("/api/v4/projects/{}/merge_requests/{}/approve", job.get_project_id(), merge_request_id);

    if (const httplib::Result res = gitlab_client.Post(path); res && res->status == HTTP_CREATED)
        spdlog::info("Plumber check passed successfully approving MR with success!");
    else if (res && res->status == HTTP_UNAUTHORIZED)
        spdlog::info("Plumber check passed successfully! (MR already approved)");
    else
        spdlog::error("failed to approve merge request {} with status {}", merge_request_id, res->status);

    job_manager.remove_job(pipeline_id);
}

void Server::unapprove_merge_request(Job &job, const std::string &bot_username, const int pipeline_id) {
    spdlog::error("job {} failed! terminating..", job.get_name());
    delete_previous_bot_reactions(job, bot_username);
    react_with_emoji(job, get_env("JOB_FAILED_REACTION").value_or("x"));

    const std::string path = std::format("/api/v4/projects/{}/merge_requests/{}/unapprove", job.get_project_id(), job.get_merge_request_id());

    if (const httplib::Result res = gitlab_client.Post(path); res && res->status == HTTP_CREATED)
        spdlog::error("Plumber check failed unapproving MR with failure!");
    else if (res && res->status == HTTP_NOT_FOUND)
        spdlog::info("Plumber check failed! (MR is already not approved)");
    else
        spdlog::error("failed to unapprove merge request {} with status {}", job.get_merge_request_id(), res->status);

    job_manager.remove_job(pipeline_id);
}

void Server::handle_job_webhook(const nlohmann::json &req_body, const std::string &job_name, const std::string &bot_username) {
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

    Job &job = job_opt.value().get();
    job.set_id(job_id);
    job.set_status(job_status);

    if (job_status == "failed") {
        unapprove_merge_request(job, bot_username, pipeline_id);
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
        approve_merge_request(job, bot_username, pipeline_id);
    } else {
        job.increase_retry_amount();
        retry_job(job);
    }
}

nlohmann::json Server::get_pipeline_jobs(int project_id, int pipeline_id) {
    const std::string path = std::format("/api/v4/projects/{}/pipelines/{}/jobs", project_id, pipeline_id);
    const httplib::Result jobs = gitlab_client.Get(path);

    return nlohmann::json::parse(jobs->body);
}
