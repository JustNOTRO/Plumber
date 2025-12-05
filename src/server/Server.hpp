//
// Created by Eilon Hafzadi on 30/11/2025.
//

#pragma once

#define USER_MENTION_PREFIX "@"

#include "../data/Config.hpp"
#include "../httplib.h"
#include "spdlog/spdlog.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Server final : public httplib::Server {
public:
    Server(std::string ip, std::uint16_t port, Config &config, httplib::SSLClient &gitlab_client);

    ~Server() override;

    void start();

private:
    bool retry_last_pipeline(json &req_body, const nlohmann::basic_json<> &obj_attributes);

    std::string ip;

    std::uint16_t port;

    Config &config;

    httplib::SSLClient &gitlab_client;
};
