#include "CriterionExist.h"

namespace Contextual {

CriterionExist::CriterionExist(bool invert) : CriterionInvertible(invert) {}

int CriterionExist::getPriority() const {
    return 4;
}

}