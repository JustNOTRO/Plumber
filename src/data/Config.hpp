//
// Created by Eilon Hafzadi on 01/12/2025.
//

#pragma once

#include "yaml-cpp/yaml.h"
#include <optional>

class Config {
public:
    Config();

    template<typename T>
    std::optional<T> get_value(const std::string& key) const {
        if (!node[key])
            return std::nullopt;

        return std::make_optional(node[key].as<T>());
    }

private:
    YAML::Node node;
};
