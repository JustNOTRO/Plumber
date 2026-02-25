//
// Created by Eilon Hafzadi on 30/11/2025.
//

#pragma once

#include "httplib.h"
#include "spdlog/spdlog.h"

#include "../Server.hpp"

class HttpServer final : public Server {
public:
    HttpServer(const std::string &ip, const unsigned short port, const std::string &gitlab_instance) : Server(ip, port, gitlab_instance) {}

    void stop() override {
        server.stop();
    }

    bool bind_to_port(const std::string &ip, const unsigned short port) override {
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

private:
    httplib::Server server;
};
