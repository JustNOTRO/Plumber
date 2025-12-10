//
// Created by Eilon Hafzadi on 07/12/2025.
//

#include "Job.hpp"

Job::Job(const nlohmann::json &job_body, const nlohmann::json &req_body, const int &project_id)
: req_body(req_body), job_body(job_body), project_id(project_id) {

}

void Job::increase_retry_amount(const int &increase_by) {
    cur_retry_amount += increase_by;
}

int Job::get_project_id() const {
    return project_id;
}

void Job::set_project_id(const int &new_project_id) {
    this->project_id = new_project_id;
}

int Job::get_id() const {
    return id;
}

void Job::set_id(const int &new_id) {
    this->id = new_id;
}

int Job::get_retry_amount() const {
    return cur_retry_amount;
}

Job::Status Job::get_status() const {
    return this->status;
}

void Job::set_status(const Status &new_job) {
    this->status = new_job;
}

std::string Job::get_name() const {
    return this->name;
}

void Job::set_name(const std::string &new_name) {
    this->name = new_name;
}