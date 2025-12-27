//
// Created by Eilon Hafzadi on 07/12/2025.
//

#include "JobManager.hpp"
#include "Job.hpp"
#include "../server/Server.hpp"

struct JobInfo;

Job JobManager::create_job(const JobInfo &job_info) {
    const int id = job_info.id;
    const int project_id = job_info.project_id;

    return {id, project_id};
}

void JobManager::add_job(int pipeline_id, Job &job) {
    jobs.insert({pipeline_id, job});
}

void JobManager::remove_job(int pipeline_id) {
    jobs.erase(pipeline_id);
}

std::optional<std::reference_wrapper<Job>> JobManager::get_job(int pipeline_id) {
    if (!jobs.contains(pipeline_id))
        return std::nullopt;

    return std::make_optional(std::ref(jobs.at(pipeline_id)));
}