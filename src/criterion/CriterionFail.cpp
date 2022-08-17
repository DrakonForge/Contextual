#include "CriterionFail.h"

#include "MathUtils.h"

namespace Contextual {

CriterionFail::CriterionFail(const float chanceToFail) : m_chanceToFail(chanceToFail) {}

int CriterionFail::getPriority() const {
    return 5;
}

bool CriterionFail::evaluate(const std::string& table, const std::string& key, const DatabaseQuery& query) const {
    DatabaseQuery::WillFail failType = query.willFail();
    if (failType == DatabaseQuery::WillFail::kNormal) {
        return MathUtils::randFloat(0.0f, 1.0f) >= m_chanceToFail;
    }
    if (failType == DatabaseQuery::WillFail::kNever) {
        return true;
    }
    // For "Always Fail" and any uncaught cases
    return false;
}

}  // namespace Contextual