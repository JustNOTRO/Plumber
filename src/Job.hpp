//
// Created by Eilon Hafzadi on 07/12/2025.
//

#pragma once
#include <nlohmann/json.hpp>

class Job {

public:
    enum class Status {
        CREATED,
        PENDING,
        RUNNING,
        FAILED,
        SUCCESS
    };

    Job(const nlohmann::json &job_body, const nlohmann::json &req_body, const int &project_id);

    ~Job() = default;

    void increase_retry_amount(const int &increase_by);

    [[nodiscard]] int get_project_id() const;

    void set_project_id(const int &new_project_id);

    [[nodiscard]] int get_id() const;

    [[nodiscard]] int get_retry_amount() const;

    Status get_status() const;

    void set_status(const Status &new_job);

    std::string get_name() const;

    void set_name(const std::string &new_name);

    void set_id(const int& new_id);


private:
    const nlohmann::json &req_body;

    const nlohmann::json &job_body;

    int cur_retry_amount = 1;

    Status status;

    int id = job_body.at("id");

    int project_id;

    std::string name;
};
