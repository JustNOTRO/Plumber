#include "Server.hpp"
#include "spdlog/spdlog.h"

#include <utility>

Server::Server(std::string ip, const std::uint16_t &port, const Config &config) : ip(std::move(ip)), port(port), config(config) {

    Post("/retry", [config](const httplib::Request &req, httplib::Response &) {
        const auto bot_username_opt = config.get_value<std::string>("bot_username");
        if (!bot_username_opt.has_value()) {
            spdlog::error("Could not find bot username.");
            return;
        }

        const json json_body = json::parse(req.body);
        const std::string note = json_body.at("object_attributes").at("note");

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