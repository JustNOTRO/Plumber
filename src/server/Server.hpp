//
// Created by Eilon Hafzadi on 30/11/2025.
//

#pragma once

#define BOT_MENTION_PREFIX "@"

#include "httplib.h"
#include "spdlog/spdlog.h"
#include "../managers/JobManager.hpp"

#include <nlohmann/json.hpp>


class Server final : public httplib::Server {
public:
    Server();

    void start();

private:
    void retry_job(Job &job);

    [[nodicard]] std::optional<std::reference_wrapper<Job>> get_job_by_name(const std::string &job_name, const nlohmann::json &req_body);

    void handle_comment_webhook(const nlohmann::json &req_body, const std::string &bot_username, const std::string &job_name);

    void handle_job_webhook(const nlohmann::json &req_body, const std::string &job_name);

    [[nodiscard]] std::optional<nlohmann::json> get_pipeline_jobs(int project_id, int pipeline_id);

    void setup_gitlab_client();

    template<typename T>
    std::expected<T, std::string> get_node(const nlohmann::json &json, const std::string &name) {
        if (!json.contains(name)) {
            auto err = std::format("could not find {}", name);
            return std::unexpected(err);
        }

        return json.at(name).get<T>();
    }

    std::string ip;

    std::uint16_t port;

    httplib::Client gitlab_client;

    JobManager job_manager;
};
