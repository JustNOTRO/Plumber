//
// Created by Eilon Hafzadi on 22/01/2026.
//

#include "ServerFactory.hpp"

#include "../server/http/HttpServer.hpp"
#include "../server/http/HttpsServer.hpp"
#include "../utils/ServerUtils.hpp"
#include "spdlog/spdlog.h"

std::unique_ptr<Server> ServerFactory::create(const std::string &ip, std::uint16_t port, const std::string &gitlab_instance) {
    if (!std::getenv("CERTIFICATE")) {
        return std::make_unique<HttpsServer>(ip, port, gitlab_instance,
            ServerUtils::require_env("CERTIFICATE"),
            ServerUtils::require_env("CERT_KEY")
            );
    }

    return std::make_unique<HttpServer>(ip, port, gitlab_instance);
}
