//
// Created by Eilon Hafzadi on 30/11/2025.
//

#pragma once

#define BOT_MENTION_PERFIX "@"

#include "../data/Config.hpp"
#include "httplib.h"
#include "spdlog/spdlog.h"
#include "../managers/JobManager.hpp"

#include <nlohmann/json.hpp>


class Server final : public httplib::Server {
public:
    Server();

    void start();

private:
    bool retry_job(const Job &job);

    std::optional<Job> get_job_by_name(const std::string &job_name, const nlohmann::json &req_body);

    void handle_comment_webhook(const nlohmann::json &req_body, const std::string &bot_username, const std::string &job_name);

    void handle_job_webhook(const nlohmann::json &req_body, const std::string &job_name);

    std::optional<nlohmann::json> get_pipeline_jobs(const int &project_id, const int &pipeline_id);

    std::string ip;

    std::uint16_t port;

    Config config;

    httplib::Client gitlab_client;

    JobManager job_manager;
};
