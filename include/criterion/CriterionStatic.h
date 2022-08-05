#pragma once

#include "CriterionFloatComparable.h"

namespace Contextual {

class CriterionStatic : public CriterionFloatComparable {
public:
    CriterionStatic(float min, float max, bool invert);
    [[nodiscard]] bool evaluate(const std::string& table, const std::string& key, const DatabaseQuery& query) const override;
    [[nodiscard]] bool compare(float value) const override;
    [[nodiscard]] int getPriority() const override;
private:
    const float m_min;
    const float m_max;
};

}