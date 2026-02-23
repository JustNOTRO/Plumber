//
// Created by Eilon Hafzadi on 24/01/2026.
//

#include "FileModifiedListener.hpp"

#include "spdlog/spdlog.h"

FileModifiedListener::FileModifiedListener(const std::weak_ptr<Server> &server) : server(server) {}

bool should_restart_server(const std::string &certificate, const std::string &filename) {
    return filename == certificate;
}

void FileModifiedListener::handleFileAction(efsw::WatchID, const std::string &dir, const std::string &filename, const efsw::Action action, std::string) {
    const char *cert_path = std::getenv("SSL_CERT_PATH");
    if (!cert_path)
        return;

    const auto file_path = std::filesystem::path(cert_path);
    const std::string certificate = file_path.filename().string();

    const auto shared_server = server.lock();
    if (!shared_server || !should_restart_server(certificate, filename))
        return;

    {
        const auto now = std::chrono::steady_clock::now();
        std::lock_guard lock(mutex);

        const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_event_times[filename]);
        if (last_event_times.contains(filename) && diff.count() < DEBOUNCE_INTERVAL_MILLIS)
            return;

        last_event_times[filename] = now;
    }

    spdlog::info("Restarting server due to cert file change: {}", filename);
    shared_server->restart();
}
