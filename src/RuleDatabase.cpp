#include "RuleDatabase.h"

namespace Contextual {

namespace {

const std::unique_ptr<RuleTable> g_NOT_FOUND = nullptr;

}

RuleDatabase::RuleDatabase(ContextManager& contextManager) : m_contextManager(contextManager) {

}

RuleDatabaseReturnCode RuleDatabase::addRuleTable(const std::string& group,
                                                  const std::string& category,
                                                  std::unique_ptr<RuleTable>& ruleTable) {
    GroupCategory groupCategory = { group, category };
    if (m_groupCategoryToTable.find(groupCategory) != m_groupCategoryToTable.end()) {
        return RuleDatabaseReturnCode::kAlreadyDefined;
    }

    m_groupCategoryToTable.insert({groupCategory, std::move(ruleTable) });
    return RuleDatabaseReturnCode::kSuccess;
}

const std::unique_ptr<RuleTable>& RuleDatabase::getRuleTable(
    const std::string& group, const std::string& category) const {
    GroupCategory groupCategory = { group, category };
    if(m_groupCategoryToTable.find(groupCategory) == m_groupCategoryToTable.end()) {
        return g_NOT_FOUND;
    }
    return m_groupCategoryToTable.at(groupCategory);
}

ContextManager& RuleDatabase::getContextManager() {
    return m_contextManager;
}

}