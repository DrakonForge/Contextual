#pragma once

#include <string>

#include "CriterionFloatComparable.h"

namespace Contextual {

class CriterionDynamic : public CriterionFloatComparable {
public:
    CriterionDynamic(float min, float max, std::string  otherTable, std::string  otherName, bool invert);
    [[nodiscard]] bool evaluate(const std::string& table, const std::string& key, const DatabaseQuery& query) const override;
    [[nodiscard]] bool compare(float delta) const override;
    [[nodiscard]] int getPriority() const override;
private:
    const float m_minDelta;
    const float m_maxDelta;
    const std::string m_otherTable;
    const std::string m_otherKey;
};

}