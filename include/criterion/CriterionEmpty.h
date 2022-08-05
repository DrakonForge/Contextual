#pragma once

#include "CriterionListComparable.h"

namespace Contextual {

class CriterionEmpty : public CriterionListComparable {
public:
    explicit CriterionEmpty(bool invert);
    [[nodiscard]] bool evaluate(const std::string& table, const std::string& key, const DatabaseQuery& query) const override;
    [[nodiscard]] bool compare(const std::unordered_set<int> &value) const override;
    [[nodiscard]] int getPriority() const override;
};

}