//
// Created by Eilon Hafzadi on 07/12/2025.
//

#pragma once

#include "Job.hpp"

#include <unordered_map>
#include <nlohmann/json.hpp>

class JobManager {
public:
    Job create_job(const int &pipeline_id, const nlohmann::json &job_body);

    void remove_job(const int &pipeline_id);

    Job& get_job(const int &pipeline_id);
private:
    std::unordered_map<int, Job> jobs = {};
};
