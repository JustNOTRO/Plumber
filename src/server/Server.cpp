#include "Server.hpp"
#include "Comment.hpp"

Server::Server(std::string ip, const std::uint16_t port, Config &config, httplib::SSLClient &gitlab_client)
    : ip(std::move(ip)), port(port), config(config), gitlab_client(gitlab_client) {
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

        json req_body = json::parse(req.body);

        if (!req_body.contains("object_attributes")) {
            spdlog::error("object_attributes not found in request body.");
            return;
        }

        if (const auto obj_kind = req_body.at("object_kind"); obj_kind != "note")
            return;

        spdlog::info(req.body);

        const auto obj_attributes = req_body.at("object_attributes");
        Comment comment(obj_attributes);

        if (!obj_attributes.contains("noteable_type")) {
            spdlog::error("noteable_type not found in request body.");
            return;
        }

        if (comment.get_type() != Comment::MERGE_REQUEST)
            return;

        const std::string note = obj_attributes.at("note").get<std::string>();
        if (!note.contains(USER_MENTION_PREFIX + bot_username_opt.value()))
            return;

        if (retry_last_pipeline(req_body, obj_attributes))
            spdlog::info("Pipeline retried successfully.");
        else
            spdlog::error("Failed to retry pipeline.");
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

bool Server::retry_last_pipeline(json &req_body, const nlohmann::basic_json<> &obj_attributes) {
    if (!obj_attributes.contains("project_id")) {
        spdlog::error("project_id not found in request body.");
        return false;
    }

    if (!req_body.at("merge_request").contains("head_pipeline_id")) {
        spdlog::error("head_pipeline_id not found in request body.");
        return false;
    }

    const int project_id = obj_attributes.at("project_id").get<int>();
    const int pipeline_id = req_body.at("merge_request").at("head_pipeline_id").get<int>();
    const std::string path = std::format("/api/v4/projects/{}/pipelines/{}/retry", project_id, pipeline_id);

    httplib::Result res = gitlab_client.Post(path);
    return res && res->status == 201;
}
