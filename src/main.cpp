//
// Created by Eilon Hafzadi on 28/11/2025.
//

#include "server/Server.hpp"
#include "spdlog/spdlog.h"

#define DEFAULT_IP "0.0.0.0"
#define DEFAULT_PORT 8080
#define HTTPS_PORT 443

int main() {
    Config config;

    const auto ip_addr = config.get_value<std::string>("ip").value_or(DEFAULT_IP);
    const std::uint16_t port = config.get_value<std::uint16_t>("port").value_or(DEFAULT_PORT);

    const auto gitlab_instance = config.get_value<std::string>("gitlab_instance").value_or("gitlab.com");
    httplib::Client gitlab_client(gitlab_instance, HTTPS_PORT);

    Server server(ip_addr, port, config, gitlab_client);
    server.start();
}