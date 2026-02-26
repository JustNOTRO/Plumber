//
// Created by Eilon Hafzadi on 10/01/2026.
//

#include "ServerUtils.hpp"
#include "spdlog/spdlog.h"

std::string ServerUtils::require_env(const char *name) {
    const char *v = std::getenv(name);
    if (!v) {
        spdlog::error("{} is not set", name);
        std::exit(1);
    }

    return v;
}

std::uint16_t ServerUtils::require_port(const char *name) {
    std::string v = require_env(name);

    try {
        return std::stoul(v);
    } catch (std::exception &e) {
        spdlog::error("could not parse {}, {}", v, e.what());
        std::exit(1);
    }
}

std::string ServerUtils::require_url_scheme(const char *name) {
    std::string env = require_env(name);
    if (!env.starts_with("http://") && !env.starts_with("https://")) {
        spdlog::error("could not parse {}, invalid URL scheme", name);
        std::exit(1);
    }

    return env;
}

int ServerUtils::require_retry_amount(const char *name) {
    const std::string env = require_env(name);

    try {
        return std::stoi(env);
    } catch (std::exception &e) {
        spdlog::error("could not parse {} {}", name, e.what());
        std::exit(1);
    }
}

std::optional<std::string> ServerUtils::get_env(const char *name) {
    const char *v = std::getenv(name);
    if (!v)
        return std::nullopt;

    return std::make_optional(v);
}