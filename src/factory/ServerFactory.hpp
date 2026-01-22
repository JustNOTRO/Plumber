//
// Created by Eilon Hafzadi on 22/01/2026.
//

#pragma once

#include "../server/Server.hpp"

class ServerFactory {

public:
    ServerFactory() = delete;

    static std::unique_ptr<Server> create(const std::string &ip, std::uint16_t port, const std::string &gitlab_instance);
};
