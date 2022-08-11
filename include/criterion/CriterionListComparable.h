#pragma once

#include <unordered_set>

#include "CriterionInvertible.h"

namespace Contextual {

class CriterionListComparable : public CriterionInvertible {
public:
    virtual bool compare(const std::unordered_set<int>& value) const = 0;

protected:
    explicit CriterionListComparable(bool invert);
};

}  // namespace Contextual