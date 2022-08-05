#pragma once

#include "CriterionInvertible.h"

namespace Contextual {

class CriterionExist : public CriterionInvertible {
public:
    CriterionExist(bool invert);
    int getPriority() const override;
};

}