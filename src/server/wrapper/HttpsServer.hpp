//
// Created by Eilon Hafzadi on 22/01/2026.
//

#pragma once

#include "../Server.hpp"

class HttpsServer final : public Server {
public:
    HttpsServer(const std::string &ip,
        const std::uint16_t port,
        const std::string &gitlab_instance,
        const std::string &certificate,
        const std::string &cert_key
        ) : Server(ip, port, gitlab_instance), server(certificate.c_str(), cert_key.c_str()) {}

    void stop() override {
        server.stop();
    }

    bool bind_to_port(const std::string &ip, const std::uint16_t port) override {
        return server.bind_to_port(ip, port);
    }

    bool listen_after_bind() override {
        return server.listen_after_bind();
    }

    httplib::Server &Get(const std::string &pattern, const Handler handler) override {
        return server.Get(pattern, handler);
    }

    httplib::Server &Post(const std::string &pattern, const Handler handler) override {
        return server.Post(pattern, handler);
    }

    httplib::Server &Post(const std::string &pattern, HandlerWithContentReader handler) override {
        return server.Post(pattern, handler);
    }

    httplib::Server &Put(const std::string &pattern, const Handler handler) override {
        return server.Put(pattern, handler);
    }

    httplib::Server &Put(const std::string &pattern, HandlerWithContentReader handler) override {
        return server.Put(pattern, handler);
    }

    httplib::Server &Patch(const std::string &pattern, const Handler handler) override {
        return server.Patch(pattern, handler);
    }

    httplib::Server &Patch(const std::string &pattern, HandlerWithContentReader handler) override {
        return server.Patch(pattern, handler);
    }

    httplib::Server &Delete(const std::string &pattern, const Handler handler) override {
        return server.Delete(pattern, handler);
    }

    httplib::Server &Delete(const std::string &pattern, HandlerWithContentReader handler) override {
        return server.Delete(pattern, handler);
    }

    httplib::Server &Options(const std::string &pattern, const Handler handler) override {
        return server.Options(pattern, handler);
    }

    bool update_certs_pem(const char *cert_pem, const char *key_pem,
                        const char *client_ca_pem = nullptr,
                        const char *password = nullptr) {
        return server.update_certs_pem(cert_pem, key_pem, client_ca_pem, password);
    }

private:
    httplib::SSLServer server;
};
