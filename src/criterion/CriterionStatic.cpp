#include "CriterionStatic.h"

namespace Contextual {

namespace {
const int g_PRIORITY = 3;
}

CriterionStatic::CriterionStatic(float min, float max, bool invert) : CriterionFloatComparable(invert), m_min(min), m_max(max) {}

bool CriterionStatic::compare(float value) const {
    return m_invert != (m_min <= value && value <= m_max);
}

int CriterionStatic::getPriority() const {
    return g_PRIORITY;
}



}