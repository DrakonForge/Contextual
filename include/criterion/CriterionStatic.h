#pragma once

#include "CriterionFloatComparable.h"

namespace Contextual {

class CriterionStatic : public CriterionFloatComparable {
public:
    CriterionStatic(float min, float max, bool invert);
    [[nodiscard]] bool compare(float value) const override;
    [[nodiscard]] int getPriority() const override;
private:
    float m_min;
    float m_max;
};

}