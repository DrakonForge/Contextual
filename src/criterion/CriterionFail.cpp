#include "CriterionFail.h"

#include <random>

namespace Contextual {

CriterionFail::CriterionFail(const float chanceToFail) : m_chanceToFail(chanceToFail) {}

int CriterionFail::getPriority() const {
    return 5;
}

bool CriterionFail::evaluate(const std::string& table, const std::string& key, const DatabaseQuery& query) const {
    // TODO Improve RNG generation
    // https://stackoverflow.com/questions/28653255/stddefault-random-engine-generates-the-same-values-even-with-changing-seed
    static std::default_random_engine e;
    static std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    DatabaseQuery::WillFail failType = query.willFail();
    if (failType == DatabaseQuery::WillFail::kNormal) {
        return dis(e) >= m_chanceToFail;
    }
    if (failType == DatabaseQuery::WillFail::kNever) {
        return true;
    }
    // For "Always Fail" and any uncaught cases
    return false;
}

}  // namespace Contextual