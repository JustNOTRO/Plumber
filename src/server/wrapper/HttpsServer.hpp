//
// Created by Eilon Hafzadi on 22/01/2026.
//

#pragma once

#include "../../listeners/FileModifiedListener.hpp"
#include "../Server.hpp"
#include "../../utils/ServerUtils.hpp"
#include "efsw/efsw.hpp"

struct CertWatcher {
    efsw::FileWatcher watcher;
    FileModifiedListener listener;
    std::string directory;

    explicit CertWatcher(const std::weak_ptr<Server> &server) : listener(server) {
        const char *cert = std::getenv("SSL_KEY_PATH");
        if (cert == nullptr)
            return;

        const auto file_path = std::filesystem::path(cert);

        const std::string dir = file_path.parent_path().string();
        directory = dir;

        watcher.addWatch(directory, &listener, false);
    }

    ~CertWatcher() {
        stop();
    }

    void watch() {
        watcher.watch();
    }

    void stop() {
        watcher.removeWatch(directory);
    }
};

class HttpsServer : public Server {
public:
    HttpsServer(
        const std::string &ip,
        const unsigned short port,
        const std::string &gitlab_instance,
        const std::string &certificate,
        const std::string &cert_key) : Server(ip, port, gitlab_instance), server(certificate.c_str(), cert_key.c_str()
    ) {}

    void start() override {
        std::weak_ptr<Server> weak_server = weak_from_this();
        const auto cert_watcher = std::make_unique<CertWatcher>(weak_server);
        cert_watcher->watch();

        Server::start();
    }

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

    httplib::Server &set_payload_max_length(size_t length) override {
        return server.set_payload_max_length(length);
    }

    bool update_certs_pem(const char *cert_pem, const char *key_pem,
                        const char *client_ca_pem = nullptr,
                        const char *password = nullptr) const {
        httplib::tls::ctx_t tls_ctx = server.tls_context();

        if (!tls_ctx || !cert_pem || !key_pem) { return false; }
        auto ssl_ctx = static_cast<SSL_CTX *>(tls_ctx);

        // Load certificate from PEM
        auto cert_bio = BIO_new_mem_buf(cert_pem, -1);
        if (!cert_bio) { return false; }
        auto cert = PEM_read_bio_X509(cert_bio, nullptr, nullptr, nullptr);
        BIO_free(cert_bio);
        if (!cert) { return false; }

        // Load private key from PEM
        auto key_bio = BIO_new_mem_buf(key_pem, -1);
        if (!key_bio) {
            X509_free(cert);
            return false;
        }
        auto key = PEM_read_bio_PrivateKey(key_bio, nullptr, nullptr,
                                           password ? const_cast<char *>(password)
                                                    : nullptr);
        BIO_free(key_bio);
        if (!key) {
            X509_free(cert);
            return false;
        }

        // Update certificate and key
        const std::string cert_path = ServerUtils::require_env("SSL_CERT_PATH");

        auto ret = SSL_CTX_use_certificate_chain_file(ssl_ctx, cert_path.c_str()) == 1 &&
                   SSL_CTX_use_PrivateKey(ssl_ctx, key) == 1;

        X509_free(cert);
        EVP_PKEY_free(key);
        return ret;
    }

private:
    httplib::SSLServer server;
};
