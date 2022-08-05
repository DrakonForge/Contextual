#include "CriterionDynamic.h"

#include <utility>

namespace Contextual {

CriterionDynamic::CriterionDynamic(const float min, const float max, std::string otherTable, std::string otherKey, const bool invert)
    : CriterionFloatComparable(invert), m_minDelta(min), m_maxDelta(max), m_otherTable(std::move(otherTable)), m_otherKey(std::move(otherKey)) {}

bool CriterionDynamic::evaluate(const std::string& table,
                               const std::string& key,
                               const DatabaseQuery& query) const {
    if(std::shared_ptr<ContextTable> contextTable1 = query.getContextTable(table)) {
        if(std::shared_ptr<ContextTable> contextTable2 = query.getContextTable(m_otherTable)) {
            if(std::optional<float> value1 = contextTable1->getRawValue(key)) {
                if(std::optional<float> value2 = contextTable2->getRawValue(m_otherKey)) {
                    const float delta = *value1 - *value2;
                    return compare(delta);
                }
            }
        }
    }
    return false;
}

bool CriterionDynamic::compare(float delta) const {
    return m_invert != (m_minDelta <= delta && delta <= m_maxDelta);
}

int CriterionDynamic::getPriority() const {
    return 3;
}

}