//
// Created by Eilon Hafzadi on 28/11/2025.
//

#include "httplib.h"
#include <filesystem>
#include "yaml-cpp/yaml.h"
#include <print>
#include <nlohmann/json.hpp>

#define USER_MENTION_PREFIX "@"

using json = nlohmann::json;

void try_load_config(YAML::Node &config) {
    try {
        const std::string path = std::filesystem::current_path().parent_path().string() + "/src/config.yaml";
        config = YAML::LoadFile(path);
    } catch (std::exception &e) {
        std::println("Exception caught: {}", e.what());
        std::exit(1);
    }
}

std::optional<std::string> get_server_ip(YAML::Node &config) {
    if (!config["ip"])
        return std::nullopt;

    return std::make_optional(config["ip"].as<std::string>());
}

std::optional<std::uint16_t> get_server_port(YAML::Node &config) {
    if (!config["port"])
        return std::nullopt;

    return std::make_optional(config["port"].as<std::uint16_t>());
}

int main() {
    httplib::Server server;
    YAML::Node config;
    try_load_config(config);

    server.Post("/retry", [config](const httplib::Request &req, httplib::Response &) {
        const auto bot_username = config["bot_username"].as<std::string>();

        const json json_body = json::parse(req.body);
        const std::string note = json_body.at("object_attributes").at("note");

        if (note.contains(USER_MENTION_PREFIX + bot_username)) {
            std::println("Mentioned bot");
        } else {
            std::println("Not mentioned bot");
        }
    });

    const auto server_ip_opt = get_server_ip(config);
    if (server_ip_opt->empty()) {
        std::println("Config file is missing ip key");
        return 1;
    }

    const auto server_port_opt = get_server_port(config);
    if (!server_port_opt.has_value()) {
        std::println("Config file is missing port key");
        return 1;
    }

    const auto &ip = server_ip_opt.value();
    const auto &port = server_port_opt.value();

    std::println("Server is now running on: {}:{}", ip, port);
    server.listen(ip, port);
}
