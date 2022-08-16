#include "RuleTable.h"

#include <algorithm>
#include <limits>
#include <random>

#include "ResponseMultiple.h"
#include "ResponseSimple.h"

namespace Contextual {

namespace {
const std::shared_ptr<RuleEntry> g_NOT_FOUND = nullptr;

// Descending order
bool compareEntries(const std::shared_ptr<RuleEntry>& entry1, const std::shared_ptr<RuleEntry>& entry2) {
    return entry1->priority > entry2->priority;
}

// Return true if all criteria match, false otherwise
bool match(const DatabaseQuery& query, const std::vector<std::shared_ptr<Criteria>>& criteria) {
    return std::all_of(criteria.begin(), criteria.end(), [&query](const auto& criterionTuple) {
        return criterionTuple->criterion->evaluate(criterionTuple->table, criterionTuple->key, query);
    });
}

}  // namespace

void RuleTable::addEntry(std::shared_ptr<RuleEntry>& ruleEntry) {
    m_entries.push_back(std::move(ruleEntry));
    m_sorted = false;
}

void RuleTable::addEntries(const std::vector<std::shared_ptr<RuleEntry>>& ruleEntries) {
    m_entries.reserve(m_entries.size() + ruleEntries.size());
    m_entries.insert(m_entries.end(), ruleEntries.begin(), ruleEntries.end());
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
        if (match(query, entry->criteria)) {
            if (entry->priority > highestMatchingPriority) {
                highestMatchingPriority = entry->priority;
                candidates.clear();
            }
            candidates.push_back(entry->response);
        }
    }

    // Return random candidate
    if (candidates.empty()) {
        return {};
    }
    if (candidates.size() == 1) {
        return {candidates[0], highestMatchingPriority};
    }
    // TODO Improve RNG generation
    static std::default_random_engine e;
    std::uniform_int_distribution<size_t> dis(0, candidates.size() - 1);
    size_t index = dis(e);
    return {candidates[index], highestMatchingPriority};
}

UniformMatch RuleTable::queryUniform(const DatabaseQuery& query) const {
    std::vector<std::shared_ptr<Response>> options;
    for (const auto& entry : m_entries) {
        if (match(query, entry->criteria)) {
            options.push_back(entry->response);
        }
    }
    return {options};
}

WeightedMatch RuleTable::queryWeighted(const DatabaseQuery& query) const {
    std::vector<std::pair<std::shared_ptr<Response>, int>> weightedOptions;
    for (const auto& entry : m_entries) {
        if (match(query, entry->criteria)) {
            weightedOptions.emplace_back(std::make_pair(entry->response, entry->priority));
        }
    }
    return {weightedOptions};
}

SimpleUniformMatch RuleTable::querySimpleUniform(const DatabaseQuery& query,
                                                 const std::unordered_set<std::string>& skip, bool unique) const {
    std::vector<std::string> options;
    std::unordered_set<std::string> encountered;
    for (const auto& entry : m_entries) {
        if (match(query, entry->criteria)) {
            ResponseType type = entry->response->getType();
            std::shared_ptr<ResponseSimple> simpleResponse;
            if (type == ResponseType::kSimple) {
                simpleResponse = std::static_pointer_cast<ResponseSimple>(entry->response);
            } else if (type == ResponseType::kMultiple) {
                // Look one nested layer for simple
                const auto& multipleResponse = std::static_pointer_cast<ResponseMultiple>(entry->response);
                for (const auto& response : multipleResponse->getResponses()) {
                    if (response->getType() == ResponseType::kSimple) {
                        simpleResponse = std::static_pointer_cast<ResponseSimple>(response);
                        break;
                    }
                }
            }
            if (simpleResponse == nullptr) {
                continue;
            }

            // Add all
            for (const auto& option : simpleResponse->getOptions()) {
                if (skip.find(option) != skip.end() || (unique && encountered.find(option) != encountered.end())) {
                    continue;
                }
                if (unique) {
                    encountered.insert(option);
                }
                options.push_back(option);
            }
        }
    }
    return {options};
}

SimpleWeightedMatch RuleTable::querySimpleWeighted(const DatabaseQuery& query,
                                                   const std::unordered_set<std::string>& skip, bool unique) const {
    std::vector<std::pair<std::string, int>> weightedOptions;
    std::unordered_set<std::string> encountered;
    for (const auto& entry : m_entries) {
        if (match(query, entry->criteria)) {
            ResponseType type = entry->response->getType();
            std::shared_ptr<ResponseSimple> simpleResponse;
            if (type == ResponseType::kSimple) {
                simpleResponse = std::static_pointer_cast<ResponseSimple>(entry->response);
            } else if (type == ResponseType::kMultiple) {
                // Look one nested layer for simple
                const auto& multipleResponse = std::static_pointer_cast<ResponseMultiple>(entry->response);
                for (const auto& response : multipleResponse->getResponses()) {
                    if (response->getType() == ResponseType::kSimple) {
                        simpleResponse = std::static_pointer_cast<ResponseSimple>(response);
                        break;
                    }
                }
            }
            if (simpleResponse == nullptr) {
                continue;
            }

            // Add all
            for (const auto& option : simpleResponse->getOptions()) {
                if (skip.find(option) != skip.end() || (unique && encountered.find(option) != encountered.end())) {
                    continue;
                }
                if (unique) {
                    encountered.insert(option);
                }
                weightedOptions.emplace_back(std::make_pair(option, entry->priority));
            }
        }
    }
    return {weightedOptions};
}

size_t RuleTable::getNumEntries() const {
    return m_entries.size();
}

const std::vector<std::shared_ptr<RuleEntry>> RuleTable::getEntries() const {
    return m_entries;
}

}  // namespace Contextual