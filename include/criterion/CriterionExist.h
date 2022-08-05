#pragma once

#include "CriterionInvertible.h"

namespace Contextual {

class CriterionExist : public CriterionInvertible {
public:
    explicit CriterionExist(bool invert);
    [[nodiscard]] int getPriority() const override;
};

}