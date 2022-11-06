#pragma once

#include <vector>

#include "sql_defs.hpp"

struct Select {
  std::vector<Condition> conditions_;
  std::vector<RelAttr> select_lists_;
  std::vector<std::string> tables_;

  Select() = default;

  Select(const Select &) = default;

  Select &operator=(const Select &s)
  {
    conditions_ = s.conditions_;
    select_lists_ = s.select_lists_;
    tables_ = s.tables_;
    return *this;
  }

  Select(Select &&s)
      : conditions_(std::move(s.conditions_)), select_lists_(std::move(s.select_lists_)), tables_(std::move(s.tables_))
  {}

  Select &operator=(Select &&s)
  {
    conditions_ = std::move(s.conditions_);
    select_lists_ = std::move(s.select_lists_);
    tables_ = std::move(s.tables_);
    return *this;
  }
};

struct CreateTable {
  std::string table_name_;
};

struct Insert {};

struct Update {};

struct Query {};