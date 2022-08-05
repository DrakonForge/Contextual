#pragma once

#include "Criterion.h"

namespace Contextual {

class CriterionFail : public Criterion {
public:
    explicit CriterionFail(float chance);
    [[nodiscard]] int getPriority() const override;
private:
    const float m_chance;
};

}