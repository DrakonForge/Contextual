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
    Criteria(std::string table, std::string key, std::shared_ptr<Criterion> criterion)
        : table(std::move(table)), key(std::move(key)), criterion(std::move(criterion)) {}
};

struct RuleEntry {
    // ID should be in the form Group.Category.Name or Group.Category.Id0000
    std::string id;
    std::vector<std::shared_ptr<Criteria>> criteria;
    std::shared_ptr<Response> response;
    int priority;
};

struct BestMatch {
    std::shared_ptr<Response> response;
    int priority;
};

struct UniformMatch {
    std::vector<std::shared_ptr<Response>> options;
};

struct WeightedMatch {
    std::vector<std::pair<std::shared_ptr<Response>, int>> weightedOptions;
};

struct SimpleUniformMatch {
    std::vector<std::string> options;
};

struct SimpleWeightedMatch {
    std::vector<std::pair<std::string, int>> weightedOptions;
};

class RuleTable {
public:
    RuleTable() = default;
    virtual ~RuleTable() = default;
    void addEntry(std::unique_ptr<RuleEntry>& ruleEntry);
    bool sortEntries();
    [[nodiscard]] BestMatch queryBest(const DatabaseQuery& query) const;
    [[nodiscard]] UniformMatch queryUniform(const DatabaseQuery& query) const;
    [[nodiscard]] WeightedMatch queryWeighted(const DatabaseQuery& query) const;
    [[nodiscard]] SimpleUniformMatch querySimpleUniform(const DatabaseQuery& query, const std::unordered_set<std::string>& skip, bool unique) const;
    [[nodiscard]] SimpleWeightedMatch querySimpleWeighted(const DatabaseQuery& query, const std::unordered_set<std::string>& skip, bool unique) const;
    [[nodiscard]] size_t getNumEntries() const;

private:
    std::vector<std::unique_ptr<RuleEntry>> m_entries;
    bool m_sorted = false;
};

}  // namespace Contextual