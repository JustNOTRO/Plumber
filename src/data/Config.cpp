//
// Created by Eilon Hafzadi on 01/12/2025.
//

#include "Config.hpp"
#include "spdlog/spdlog.h"

#include <filesystem>

Config::Config() {
    try {
        const std::string path = std::filesystem::current_path().parent_path().string() + "/src/data/config.yaml";
        this->node = YAML::LoadFile(path);
    } catch (std::exception &e) {
        spdlog::error("Exception caught: {}", e.what());
        std::exit(1);
    }
}

Config::~Config() {
    delete &node;
}