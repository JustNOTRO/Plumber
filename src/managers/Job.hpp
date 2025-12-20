//
// Created by Eilon Hafzadi on 07/12/2025.
//

#pragma once
#include <nlohmann/json.hpp>

class Job {

public:
    Job(int id, int project_id);

    void increase_retry_amount();

    [[nodiscard]] int get_project_id() const;

    [[nodiscard]] int get_id() const;

    void set_id(int new_id);

    [[nodiscard]] int get_retry_amount() const;

    [[nodiscard]] const std::string& get_name() const;

    void set_name(const std::string &new_name);

    const std::string& get_status();

    void set_status(const std::string &new_status);

private:
    int id;

    int project_id;

    std::string name;

    int retry_amount;

    std::string status;
};
