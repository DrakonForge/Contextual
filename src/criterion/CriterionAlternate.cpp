#include "CriterionAlternate.h"

#include <utility>

namespace Contextual {

CriterionAlternate::CriterionAlternate(std::unordered_set<int> options, bool invert) : CriterionFloatComparable(invert), m_options(std::move(options)) {}

bool CriterionAlternate::evaluate(const std::string& table,
                               const std::string& key,
                               const DatabaseQuery& query) const {
    std::shared_ptr<ContextTable> contextTable = query.getContextTable(table);
    if(contextTable != nullptr) {
        std::optional<float> value = contextTable->getRawValue(key);
        if(value) {
            return compare(*value);
        }
    }
    return false;
}

bool CriterionAlternate::compare(float value) const {
    return m_invert == (m_options.find(static_cast<int>(value)) == m_options.end());
}

int CriterionAlternate::getPriority() const {
    return 2;
}

}