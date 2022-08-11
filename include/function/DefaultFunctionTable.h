#pragma once

#include "FunctionTable.h"

namespace Contextual {

class DefaultFunctionTable : public FunctionTable {
public:
    void initialize() override;
    FunctionVal doCall(const std::string& name, const std::vector<std::shared_ptr<SymbolToken>>& args,
                       DatabaseQuery& query) const override;
};

}  // namespace Contextual