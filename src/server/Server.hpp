//
// Created by Eilon Hafzadi on 30/11/2025.
//

#pragma once

#define USER_MENTION_PREFIX "@"

#include "../data/Config.hpp"
#include "../httplib.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Server final : public httplib::Server {
public:
    Server(std::string ip, const std::uint16_t &port, Config &config);

    ~Server() override;

    void start();

private:
    std::string ip;

    std::uint16_t port;

    Config &config;
};
