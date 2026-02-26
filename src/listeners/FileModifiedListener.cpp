//
// Created by Eilon Hafzadi on 24/01/2026.
//

#include "FileModifiedListener.hpp"

#include "../server/wrapper/HttpsServer.hpp"
#include "../utils/ServerUtils.hpp"
#include "spdlog/spdlog.h"

FileModifiedListener::FileModifiedListener(const std::weak_ptr<Server> &server) : server(server) {}

bool update_needed(const std::string &certificate, const std::string &filename) {
    return filename == certificate;
}

std::string FileModifiedListener::read_all(const char *path) {
    std::ifstream stream(path);
    return {
        (std::istreambuf_iterator(stream)),
        std::istreambuf_iterator<char>()
    };
}

void FileModifiedListener::handleFileAction(efsw::WatchID, const std::string &dir, const std::string &filename, const efsw::Action action, std::string) {
    const std::string ssl_cert_path = ServerUtils::require_env("SSL_CERT_PATH");
    const std::string ssl_key_path = ServerUtils::require_env("SSL_KEY_PATH");

    const auto file_path = std::filesystem::path(ssl_cert_path);
    const std::string certificate = file_path.filename().string();

    const auto shared_server = server.lock();
    if (!shared_server || !update_needed(certificate, filename))
        return;

    {
        const auto now = std::chrono::steady_clock::now();
        std::lock_guard lock(mutex);

        const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_event_times[filename]);
        if (last_event_times.contains(filename) && diff.count() < DEBOUNCE_INTERVAL_MILLIS)
            return;

        last_event_times[filename] = now;
    }


    if (action == efsw::Action::Modified) {
        auto cert = read_all(ssl_cert_path.c_str());
        auto key = read_all(ssl_key_path.c_str());
        const auto ssl_server = dynamic_cast<HttpsServer*>(shared_server.get());

        if (!cert.empty() && !key.empty())
            ssl_server->update_certs_pem(cert.c_str(), key.c_str());
    }
}
