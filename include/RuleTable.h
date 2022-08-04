#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Criterion.h"
#include "Response.h"

namespace Contextual {

struct Criteria {
    std::string table;
    std::string key;
    std::shared_ptr<Criterion> criterion;
};

struct RuleEntry {
    std::string name;
    std::vector<Criteria> criteria;
    std::shared_ptr<Response> response;
    int priority;
};

class RuleTable {
public:
    RuleTable() = default;
    virtual ~RuleTable() = default;
private:
    std::vector<RuleEntry> m_entries;
};

}