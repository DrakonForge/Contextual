#include "RuleDatabase.h"

#include <utility>

namespace Contextual {

namespace {

const std::unique_ptr<RuleTable> g_NOT_FOUND = nullptr;

}

RuleDatabase::RuleDatabase(std::shared_ptr<ContextManager> contextManager) : m_contextManager(std::move(contextManager)) {}

RuleDatabaseReturnCode RuleDatabase::addRuleTable(const std::string& group, const std::string& category,
                                                  std::unique_ptr<RuleTable>& ruleTable) {
    GroupCategory groupCategory = {group, category};
    if (m_groupCategoryToTable.find(groupCategory) != m_groupCategoryToTable.end()) {
        return RuleDatabaseReturnCode::kAlreadyDefined;
    }

    m_groupCategoryToTable.emplace(groupCategory, std::move(ruleTable));
    return RuleDatabaseReturnCode::kSuccess;
}

const std::unique_ptr<RuleTable>& RuleDatabase::getRuleTable(const std::string& group,
                                                             const std::string& category) const {
    GroupCategory groupCategory = {group, category};
    auto got = m_groupCategoryToTable.find(groupCategory);
    if (got == m_groupCategoryToTable.end()) {
        return g_NOT_FOUND;
    }
    return got->second;
}

std::shared_ptr<ContextManager>& RuleDatabase::getContextManager() {
    return m_contextManager;
}

}  // namespace Contextual