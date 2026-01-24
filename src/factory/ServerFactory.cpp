//
// Created by Eilon Hafzadi on 22/01/2026.
//

#include "ServerFactory.hpp"

#include "../server/http/HttpServer.hpp"
#include "../server/http/HttpsServer.hpp"

#include <fstream>

bool use_ssl(const char* cert, const char* cert_key) {

    if (!cert || !cert_key)
        return false;

    const std::ifstream cert_file(cert);
    const std::ifstream cert_key_file(cert_key);

    return cert_file.good() && cert_key_file.good();
}

std::unique_ptr<Server> ServerFactory::create(const std::string &ip, std::uint16_t port, const std::string &gitlab_instance) {
    const char* cert = std::getenv("CERT");
    const char* cert_key = std::getenv("CERT_KEY");

    if (use_ssl(cert, cert_key))
        return std::make_unique<HttpsServer>(ip, port, gitlab_instance, cert, cert_key);

    return std::make_unique<HttpServer>(ip, port, gitlab_instance);
}