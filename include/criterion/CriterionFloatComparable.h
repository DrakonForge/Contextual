#pragma once

#include "CriterionInvertible.h"

namespace Contextual {

class CriterionFloatComparable : public CriterionInvertible {
public:
    virtual bool compare(float value) const = 0;
protected:
    explicit CriterionFloatComparable(bool invert);
};

}