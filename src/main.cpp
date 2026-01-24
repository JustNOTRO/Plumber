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

    explicit CertWatcher(const std::weak_ptr<Server> &server) : listener(server) {
        const std::string cert = std::getenv("SSL_KEY_PATH");
        const auto file_path = std::filesystem::path(cert);
        const std::string path = file_path.parent_path().string();

        watcher.addWatch(path, &listener, false);
    }

    void watch() {
        watcher.watch();
    }
};

std::weak_ptr<Server> weak_server;

void handle_exit_signal(int __attribute__((unused)) signal) {
    spdlog::info("Stopping the server..");

    if (const auto server = weak_server.lock())
        server->stop();
}

int main() {
    const std::string ip = ServerUtils::require_env("SERVER_IP");
    const std::uint16_t port = ServerUtils::require_port("SERVER_PORT");
    const std::string gitlab_instance = ServerUtils::require_env("GITLAB_INSTANCE");

    const std::shared_ptr<Server> server = ServerFactory::create(ip, port, gitlab_instance);
    weak_server = server;

    std::signal(SIGTERM, handle_exit_signal);

    if (const auto ssl_server = dynamic_cast<HttpsServer*>(server.get()); ssl_server) {
        const auto cert_watcher = std::make_unique<CertWatcher>(weak_server);
        cert_watcher->watch();
    }

    server->start();
}