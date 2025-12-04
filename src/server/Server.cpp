#include "Server.hpp"
#include "spdlog/spdlog.h"

#include <utility>

Server::Server(std::string ip, const std::uint16_t &port, Config &config, httplib::Client &gitlab_client)
: ip(std::move(ip)), port(port), config(config), gitlab_client(gitlab_client) {

    Post("/retry", [&config](const httplib::Request &req, httplib::Response &) {
        const auto bot_username_opt = config.get_value<std::string>("bot_username");
        if (!bot_username_opt.has_value()) {
            spdlog::error("Could not find bot username.");
            return;
        }

        try {
            json json_body = json::parse(req.body);

            if (!json_body.contains("object_attributes")) {
                spdlog::error("object_attributes not found in request body.");
                return;
            }

            auto object_attributes = json_body.at("object_attributes");

            if (!object_attributes.contains("note")) {
                spdlog::error("note not found in request body.");
                return;
            }

            const std::string note = object_attributes.at("note");

            if (!object_attributes.contains("noteable_type")) {
                spdlog::error("noteable_type not found in request body.");
                return;
            }

            const std::string noteable_type = object_attributes.at("noteable_type");
            if (noteable_type != "MergeRequest")
                return;

            if (note.contains(USER_MENTION_PREFIX + bot_username_opt.value())) {
                spdlog::info("User has mentioned a bot");
                // do some stuff here
            }
        } catch (const std::exception &e) {
            spdlog::error("Could not parse request body: {}", e.what());
        }
    });
}

Server::~Server() {
    stop();
}

void Server::start() {
    spdlog::info("Server is now running on: {}:{}", ip, port);
    listen(ip, port);
}
