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
    std::weak_ptr<Server> server;
};

