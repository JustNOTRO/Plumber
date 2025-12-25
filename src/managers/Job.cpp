//
// Created by Eilon Hafzadi on 07/12/2025.
//

#include "Job.hpp"

#include "spdlog/spdlog.h"

Job::Job(const int id, const int project_id) : id(id), project_id(project_id), retry_amount(0) {}

void Job::increase_retry_amount() {
    retry_amount++;
}

int Job::get_project_id() const {
    return project_id;
}

int Job::get_id() const {
    return id;
}

void Job::set_id(int new_id) {
    this->id = new_id;
}

int Job::get_retry_amount() const {
    return retry_amount;
}

const std::string &Job::get_name() const {
    return this->name;
}

void Job::set_name(const std::string &new_name) {
    this->name = new_name;
}

const std::string& Job::get_status() const {
    return status;
}

void Job::set_status(const std::string &new_status) {
    this->status = new_status;
}

int Job::get_merge_request_id() const {
    return merge_request_id;
}

void Job::set_merge_request_id(int merge_req_id) {
    this->merge_request_id = merge_req_id;
}

int Job::get_comment_id() const {
    return comment_id;
}

void Job::set_comment_id(int new_comment_id) {
    this->comment_id = new_comment_id;
}