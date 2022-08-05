#pragma once

#include <vector>

#include "CriterionListComparable.h"

namespace Contextual {

class CriterionIncludes : public CriterionListComparable {
public:
    CriterionIncludes(std::vector<int> options, bool invert);
    [[nodiscard]] bool compare(const std::unordered_set<int> &value) const override;
    [[nodiscard]] int getPriority() const override;
private:
    std::vector<int> m_options;
};

}