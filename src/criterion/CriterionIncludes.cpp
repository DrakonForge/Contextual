#include "CriterionIncludes.h"

#include <utility>

namespace Contextual {

CriterionIncludes::CriterionIncludes(std::vector<int> options, bool invert)
    : CriterionListComparable(invert), m_options(std::move(options)) {}

bool CriterionIncludes::evaluate(const std::string& table, const std::string& key, const DatabaseQuery& query) const {
    std::shared_ptr<ContextTable> contextTable = query.getContextTable(table);
    if (contextTable != nullptr) {
        const std::unique_ptr<std::unordered_set<int>>& value = contextTable->getList(key);
        if (value != nullptr) {
            return compare(*value);
        }
    }
    return false;
}

bool CriterionIncludes::compare(const std::unordered_set<int>& value) const {
    bool included = false;
    for (const int option : m_options) {
        if (value.find(option) != value.end()) {
            included = true;
            break;
        }
    }
    return m_invert != included;
}

int CriterionIncludes::getPriority() const {
    return 1;
}

}  // namespace Contextual