#include "CriterionExist.h"

namespace Contextual {

CriterionExist::CriterionExist(const bool invert) : CriterionInvertible(invert) {}

bool CriterionExist::evaluate(const std::string& table, const std::string& key, const DatabaseQuery& query) const {
    std::shared_ptr<ContextTable> contextTable = query.getContextTable(table);
    if (contextTable != nullptr) {
        return m_invert != contextTable->hasKey(key);
    }
    return m_invert;
}

int CriterionExist::getPriority() const {
    return 4;
}

}  // namespace Contextual