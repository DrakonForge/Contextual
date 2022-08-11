#include "token/TokenFunction.h"

#include <utility>

namespace Contextual {

TokenFunction::TokenFunction(std::string name,
                             std::vector<std::shared_ptr<SymbolToken>> args) : m_name(std::move(name)), m_args(std::move(args)) {}

std::optional<std::string> TokenFunction::evaluate(const DatabaseQuery& query) const {
    // TODO: Evaluate function
    return std::nullopt;
}

std::string TokenFunction::toString() const {
    if(m_args.empty()) {
        return "[Function=" + m_name + "()]";
    } else {
        std::string str = "[Function=" + m_name + "(" + m_args[0]->toString();
        for(int i = 1; i < m_args.size(); ++i) {
            str += ", " + m_args[i]->toString();
        }
        str += ")]";
        return str;
    }

}

TokenType TokenFunction::getType() const {
    return TokenType::kFunction;
}
const std::string& TokenFunction::getName() const {
    return m_name;
}
const std::vector<std::shared_ptr<SymbolToken>>& TokenFunction::getArgs()
    const {
    return m_args;
}

}