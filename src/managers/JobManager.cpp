//
// Created by Eilon Hafzadi on 07/12/2025.
//

#include "JobManager.hpp"
#include "Job.hpp"
#include "../server/Server.hpp"

Job& JobManager::create_job(int pipeline_id, const nlohmann::json &job_body) {
    const int id = job_body.at("id").get<int>();
    const int project_id = job_body.at("pipeline").at("project_id").get<int>();

    auto job = Job(id, project_id);
    jobs.insert({pipeline_id, job});
    return jobs.at(pipeline_id);
}

void JobManager::remove_job(int pipeline_id) {
    jobs.erase(pipeline_id);
}

std::expected<std::reference_wrapper<Job>, std::string> JobManager::get_job(int pipeline_id) {
    if (!jobs.contains(pipeline_id))
        return std::unexpected("job not found");

    return jobs.at(pipeline_id);
}