#include "RuleDatabase.h"

#include <utility>

#include "ResponseContext.h"
#include "ResponseMultiple.h"
#include "SpeechGenerator.h"

namespace Contextual {

namespace {

const std::unique_ptr<RuleTable> g_NOT_FOUND = nullptr;

}  // namespace

RuleDatabase::RuleDatabase(std::shared_ptr<ContextManager> contextManager)
    : m_contextManager(std::move(contextManager)) {}

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

QueryReturnCode RuleDatabase::queryBestMatch(BestMatch& bestMatch, DatabaseQuery& query) const {
    // Look for table
    const std::unique_ptr<RuleTable>& table = getRuleTable(query.getGroup(), query.getCategory());
    if (table == nullptr) {
        return QueryReturnCode::kFailure;
    }

    // Get best match for table
    bestMatch = table->queryBest(query);
    if (bestMatch.response == nullptr) {
        return QueryReturnCode::kFailure;
    }
    return QueryReturnCode::kSuccess;
}

QueryReturnCode RuleDatabase::queryUniformMatch(UniformMatch& uniformMatch, DatabaseQuery& query) const {
    // Set query to skip fail criterion
    query.setWillFail(DatabaseQuery::WillFail::kNever);

    // Look for table
    const std::unique_ptr<RuleTable>& table = getRuleTable(query.getGroup(), query.getCategory());
    if (table == nullptr) {
        return QueryReturnCode::kFailure;
    }

    uniformMatch = table->queryUniform(query);
    if (uniformMatch.options.empty()) {
        return QueryReturnCode::kFailure;
    }
    return QueryReturnCode::kSuccess;
}

QueryReturnCode RuleDatabase::queryWeightedMatch(WeightedMatch& weightedMatch, DatabaseQuery& query) const {
    // Set query to skip fail criterion
    query.setWillFail(DatabaseQuery::WillFail::kNever);

    // Look for table
    const std::unique_ptr<RuleTable>& table = getRuleTable(query.getGroup(), query.getCategory());
    if (table == nullptr) {
        return QueryReturnCode::kFailure;
    }

    weightedMatch = table->queryWeighted(query);
    if (weightedMatch.weightedOptions.empty()) {
        return QueryReturnCode::kFailure;
    }
    return QueryReturnCode::kSuccess;
}

QueryReturnCode RuleDatabase::querySimpleUniformMatch(SimpleUniformMatch& simpleUniformMatch, DatabaseQuery& query,
                                                      const std::unordered_set<std::string>& skip, bool unique) const {
    // Set query to skip fail criterion
    query.setWillFail(DatabaseQuery::WillFail::kNever);

    // Look for table
    const std::unique_ptr<RuleTable>& table = getRuleTable(query.getGroup(), query.getCategory());
    if (table == nullptr) {
        return QueryReturnCode::kFailure;
    }

    simpleUniformMatch = table->querySimpleUniform(query, skip, unique);
    if (simpleUniformMatch.options.empty()) {
        return QueryReturnCode::kFailure;
    }
    return QueryReturnCode::kSuccess;
}

QueryReturnCode RuleDatabase::querySimpledWeightedMatch(SimpleWeightedMatch& simpleWeightedMatch, DatabaseQuery& query,
                                                        const std::unordered_set<std::string>& skip,
                                                        bool unique) const {
    // Set query to skip fail criterion
    query.setWillFail(DatabaseQuery::WillFail::kNever);

    // Look for table
    const std::unique_ptr<RuleTable>& table = getRuleTable(query.getGroup(), query.getCategory());
    if (table == nullptr) {
        return QueryReturnCode::kFailure;
    }

    simpleWeightedMatch = table->querySimpleWeighted(query, skip, unique);
    if (simpleWeightedMatch.weightedOptions.empty()) {
        return QueryReturnCode::kFailure;
    }
    return QueryReturnCode::kSuccess;
}

}  // namespace Contextual