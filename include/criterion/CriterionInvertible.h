#pragma once

#include "Criterion.h"

namespace Contextual {

class CriterionInvertible : public Criterion {
protected:
    explicit CriterionInvertible(bool invert);
    bool m_invert;
};

}  // namespace Contextual