#include "HttpServer.hpp"
#include "../../utils/ServerUtils.hpp"

void HttpServer::setup_gitlab_client() {
    std::string gitlab_access_token = ServerUtils::require_env("GITLAB_ACCESS_TOKEN");

    gitlab_client.set_default_headers({{"PRIVATE-TOKEN", gitlab_access_token}});
    gitlab_client.set_error_logger([](const httplib::Error &err, const httplib::Request *req) {
        if (req)
            spdlog::error("{} {}", req->method, req->path);

        spdlog::error("failed: {}", httplib::to_string(err));

        switch (err) {
            case httplib::Error::Connection:
                spdlog::error("(verify server is running and reachable)");
                break;
            case httplib::Error::ConnectionTimeout:
                spdlog::error(" (increase timeout or check network latency)");
                break;
            case httplib::Error::Read:
                spdlog::error(" (server may have closed connection prematurely)");
                break;
            default:
                break;
        }
    });
}