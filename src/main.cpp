//
// Created by Eilon Hafzadi on 28/11/2025.
//

#include "server/Server.hpp"

#include <spdlog/spdlog.h>

int main() {
    const Config config;
    const auto ip_addr_opt = config.get_value<std::string>("ip");

    if (!ip_addr_opt.has_value()) {
        spdlog::error("Could not find IP address");
        return 1;
    }

    const auto port_opt = config.get_value<std::uint16_t>("port");
    if (!port_opt.has_value()) {
        spdlog::error("Could not find port");
        return 1;
    }

    const auto ip = ip_addr_opt.value();
    const auto port = port_opt.value();

    Server server(ip, port, config);
    server.start();
}
