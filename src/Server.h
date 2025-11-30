//
// Created by Eilon Hafzadi on 30/11/2025.
//

#ifndef PLUMBER_SERVER_H
#define PLUMBER_SERVER_H
#define USER_MENTION_PREFIX "@"

#include "httplib.h"
#include "yaml-cpp/yaml.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Server final : public httplib::Server {
public:
    Server(std::string &ip, std::uint16_t &port, YAML::Node &config);

    ~Server() override;

    void start();

private:
    std::string &ip;

    std::uint16_t &port;

    YAML::Node config;
};


#endif //PLUMBER_SERVER_H
