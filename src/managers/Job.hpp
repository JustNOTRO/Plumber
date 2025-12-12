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

    Job(const int &id, const int &project_id);

    ~Job() = default;

    void increase_retry_amount();

    [[nodiscard]] int get_project_id() const;

    [[nodiscard]] int get_id() const;

    void set_id(const int& new_id);

    [[nodiscard]] int get_retry_amount() const;

    [[nodiscard]] Status get_status() const;

    void set_status(const Status &new_job);

    [[nodiscard]] std::string get_name() const;

    void set_name(const std::string &new_name);

private:
    Status status;

    int id;

    int project_id;

    std::string name;

    int retry_amount;
};
