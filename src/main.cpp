//
// Created by Eilon Hafzadi on 28/11/2025.
//

#include "server/Server.hpp"

#define DEFAULT_IP "0.0.0.0"
#define DEFAULT_PORT 8080

int main() {
    Config config;

    const auto ip_addr = config.get_value<std::string>("ip").value_or(DEFAULT_IP);
    const std::uint16_t port = config.get_value<std::uint16_t>("port").value_or(DEFAULT_PORT);

    const auto gitlab_instance = config.get_value<std::string>("gitlab_instance").value_or("gitlab.com");
    httplib::SSLClient gitlab_client(gitlab_instance);

    const auto gitlab_access_token_opt = config.get_value<std::string>("gitlab_access_token");

    if (!gitlab_access_token_opt.has_value()) {
        spdlog::error("gitlab access token not found.");
        return 1;
    }

    gitlab_client.set_default_headers({{"PRIVATE-TOKEN", gitlab_access_token_opt.value()}});

    Server server(ip_addr, port, config, gitlab_client);
    server.start();
}