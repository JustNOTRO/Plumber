//
// Created by Eilon Hafzadi on 28/11/2025.
//

#include "factory/ServerFactory.hpp"
#include "listeners/FileModifiedListener.hpp"
#include "server/wrapper/HttpServer.hpp"
#include "utils/ServerUtils.hpp"

std::weak_ptr<Server> weak_server;

void handle_exit_signal(int __attribute__((unused)) signal) {
    spdlog::info("Stopping the server..");

    if (const auto server = weak_server.lock())
        server->stop();
    else
        spdlog::error("Failed to stop the server.");
}

int main() {
    const std::string ip = ServerUtils::require_env("SERVER_IP");
    const unsigned short port = ServerUtils::require_port("SERVER_PORT");
    const std::string gitlab_instance = ServerUtils::require_env("GITLAB_INSTANCE");

    const std::shared_ptr<Server> server = ServerFactory::create(ip, port, gitlab_instance);
    weak_server = server;

    std::signal(SIGTERM, handle_exit_signal);
    std::signal(SIGINT, handle_exit_signal);

    server->start();
}