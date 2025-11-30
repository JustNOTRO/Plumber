//
// Created by Eilon Hafzadi on 28/11/2025.
//

#include "Server.h"

void try_load_config(YAML::Node &config) {
    try {
        const std::string path = std::filesystem::current_path().parent_path().string() + "/src/config.yaml";
        config = YAML::LoadFile(path);
    } catch (std::exception &e) {
        std::println("Exception caught: {}", e.what());
        std::exit(1);
    }
}

int main() {
    YAML::Node config;
    try_load_config(config);

    const auto server_ip_opt = std::make_optional(config["ip"].as<std::string>());

    if (server_ip_opt->empty()) {
        std::println("Config file is missing ip key");
        std::exit(1);
    }

    const auto server_port_opt = std::make_optional(config["port"].as<std::uint16_t>());
    if (!server_port_opt.has_value()) {
        std::println("Config file is missing port key");
        std::exit(1);
    }

    std::string ip = server_ip_opt.value();
    std::uint16_t port = server_port_opt.value();

    Server server(ip, port, config);
    server.start();
}
