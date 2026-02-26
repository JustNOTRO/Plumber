//
// Created by Eilon Hafzadi on 22/01/2026.
//

#pragma once

#include "../server/Server.hpp"

class ServerFactory {

public:
    ServerFactory() = delete;

    static std::shared_ptr<Server> create(const std::string &ip, unsigned short port, const std::string &gitlab_instance);
};
