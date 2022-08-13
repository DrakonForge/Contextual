#pragma once

#include "CriterionInvertible.h"

namespace Contextual {

class CriterionFloatComparable : public CriterionInvertible {
public:
    [[nodiscard]] virtual bool compare(float value) const = 0;

protected:
    explicit CriterionFloatComparable(bool invert);
};

}  // namespace Contextual