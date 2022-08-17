#pragma once

#include "ResponseContext.h"

namespace Contextual {

class ResponseContextAdd : public ResponseContext {
public:
    ResponseContextAdd(std::string table, std::string key, float value);
    void execute(DatabaseQuery& query) override;

private:
    float m_value;
};

}  // namespace Contextual