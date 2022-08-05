#pragma once

#include <unordered_set>

#include "CriterionFloatComparable.h"

namespace Contextual {

class CriterionAlternate : public CriterionFloatComparable {
public:
    CriterionAlternate(std::unordered_set<int> options, bool invert);
    [[nodiscard]] bool compare(float value) const override;
    [[nodiscard]] int getPriority() const override;
private:
    std::unordered_set<int> m_options;
};

}