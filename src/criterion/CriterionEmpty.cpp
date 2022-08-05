#include "CriterionEmpty.h"

namespace Contextual {

CriterionEmpty::CriterionEmpty(bool invert) : CriterionListComparable(invert) {}

bool CriterionEmpty::evaluate(const std::string& table,
                                 const std::string& key,
                                 const DatabaseQuery& query) const {
    std::shared_ptr<ContextTable> contextTable = query.getContextTable(table);
    if(contextTable != nullptr) {
        const std::unique_ptr<std::unordered_set<int>>& value = contextTable->getList(key);
        if(value != nullptr) {
            return compare(*value);
        }
    }
    return false;
}

bool CriterionEmpty::compare(const std::unordered_set<int>& value) const {
    return m_invert != value.empty();
}

int CriterionEmpty::getPriority() const {
    return 4;
}

}