#pragma once

#include "Criterion.h"

namespace Contextual {

class CriterionFail : public Criterion {
public:
    CriterionFail(float chance);
    int getPriority() const override;
private:
    const float m_chance;
};

}