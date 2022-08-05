#include "CriterionStatic.h"

#include <memory>
#include <optional>

#include "ContextTable.h"

namespace Contextual {

CriterionStatic::CriterionStatic(const float min, const float max, const bool invert) : CriterionFloatComparable(invert), m_min(min), m_max(max) {}

bool CriterionStatic::evaluate(const std::string& table,
                               const std::string& key,
                               const DatabaseQuery& query) const {
    std::shared_ptr<ContextTable> contextTable = query.getContextTable(table);
    if(contextTable != nullptr) {
        std::optional<float> value = contextTable->getRawValue(key);
        if(value) {
            return compare(*value);
        }
    }
    return false;
}

bool CriterionStatic::compare(float value) const {
    return m_invert != (m_min <= value && value <= m_max);
}

int CriterionStatic::getPriority() const {
    return 3;
}

}