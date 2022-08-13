#pragma once

#include "Criterion.h"

namespace Contextual {

class CriterionFail : public Criterion {
public:
    explicit CriterionFail(float chanceToFail);
    [[nodiscard]] bool evaluate(const std::string& table, const std::string& key,
                                const DatabaseQuery& query) const override;
    [[nodiscard]] int getPriority() const override;

private:
    const float m_chanceToFail;
};

}  // namespace Contextual