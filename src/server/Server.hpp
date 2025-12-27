//
// Created by Eilon Hafzadi on 30/11/2025.
//

#pragma once

#include "httplib.h"
#include "spdlog/spdlog.h"
#include "../managers/JobManager.hpp"

#include <expected>
#include <nlohmann/json.hpp>

constexpr std::string BOT_MENTION_PREFIX = "@";

struct JobInfo {
    std::string name;
    int id{};
    int project_id{};
    int merge_req_id{};
    int comment_id{};
    int pipeline_id{};
};

class Server final : public httplib::Server {
public:
    Server();

    void start();

private:
    void retry_job(Job &job);

    [[nodiscard]] std::optional<std::reference_wrapper<Job>> get_job_by_name(const std::string &job_name, const nlohmann::json &req_body);

    void delete_previous_bot_reactions(Job &job, const std::string &bot_username);

    void handle_comment_webhook(const nlohmann::json &req_body, const std::string &bot_username, const std::string &job_name);

    void approve_merge_request(Job &job, const std::string &bot_username, const int pipeline_id);

    void unapprove_merge_request(Job &job, const std::string &bot_username, int pipeline_id);

    void handle_job_webhook(const nlohmann::json &req_body, const std::string &job_name, const std::string &bot_username);

    [[nodiscard]] nlohmann::json get_pipeline_jobs(int project_id, int pipeline_id);

    void setup_gitlab_client();

    void react_with_emoji(const Job &job, const std::string &emoji);

    std::expected<Job, std::string> create_job(const nlohmann::json &pipeline_jobs, JobInfo &job_info);

    void retry_job(JobInfo &job_info);

    std::string ip;

    std::uint16_t port;

    httplib::Client gitlab_client;

    JobManager job_manager;
};
