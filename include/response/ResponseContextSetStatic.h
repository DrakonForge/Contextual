#pragma once

#include "ResponseContext.h"

namespace Contextual {

class ResponseContextSetStatic : public ResponseContext {
public:
    ResponseContextSetStatic(std::string table, std::string key, FactType type, float value);
    void execute(DatabaseQuery& query) override;

private:
    FactType m_type;
    float m_value;
};

}  // namespace Contextual