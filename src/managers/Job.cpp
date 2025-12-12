//
// Created by Eilon Hafzadi on 07/12/2025.
//

#include "Job.hpp"

#include "spdlog/spdlog.h"

Job::Job(const int &id, const int &project_id) : status(Status::CREATED), id(id), project_id(project_id), retry_amount(1) {

}

void Job::increase_retry_amount() {
    retry_amount++;
}

int Job::get_project_id() const {
    return project_id;
}

int Job::get_id() const {
    return id;
}

void Job::set_id(const int &new_id) {
    this->id = new_id;
}

int Job::get_retry_amount() const {
    return retry_amount;
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