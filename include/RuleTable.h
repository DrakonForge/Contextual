#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "criterion/Criterion.h"
#include "DatabaseQuery.h"
#include "response/Response.h"

namespace Contextual {

struct Criteria {
    std::string table;
    std::string key;
    std::shared_ptr<Criterion> criterion;
    Criteria(std::string table, std::string key, std::shared_ptr<Criterion> criterion) : table(std::move(table)), key(std::move(key)), criterion(std::move(criterion)) {}
};

struct RuleEntry {
    // ID should be in the form Group.Category.Name or Group.Category.Id0000
    std::string id;
    std::vector<std::shared_ptr<Criteria>> criteria;
    std::shared_ptr<Response> response;
    int priority;
};

class RuleTable {
public:
    RuleTable() = default;
    virtual ~RuleTable() = default;
    void addEntry(std::unique_ptr<RuleEntry>& ruleEntry);
    bool sortEntries();
    const std::unique_ptr<RuleEntry>& query(DatabaseQuery query);
    size_t getNumEntries();
private:
    std::vector<std::unique_ptr<RuleEntry>> m_entries;
    bool m_sorted = false;
};

}