//
// Created by Eilon Hafzadi on 28/11/2025.
//

#include "factory/ServerFactory.hpp"
#include "server/http/HttpServer.hpp"
#include "utils/ServerUtils.hpp"

void handle_exit_signal(int __attribute__((unused)) signal) {
    spdlog::info("Stopping the server..");
    std::exit(0);
}

int main() {
    const std::string ip = ServerUtils::require_env("SERVER_IP");
    const std::uint16_t port = ServerUtils::require_port("SERVER_PORT");
    const std::string gitlab_instance = ServerUtils::require_env("GITLAB_INSTANCE");

    const std::unique_ptr<Server> server = ServerFactory::create(ip, port, gitlab_instance);

    std::signal(SIGTERM, handle_exit_signal);
    server->start();
}
