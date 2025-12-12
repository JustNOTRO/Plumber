//
// Created by Eilon Hafzadi on 01/12/2025.
//

#include "Config.hpp"
#include "spdlog/spdlog.h"

#include <filesystem>

Config::Config() {
    try {
        const std::string path = std::filesystem::current_path().parent_path().string() + "/config.yaml";
        this->node = YAML::LoadFile(path);
    } catch (YAML::ParserException &e) {
        spdlog::error("Could not load config: ", e.what());
        std::exit(1);
    }
}