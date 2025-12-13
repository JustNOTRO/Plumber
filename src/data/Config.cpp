//
// Created by Eilon Hafzadi on 01/12/2025.
//

#include "Config.hpp"
#include "spdlog/spdlog.h"

Config::Config() {
    try {
        const std::string path = std::getenv("CONFIG_PATH") ? std::getenv("CONFIG_PATH") : "../config.yaml";
        this->node = YAML::LoadFile(path);
    } catch (YAML::ParserException &e) {
        spdlog::error("Could not load config: ", e.what());
        std::exit(1);
    }
}