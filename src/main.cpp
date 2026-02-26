//
// Created by Eilon Hafzadi on 28/11/2025.
//

#include "efsw/efsw.hpp"
#include "factory/ServerFactory.hpp"
#include "listeners/FileModifiedListener.hpp"
#include "server/wrapper/HttpServer.hpp"
#include "server/wrapper/HttpsServer.hpp"
#include "utils/ServerUtils.hpp"

struct CertWatcher {
    efsw::FileWatcher watcher;
    FileModifiedListener listener;
    std::string directory;

    explicit CertWatcher(const std::weak_ptr<Server> &server) : listener(server) {
        const std::string cert = std::getenv("SSL_KEY_PATH");
        const auto file_path = std::filesystem::path(cert);

        const std::string dir = file_path.parent_path().string();
        directory = dir;

        watcher.addWatch(directory, &listener, false);
    }

    ~CertWatcher() {
        stop();
    }

    void watch() {
        watcher.watch();
    }

    void stop() {
        watcher.removeWatch(directory);
    }
};

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
    const auto cert_watcher = std::make_unique<CertWatcher>(server);
    weak_server = server;

    std::signal(SIGTERM, handle_exit_signal);
    std::signal(SIGINT, handle_exit_signal);

    if (const auto ssl_server = dynamic_cast<HttpsServer*>(server.get()); ssl_server)
        cert_watcher->watch();

    server->start();
}