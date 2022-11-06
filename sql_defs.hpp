#pragma once

#include <any>
#include <string>
#include <variant>

enum CmpType { Greater = 0, Smaller, Equal, Not_Equal, None };

using Value = std::any;

struct RelAttr {
  std::string table_name_;
  std::string attr_name_;
};

struct Condition {
  CmpType cmp_ = CmpType::None;

  std::any left_;
  std::any right_;
  bool left_is_attr = false;
  bool right_is_attr = false;
};
