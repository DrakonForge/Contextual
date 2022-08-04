#include "RuleDatabase.h"

namespace Contextual {

RuleDatabase::RuleDatabase() {

}

RuleDatabaseReturnCode RuleDatabase::addRuleTable(const std::string& group,
                                                  const std::string& category,
                                                  const RuleTable& ruleTable) {
    GroupCategory groupCategory = { group, category };
    if (m_groupCategoryToTable.find(groupCategory) != m_groupCategoryToTable.end()) {
        return RuleDatabaseReturnCode::kAlreadyDefined;
    }

    m_groupCategoryToTable.insert({groupCategory, ruleTable });
    return RuleDatabaseReturnCode::kSuccess;
}

}