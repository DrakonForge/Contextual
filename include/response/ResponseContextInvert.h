#pragma once

#include "ResponseContext.h"

namespace Contextual {

class ResponseContextInvert : public ResponseContext {
public:
    ResponseContextInvert(std::string table, std::string key);
    void execute(DatabaseQuery& query) override;
};

}  // namespace Contextual