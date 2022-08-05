#include "CriterionStatic.h"

namespace Contextual {

CriterionStatic::CriterionStatic(const float min, const float max, const bool invert) : CriterionFloatComparable(invert), m_min(min), m_max(max) {}

bool CriterionStatic::compare(float value) const {
    return m_invert != (m_min <= value && value <= m_max);
}

int CriterionStatic::getPriority() const {
    return 3;
}

}