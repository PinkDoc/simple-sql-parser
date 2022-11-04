#pragma once

#include "sql_defs.hpp"
#include <vector>

struct Select {
    std::vector<Condition> conditions_;
    std::vector<AttrInfo> attr_infos_;
    std::vector<std::string> tables_;
};