#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "ResponseSpeech.h"
#include "RuleTable.h"
#include "TextToken.h"

namespace Contextual {

enum class RuleDatabaseReturnCode : uint32_t { kSuccess = 0, kAlreadyDefined = 10 };

enum class QueryReturnCode { kSuccess, kFailure };

class RuleDatabase {
public:
    explicit RuleDatabase(std::shared_ptr<ContextManager> contextManager);
    virtual ~RuleDatabase() = default;
    RuleDatabaseReturnCode addRuleTable(const std::string& group, const std::string& category,
                                        std::unique_ptr<RuleTable>& ruleTable);
    QueryReturnCode queryBestMatch(BestMatch& bestMatch, DatabaseQuery& query) const;
    QueryReturnCode queryUniformMatch(UniformMatch& uniformMatch, DatabaseQuery& query) const;
    QueryReturnCode queryWeightedMatch(WeightedMatch& weightedMatch, DatabaseQuery& query) const;
    QueryReturnCode querySimpleUniformMatch(SimpleUniformMatch& simpleUniformMatch, DatabaseQuery& query,
                                            const std::unordered_set<std::string>& skip, bool unique) const;
    QueryReturnCode querySimpledWeightedMatch(SimpleWeightedMatch& simpleWeightedMatch, DatabaseQuery& query,
                                              const std::unordered_set<std::string>& skip, bool unique) const;
    const std::unique_ptr<RuleTable>& getRuleTable(const std::string& group, const std::string& category) const;
    std::shared_ptr<ContextManager>& getContextManager();

private:
    struct GroupCategory {
        std::string group;
        std::string category;
    };

    struct GroupCategoryHash {
        std::size_t operator()(const GroupCategory& gc) const {
            return std::hash<std::string>()(gc.group) ^ std::hash<std::string>()(gc.category);
        }
    };

    struct GroupCategoryEquals {
        bool operator()(const GroupCategory& gc1, const GroupCategory& gc2) const {
            return gc1.group == gc2.group && gc1.category == gc2.category;
        }
    };

    std::shared_ptr<ContextManager> m_contextManager;
    std::unordered_map<GroupCategory, std::unique_ptr<RuleTable>, GroupCategoryHash, GroupCategoryEquals>
        m_groupCategoryToTable;
};

}  // namespace Contextual