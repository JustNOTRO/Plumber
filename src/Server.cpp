#include "Server.h"

Server::Server(std::string &ip, std::uint16_t &port, YAML::Node &config) : ip(ip), port(port), config(config) {
    std::println("Server is starting...");

    Post("/retry", [config](const httplib::Request &req, httplib::Response &) {
        const auto bot_username = config["bot_username"].as<std::string>();

        const json json_body = json::parse(req.body);
        const std::string note = json_body.at("object_attributes").at("note");

        if (note.contains(USER_MENTION_PREFIX + bot_username)) {
            std::println("Mentioned bot");
        } else {
            std::println("Not mentioned bot");
        }
    });
}

Server::~Server() {
    stop();
}

void Server::start() {
    std::println("Server is now running on: {}:{}", this->ip, this->port);
    listen(this->ip, this->port);
}