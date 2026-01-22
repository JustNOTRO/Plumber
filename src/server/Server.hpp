//
// Created by Eilon Hafzadi on 10/01/2026.
//

#pragma once

#include <expected>
#include <httplib.h>
#include <string>

#include "../managers/JobManager.hpp"

constexpr std::string BOT_MENTION_PREFIX = "@";
constexpr int HTTP_CREATED = 201;
constexpr int HTTP_DELETED = 204;
constexpr int HTTP_UNAUTHORIZED = 401;
constexpr int HTTP_NOT_FOUND = 404;

struct JobInfo {
    std::string name;
    int id{};
    int project_id{};
    int merge_req_id{};
    int comment_id{};
    int pipeline_id{};
};

class Server {
public:
    using Handler = httplib::Server::Handler;
    using HandlerWithContentReader = httplib::Server::HandlerWithContentReader;

    Server(std::string ip, std::uint16_t port, const std::string &gitlab_instance);

    virtual ~Server() = default;

    void start();

    virtual bool bind_to_port(const std::string &ip, std::uint16_t port) = 0;

    virtual bool listen_after_bind() = 0;

    virtual void listen(std::string &ip, std::uint16_t port) = 0;

    virtual httplib::Server &Get(const std::string &pattern, Handler handler) = 0;

    virtual httplib::Server &Post(const std::string &pattern, Handler handler) = 0;

    virtual httplib::Server &Post(const std::string &pattern, HandlerWithContentReader handler) = 0;

    virtual httplib::Server &Put(const std::string &pattern, Handler handler) = 0;

    virtual httplib::Server &Put(const std::string &pattern, HandlerWithContentReader handler) = 0;

    virtual httplib::Server &Patch(const std::string &pattern, Handler handler) = 0;

    virtual httplib::Server &Patch(const std::string &pattern, HandlerWithContentReader handler) = 0;

    virtual httplib::Server &Delete(const std::string &pattern, Handler handler) = 0;

    virtual httplib::Server &Delete(const std::string &pattern, HandlerWithContentReader handler) = 0;

    virtual httplib::Server &Options(const std::string &pattern, Handler handler) = 0;

protected:
    virtual void setup_gitlab_client() = 0;

    std::string ip;

    std::uint16_t port;

    httplib::Client gitlab_client;

    JobManager job_manager;

    void retry_job(const Job &job);

    void delete_previous_bot_reactions(Job &job, const std::string &bot_username);

    void handle_comment_webhook(const nlohmann::json &req_body, const std::string &bot_username,
                                const std::string &job_name);

    void approve_merge_request(Job &job, const std::string &bot_username, const int pipeline_id);

    void unapprove_merge_request(Job &job, const std::string &bot_username, const int pipeline_id);

    void handle_job_webhook(const nlohmann::json &req_body, const std::string &job_name,
                            const std::string &bot_username);

    [[nodiscard]] nlohmann::json get_pipeline_jobs(int project_id, int pipeline_id);

    void react_with_emoji(const Job &job, const std::string &emoji);

    [[nodiscard]] std::expected<Job, std::string> create_job(const nlohmann::json &pipeline_jobs, JobInfo &job_info);

    void retry_job(JobInfo &job_info);
};
