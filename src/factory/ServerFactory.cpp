//
// Created by Eilon Hafzadi on 22/01/2026.
//

#include "ServerFactory.hpp"

#include "../server/wrapper/HttpServer.hpp"
#include "../server/wrapper/HttpsServer.hpp"

#include <fstream>

bool use_ssl(const char* cert, const char* cert_key) {
    if (cert == nullptr || cert_key == nullptr)
        return false;

    const std::ifstream cert_file(cert);
    if (!cert_file.is_open()) {
        spdlog::error("Failed to open cert file {}", cert);
        return false;
    }

    const std::ifstream cert_key_file(cert_key);
    if (!cert_key_file.is_open()) {
        spdlog::error("Failed to open cert key file {}", cert_key);
        return false;
    }

    return cert_file.good() && cert_key_file.good();
}

std::shared_ptr<Server> ServerFactory::create(const std::string &ip, std::uint16_t port, const std::string &gitlab_instance) {
    const char* ssl_cert = std::getenv("SSL_CERT_PATH");
    const char* ssl_key_path = std::getenv("SSL_KEY_PATH");

    if (use_ssl(ssl_cert, ssl_key_path))
        return std::make_shared<HttpsServer>(ip, port, gitlab_instance, ssl_cert, ssl_key_path);

    return std::make_shared<HttpServer>(ip, port, gitlab_instance);
}