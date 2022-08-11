#pragma once

#include <unordered_set>

#include "CriterionFloatComparable.h"

namespace Contextual {

class CriterionAlternate : public CriterionFloatComparable {
public:
    CriterionAlternate(std::unordered_set<int> options, bool invert);
    [[nodiscard]] bool evaluate(const std::string& table, const std::string& key,
                                const DatabaseQuery& query) const override;
    [[nodiscard]] bool compare(float value) const override;
    [[nodiscard]] int getPriority() const override;

private:
    std::unordered_set<int> m_options;
};

}  // namespace Contextual