//
// Created by Eilon Hafzadi on 22/01/2026.
//

#include "ServerFactory.hpp"

#include "../server/wrapper/HttpServer.hpp"
#include "../server/wrapper/HttpsServer.hpp"

#include <fstream>

#include "../utils/ServerUtils.hpp"

bool use_ssl(const std::string &ssl_cert_path, const std::string ssl_cert_key) {
    const std::ifstream cert_file(ssl_cert_path);
    if (!cert_file.is_open()) {
        spdlog::error("Failed to open cert file {}", ssl_cert_path);
        return false;
    }

    const std::ifstream cert_key_file(ssl_cert_key);
    if (!cert_key_file.is_open()) {
        spdlog::error("Failed to open cert key file {}", ssl_cert_key);
        return false;
    }

    return cert_file.good() && cert_key_file.good();
}

std::shared_ptr<Server> ServerFactory::create(const std::string &ip, std::uint16_t port, const std::string &gitlab_instance) {
    const std::string ssl_cert_path = ServerUtils::require_env("SSL_CERT_PATH");
    const std::string ssl_key_path = ServerUtils::require_env("SSL_KEY_PATH");

    if (use_ssl(ssl_cert_path, ssl_key_path))
        return std::make_shared<HttpsServer>(ip, port, gitlab_instance, ssl_cert_path, ssl_key_path);

    return std::make_shared<HttpServer>(ip, port, gitlab_instance);
}