//
// Created by Eilon Hafzadi on 07/12/2025.
//

#pragma once

#include "../Job.hpp"

#include <unordered_map>
#include <nlohmann/json.hpp>

class JobManager {
public:
    void add_job(const int &pipeline_id, Job &job);

    void remove_job(const int &pipeline_id);

    Job &get_job(const int &pipeline_id);

    [[nodiscard]] Job get_or_create(const int &pipeline_id, const nlohmann::json &job_body) const;

private:
    std::unordered_map<int, Job> jobs = {};
};
