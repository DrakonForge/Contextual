#pragma once

#include <string>
#include <unordered_map>

#include "RuleTable.h"

namespace Contextual {

enum class RuleDatabaseReturnCode : uint32_t {
    kSuccess = 0,
    kAlreadyDefined = 10
};

class RuleDatabase {
public:
    RuleDatabase();
    virtual ~RuleDatabase() = default;
    RuleDatabaseReturnCode addRuleTable(const std::string& group, const std::string& category, const RuleTable& ruleTable);
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
    std::unordered_map<GroupCategory, RuleTable, GroupCategoryHash, GroupCategoryEquals> m_groupCategoryToTable;
};

}