#include "RuleDatabase.h"

#include <utility>

#include "ResponseMultiple.h"
#include "ResponseSpeech.h"
#include "SpeechGenerator.h"

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

// TODO: Might want to switch to result so there is an error message, can also use a bool for this
QueryReturnCode RuleDatabase::queryBestSpeechLine(std::vector<std::shared_ptr<TextToken>>& speechLine,
                                                  DatabaseQuery& query) const {
    // Look for table
    const std::unique_ptr<RuleTable>& table = getRuleTable(query.getGroup(), query.getCategory());
    if(table == nullptr) {
        return QueryReturnCode::kFailure;
    }

    // Get best match for table
    BestMatch bestMatch = table->queryBest(query);
    if(bestMatch.response == nullptr) {
        return QueryReturnCode::kFailure;
    }

    // Look for speech responses; there should only be one maximum
    // There should never be a nested multiple response, so it is either top level or in a multiple response
    std::shared_ptr<ResponseSpeech> speechResponse;
    if(bestMatch.response->getType() == ResponseType::kSpeech) {
        speechResponse = std::static_pointer_cast<ResponseSpeech>(bestMatch.response);
    } else if(bestMatch.response->getType() == ResponseType::kMultiple) {
        const auto& multipleResponse = std::static_pointer_cast<ResponseMultiple>(bestMatch.response);
        for(const auto& response : multipleResponse->getResponses()) {
            if(response->getType() == ResponseType::kSpeech) {
                speechResponse = std::static_pointer_cast<ResponseSpeech>(response);
                break;
            }
        }
    } else {
        return QueryReturnCode::kFailure;
    }

    // Double-check just in case
    if(speechResponse == nullptr) {
        return QueryReturnCode::kFailure;
    }

    // We got one!
    SpeechGeneratorReturnCode result = SpeechGenerator::generateLine(speechLine, query, speechResponse->getRandomLine());
    if(result != SpeechGeneratorReturnCode::kSuccess) {
        return QueryReturnCode::kFailure;
    }
    return QueryReturnCode::kSuccess;
}

}  // namespace Contextual