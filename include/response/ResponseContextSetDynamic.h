#pragma once

#include "ResponseContext.h"

namespace Contextual {

class ResponseContextSetDynamic : public ResponseContext {
public:
    ResponseContextSetDynamic(std::string table, std::string key, std::string otherTable, std::string otherKey);
    void execute(DatabaseQuery& query) override;

private:
    std::string m_otherTable;
    std::string m_otherKey;
};

}  // namespace Contextual