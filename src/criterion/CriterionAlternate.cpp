#include "CriterionAlternate.h"

#include <utility>

namespace Contextual {

CriterionAlternate::CriterionAlternate(std::unordered_set<int> options, bool invert) : CriterionFloatComparable(invert), m_options(std::move(options)) {}

bool CriterionAlternate::compare(float value) const {
    return m_invert == (m_options.find(static_cast<int>(value)) == m_options.end());
}

int CriterionAlternate::getPriority() const {
    return 2;
}

}