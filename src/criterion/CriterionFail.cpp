#include "CriterionFail.h"

namespace Contextual {

CriterionFail::CriterionFail(const float chance) : m_chance(chance) {}

int CriterionFail::getPriority() const {
    return 5;
}

}