//
// Created by Eilon Hafzadi on 05/12/2025.
//

#include "Comment.hpp"

#include "spdlog/spdlog.h"

Comment::Comment(const nlohmann::basic_json<> &obj_attributes) : obj_attributes(obj_attributes) {}

Comment::~Comment() {

}

Comment::CommentType Comment::get_type() const {
    const std::string noteable_type = obj_attributes.at("noteable_type");
    if (noteable_type == "MergeRequest")
        return MERGE_REQUEST;

    if (noteable_type == "Issue")
        return ISSUE;

    return UNKNOWN;
}
