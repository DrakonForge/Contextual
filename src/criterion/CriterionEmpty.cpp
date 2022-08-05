#include "CriterionEmpty.h"

namespace Contextual {

CriterionEmpty::CriterionEmpty(bool invert) : CriterionListComparable(invert) {}

bool CriterionEmpty::compare(const std::unordered_set<int>& value) const {
    return m_invert != value.empty();
}

int CriterionEmpty::getPriority() const {
    return 4;
}

}