//
// Created by Eilon Hafzadi on 24/01/2026.
//

#include "FileModifiedListener.hpp"

#include "spdlog/spdlog.h"

FileModifiedListener::FileModifiedListener(const std::weak_ptr<Server> &server) : server(server) {}

bool should_restart_server(const std::string &certificate, const std::string &filename, const efsw::Action &action) {
    return certificate == "file.txt" &&
           action == efsw::Actions::Modified ||
           action == efsw::Actions::Moved ||
           action == efsw::Actions::Add;
}

void FileModifiedListener::handleFileAction(
    efsw::WatchID,
    const std::string &dir,
    const std::string &filename,
    const efsw::Action action,
    std::string) {

    const char *cert_path = std::getenv("CERT");
    if (!cert_path)
        return;

    const auto file_path = std::filesystem::path(cert_path);
    const std::string certificate = file_path.filename().string();

    if (const auto shared_server = server.lock(); shared_server && should_restart_server(certificate, filename, action)) {
        spdlog::info("Restarting server due to cert file change");
        shared_server->restart();
    }
}
