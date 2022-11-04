#pragma once

#include <any>
#include <string>
#include <variant>

enum CmpType
{
    Greater = 0,
    Smaller,
    Equal,
    Not_Equal,
    None
};

using Value = std::any;

struct Condition {
    CmpType cmp_ = CmpType::None;

    std::variant<Value , std::string> cond_left_;
    std::variant<Value , std::string> cond_right_;
    bool left_is_attr = false;
    bool right_is_attr = false;
};

struct AttrInfo {
    std::string table_name_;
    std::string attr_name_;
};