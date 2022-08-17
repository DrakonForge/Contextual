#pragma once

#include "ResponseContext.h"

namespace Contextual {

class ResponseContextSetList : public ResponseContext {
public:
    ResponseContextSetList(std::string table, std::string key, std::unordered_set<int> value, bool isStringList);
    void execute(DatabaseQuery& query) override;

private:
    std::unordered_set<int> m_value;
    bool m_isStringList;
};

}  // namespace Contextual