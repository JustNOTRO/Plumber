//
// Created by Eilon Hafzadi on 05/12/2025.
//

#pragma once

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Comment {
public:
    enum CommentType {
        MERGE_REQUEST,
        ISSUE,
        UNKNOWN
    };

    Comment(const nlohmann::basic_json<> &obj_attributes);

    ~Comment();

    CommentType get_type() const;

private:
    const nlohmann::basic_json<> &obj_attributes;
};
