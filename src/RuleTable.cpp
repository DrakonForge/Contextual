#include "RuleTable.h"

#include <algorithm>
#include <limits>
#include <random>

namespace Contextual {

namespace {
const std::unique_ptr<RuleEntry> g_NOT_FOUND = nullptr;

// Descending order
bool compareEntries(const std::unique_ptr<RuleEntry>& entry1, const std::unique_ptr<RuleEntry>& entry2) {
    return entry1->priority > entry2->priority;
}

// Return true if all criteria match, false otherwise
bool match(const DatabaseQuery& query, const std::vector<std::shared_ptr<Criteria>>& criteria) {
    return std::all_of(criteria.begin(), criteria.end(), [&query](const auto& criterionTuple) {
        return criterionTuple->criterion->evaluate(criterionTuple->table, criterionTuple->key, query);
    });
}

}  // namespace

void RuleTable::addEntry(std::unique_ptr<RuleEntry>& ruleEntry) {
    m_entries.push_back(std::move(ruleEntry));
    m_sorted = false;
}

bool RuleTable::sortEntries() {
    if (m_sorted) {
        return false;
    }
    std::sort(m_entries.begin(), m_entries.end(), compareEntries);
    return true;
}

BestMatch RuleTable::queryBest(const DatabaseQuery& query) const {
    std::vector<std::shared_ptr<Response>> candidates;
    int highestMatchingPriority = std::numeric_limits<int>::min();
    for (const auto& entry : m_entries) {
        if (entry->priority < highestMatchingPriority) {
            break;
        }
        if(match(query, entry->criteria)) {
            if(entry->priority > highestMatchingPriority) {
                highestMatchingPriority = entry->priority;
                candidates.clear();
            }
            candidates.push_back(entry->response);
        }
    }

    // Return random candidate
    if(candidates.empty()) {
        return {};
    }
    if(candidates.size() == 1) {
        return {candidates[0], highestMatchingPriority};
    }
    // TODO Improve RNG generation
    static std::default_random_engine e;
    std::uniform_int_distribution<size_t> dis(0, candidates.size() - 1);
    size_t index = dis(e);
    return {candidates[index], highestMatchingPriority};
}

UniformMatch RuleTable::queryUniform(const DatabaseQuery& query) const {
    return {};
}

WeightedMatch RuleTable::queryWeighted(const DatabaseQuery& query) const {
    return {};
}

SimpleUniformMatch RuleTable::querySimpleUniform(const DatabaseQuery& query,
                                                 const std::unordered_set<std::string>& skip, bool unique) const {
    return {};
}

SimpleWeightedMatch RuleTable::querySimpleWeighted(const DatabaseQuery& query,
                                                   const std::unordered_set<std::string>& skip, bool unique) const {
    return {};
}

size_t RuleTable::getNumEntries() const {
    return m_entries.size();
}

}  // namespace Contextual