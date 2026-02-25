//
// Created by Eilon Hafzadi on 24/01/2026.
//

#pragma once

#include <memory>
#include <efsw/efsw.hpp>

#include "../server/Server.hpp"

class FileModifiedListener : public efsw::FileWatchListener {
public:
    explicit FileModifiedListener(const std::weak_ptr<Server> &server);

    void handleFileAction(efsw::WatchID watch_id,
                          const std::string &dir,
                          const std::string &filename,
                          efsw::Action action,
                          std::string) override;

private:
    static constexpr int DEBOUNCE_INTERVAL_MILLIS = 500;

    std::weak_ptr<Server> server;

    std::unordered_map<std::string, std::chrono::steady_clock::time_point> last_event_times;

    std::mutex mutex;

    static std::string read_all(const char* path);
};
