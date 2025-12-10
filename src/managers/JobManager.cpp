//
// Created by Eilon Hafzadi on 07/12/2025.
//

#include "JobManager.hpp"
#include "../Job.hpp"
#include "spdlog/spdlog.h"

void JobManager::add_job(const int &pipeline_id, Job &job) {
    jobs.insert({pipeline_id, job});
}

void JobManager::remove_job(const int &pipeline_id) {
    jobs.erase(pipeline_id);
}

Job &JobManager::get_job(const int &pipeline_id) {
    return jobs.at(pipeline_id);
}

Job JobManager::get_or_create(const int &pipeline_id, const nlohmann::json &job_body, const nlohmann::json &req_body) const {
    if (jobs.contains(pipeline_id))
        return jobs.at(pipeline_id);

    return Job{job_body, req_body, job_body.at("pipeline").at("project_id").get<int>()};
}