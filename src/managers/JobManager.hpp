//
// Created by Eilon Hafzadi on 07/12/2025.
//

#pragma once

#include "../server/Job.hpp"

#include <unordered_map>
#include <nlohmann/json.hpp>

struct JobInfo;

class JobManager {

public:
    [[nodiscard]] Job create_job(const JobInfo &job_info);

    void add_job(int pipeline_id, Job &job);

    void remove_job(int pipeline_id);

    [[nodiscard]] std::optional<std::reference_wrapper<Job>> get_job(int pipeline_id);
private:
    std::unordered_map<int, Job> jobs = {};
};
