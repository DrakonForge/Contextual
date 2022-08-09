#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "RuleTable.h"

namespace Contextual {

enum class RuleDatabaseReturnCode : uint32_t {
    kSuccess = 0,
    kAlreadyDefined = 10
};

struct QueryBestResult {
    std::shared_ptr<Response> response;
    int priority;
};

class RuleDatabase {
public:
    explicit RuleDatabase(ContextManager& contextManager);
    virtual ~RuleDatabase() = default;
    RuleDatabaseReturnCode addRuleTable(const std::string& group, const std::string& category, std::unique_ptr<RuleTable>& ruleTable);
    const std::unique_ptr<RuleTable>& getRuleTable(const std::string& group, const std::string& category) const;
    ContextManager& getContextManager();
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
    ContextManager& m_contextManager;
    std::unordered_map<GroupCategory, std::unique_ptr<RuleTable>, GroupCategoryHash, GroupCategoryEquals> m_groupCategoryToTable;
};

}