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

    [[nodiscard]] const std::string& get_status() const;

    void set_status(const std::string &new_status);

    [[nodiscard]] int get_merge_request_id() const;

    void set_merge_request_id(int merge_req_id);

    [[nodiscard]] int get_comment_id() const;

    void set_comment_id(int new_comment_id);

private:
    int id;

    int project_id;

    std::string name;

    int retry_amount;

    std::string status;

    int merge_request_id = 0;

    int comment_id = 0;
};
