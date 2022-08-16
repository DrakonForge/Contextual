#include "ResponseContextSetDynamic.h"

namespace Contextual {

ResponseContextSetDynamic::ResponseContextSetDynamic(std::string table, std::string key, std::string otherTable,
                                                     std::string otherKey)
    : ResponseContext(std::move(table), std::move(key)),
      m_otherTable(std::move(otherTable)),
      m_otherKey(std::move(otherKey)) {}

void ResponseContextSetDynamic::execute(DatabaseQuery& query) {
    const std::shared_ptr<ContextTable>& contextTable = query.getContextTable(m_table);
    if (contextTable == nullptr) {
        return;
    }
    const std::shared_ptr<ContextTable>& otherContextTable = query.getContextTable(m_otherTable);
    if (otherContextTable == nullptr) {
        return;
    }
    FactType type = otherContextTable->getType(m_otherKey);
    if (type == FactType::kList) {
        const auto& list = otherContextTable->getList(m_otherKey);
        bool isStringList = otherContextTable->isStringList(m_otherKey);
        if (list == nullptr) {
            auto listCopy = std::make_unique<std::unordered_set<int>>(std::move(*list));
            contextTable->set(m_key, listCopy, isStringList);
        }
    } else {
        std::optional<float> rawValue = otherContextTable->getRawValue(m_otherKey);
        if (rawValue) {
            contextTable->setRawValue(m_key, *rawValue, type);
        }
    }
}

}  // namespace Contextual