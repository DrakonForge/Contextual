#pragma once

#include "CriterionListComparable.h"

namespace Contextual {

class CriterionEmpty : public CriterionListComparable {
public:
    explicit CriterionEmpty(bool invert);
    bool compare(const std::unordered_set<int> &value) const override;
    [[nodiscard]] int getPriority() const override;
};

}