//
// Created by Eilon Hafzadi on 10/01/2026.
//

#pragma once

#include <optional>
#include <string>

class ServerUtils {
    public:
        ServerUtils() = delete;

        static std::string require_env(const char *name);

        static unsigned short require_port(const char *name);

        static std::string require_url_scheme(const char *name);

        static int require_retry_amount(const char *name);

        static std::optional<std::string> get_env(const char *name);
};
