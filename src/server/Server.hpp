//
// Created by Eilon Hafzadi on 30/11/2025.
//

#pragma once

#define BOT_MENTION_PERFIX "@"

#include "../data/Config.hpp"
#include "../httplib.h"
#include "spdlog/spdlog.h"

#include <nlohmann/json.hpp>

#include "../managers/JobManager.hpp"

using json = nlohmann::json;

class Server final : public httplib::Server {
public:
    Server(std::string ip, std::uint16_t port, Config &config, httplib::SSLClient &gitlab_client);

    ~Server() override;

    void start();

private:
    bool retry_job(const Job &job) const;

    std::optional<Job> get_job_by_name(const std::string &job_name, const json &req_body);

    void handle_comment_webhook(const json &req_body, const std::string &bot_username, const std::string &job_name);

    void handle_job_webhook(const json &req_body, const std::string &job_name);

    std::optional<nlohmann::json> get_pipeline_jobs(const int &project_id, const int &pipeline_id) const;

    std::string ip;

    std::uint16_t port;

    Config &config;

    httplib::SSLClient &gitlab_client;

    JobManager job_manager;
};
