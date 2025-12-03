#include "Server.hpp"
#include "spdlog/spdlog.h"

#include <utility>

#define HTTPS_PORT 443

Server::Server(std::string ip, const std::uint16_t &port, Config &config) : ip(std::move(ip)), port(port),
                                                                            config(config) {
    const auto gitlab_instance = config.get_value<std::string>("gitlab_instance").value_or("gitlab.com");

    httplib::Client client(gitlab_instance, HTTPS_PORT);

    Post("/retry", [&client, config](const httplib::Request &req, httplib::Response &) {
        const auto bot_username_opt = config.get_value<std::string>("bot_username");
        if (!bot_username_opt.has_value()) {
            spdlog::error("Could not find bot username.");
            return;
        }

        json json_body;
        try {
            json_body = json::parse(req.body);
            spdlog::info("Parsing");
        } catch (const std::exception &e) {
            spdlog::error("Could not parse request body: {}", e.what());
            return;
        }

        if (!json_body.contains("object_attributes")) {
            spdlog::error("object_attributes not found in request body.");
            return;
        }

        auto object_attributes = json_body.at("object_attributes");
        const std::string note = object_attributes.at("note");

        const std::string noteable_type = object_attributes.at("noteable_type");
        if (noteable_type != "MergeRequest")
            return;

        if (note.contains(USER_MENTION_PREFIX + bot_username_opt.value())) {
            spdlog::info("User has mentioned a bot");
            // do some stuff here
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
