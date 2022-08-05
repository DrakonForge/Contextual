#include "CriterionDynamic.h"

#include <utility>

namespace Contextual {

CriterionDynamic::CriterionDynamic(const float min, const float max, std::string otherTable, std::string otherKey, const bool invert)
    : CriterionFloatComparable(invert), m_minDelta(min), m_maxDelta(max), m_otherTable(std::move(otherTable)), m_otherKey(std::move(otherKey)) {}

bool CriterionDynamic::compare(float delta) const {
    return m_invert != (m_minDelta <= delta && delta <= m_maxDelta);
}

int CriterionDynamic::getPriority() const {
    return 3;
}

}