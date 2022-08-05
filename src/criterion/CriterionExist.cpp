#include "CriterionExist.h"

namespace Contextual {

CriterionExist::CriterionExist(const bool invert) : CriterionInvertible(invert) {}

int CriterionExist::getPriority() const {
    return 4;
}

}