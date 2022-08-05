#pragma once

#include "CriterionInvertible.h"

namespace Contextual {

class CriterionExist : public CriterionInvertible {
public:
    explicit CriterionExist(bool invert);
    [[nodiscard]] bool evaluate(const std::string& table, const std::string& key, const DatabaseQuery& query) const override;
    [[nodiscard]] int getPriority() const override;
};

}