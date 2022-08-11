#include "FunctionTable.h"

#include "TokenBoolean.h"
#include "TokenContext.h"
#include "TokenFloat.h"
#include "TokenFunction.h"
#include "TokenInt.h"
#include "TokenList.h"
#include "TokenString.h"

namespace Contextual {

namespace {

const std::unique_ptr<FunctionSig> g_NOT_FOUND = std::make_unique<FunctionSig>();

}  // namespace

const std::unique_ptr<FunctionSig>& FunctionTable::getSignature(const std::string& name) const {
    auto got = m_functionLookup.find(name);
    if (got == m_functionLookup.end()) {
        return g_NOT_FOUND;
    }
    return got->second;
}

// Performs preliminary typechecking to see if arguments can be converted properly
bool FunctionTable::matches(TokenType targetType, TokenType type) {
    // Function and Context are "any" types
    if (type == TokenType::kFunction || type == TokenType::kContext || targetType == TokenType::kFunction ||
        targetType == TokenType::kContext) {
        return true;
    }

    if (targetType == TokenType::kString) {
        // Anything can be converted into a string except for booleans
        return type != TokenType::kBool;
    }
    if (targetType == TokenType::kInt || targetType == TokenType::kFloat) {
        // Floats and ints are interchangeable
        return type == TokenType::kInt || type == TokenType::kFloat;
    }

    // Default comparison (for lists and booleans)
    return targetType == type;
}

FunctionVal FunctionTable::call(const std::string& name, const std::vector<std::shared_ptr<SymbolToken>>& args,
                                DatabaseQuery& query) const {
    auto got = m_functionLookup.find(name);
    if (got == m_functionLookup.end()) {
        return FunctionVal();
    }

    const std::unique_ptr<FunctionSig>& sig = got->second;

    // Check arguments
    if (sig->argTypes.empty()) {
        // Function should not have any args
        if (!args.empty()) {
            return FunctionVal();
        }
        // Pass
    } else {
        // Check size
        if (args.size() != sig->argTypes.size() || (sig->hasVarArgs && args.size() < sig->argTypes.size())) {
            // Incorrect number of arguments
            return FunctionVal();
        }
        const TokenType lastType = sig->argTypes[sig->argTypes.size() - 1];
        for (int i = 0; i < args.size(); ++i) {
            if (i < sig->argTypes.size()) {
                if (!matches(sig->argTypes[i], args[i]->getType())) {
                    return FunctionVal();
                }
            } else if (!sig->hasVarArgs || !matches(lastType, args[i]->getType())) {
                return FunctionVal();
            }
        }
        // Pass
    }

    // Call function
    FunctionVal result = doCall(name, args, query);

    // Check return type
    if (result.error || !matches(sig->returnType, result.type)) {
        return FunctionVal();
    }
    return result;
}

void FunctionTable::registerFunction(std::string name, TokenType returnType, std::vector<TokenType> argTypes,
                                     bool hasVarArgs) {
    FunctionSig sig = {returnType, std::move(argTypes), hasVarArgs};
    m_functionLookup.emplace(std::move(name), std::make_unique<FunctionSig>(sig));
}

std::optional<std::string> FunctionTable::argToString(const std::shared_ptr<SymbolToken>& token,
                                                      DatabaseQuery& query) const {
    return token->evaluate(query);
}

std::optional<std::unordered_set<int>> FunctionTable::argToList(const std::shared_ptr<SymbolToken>& token,
                                                                DatabaseQuery& query) const {
    if (token->getType() == TokenType::kContext) {
        const auto& contextToken = std::static_pointer_cast<TokenContext>(token);
        const std::shared_ptr<ContextTable>& contextTable = query.getContextTable(contextToken->getTable());
        if (contextTable == nullptr) {
            return std::nullopt;
        }
        const auto& ptr = contextTable->getList(contextToken->getKey());
        if (ptr == nullptr) {
            return std::nullopt;
        }
        return *ptr;
    }
    if (token->getType() == TokenType::kFunction) {
        const auto& functionToken = std::static_pointer_cast<TokenFunction>(token);
        FunctionVal result = call(functionToken->getName(), functionToken->getArgs(), query);
        if (!result.error && result.type == TokenType::kList) {
            return result.listVal;
        }
        return std::nullopt;
    }
    if (token->getType() == TokenType::kList) {
        const auto& listToken = std::static_pointer_cast<TokenList>(token);
        std::unordered_set<int> result;
        const auto& items = listToken->getValue();
        for (const auto& item : items) {
            std::optional<int> num = argToListItem(item, query);
            if (num) {
                result.insert(*num);
            } else {
                return std::nullopt;
            }
        }
        return result;
    }
    return std::nullopt;
}

std::optional<int> FunctionTable::argToListItem(const std::shared_ptr<SymbolToken>& token, DatabaseQuery& query) const {
    if (token->getType() == TokenType::kContext) {
        const auto& contextToken = std::static_pointer_cast<TokenContext>(token);
        const std::shared_ptr<ContextTable>& contextTable = query.getContextTable(contextToken->getTable());
        if (contextTable == nullptr) {
            return std::nullopt;
        }
        FactType type = contextTable->getType(contextToken->getKey());
        if (type == FactType::kNumber || type == FactType::kString) {
            return contextTable->getRawValue(contextToken->getKey());
        }
        return std::nullopt;
    }
    if (token->getType() == TokenType::kFunction) {
        const auto& functionToken = std::static_pointer_cast<TokenFunction>(token);
        FunctionVal result = call(functionToken->getName(), functionToken->getArgs(), query);
        if (!result.error) {
            if (result.type == TokenType::kInt) {
                return result.intVal;
            }
            if (result.type == TokenType::kFloat) {
                return result.floatVal;
            }
            if (result.type == TokenType::kString) {
                return query.getStringTable().cache(result.stringVal);
            }
        }
        return std::nullopt;
    }
    if (token->getType() == TokenType::kInt) {
        const auto& intToken = std::static_pointer_cast<TokenInt>(token);
        return intToken->getValue();
    }
    if (token->getType() == TokenType::kFloat) {
        const auto& floatToken = std::static_pointer_cast<TokenFloat>(token);
        return floatToken->getValue();
    }
    if (token->getType() == TokenType::kString) {
        const auto& strToken = std::static_pointer_cast<TokenString>(token);
        return query.getStringTable().cache(strToken->getValue());
    }
    return std::nullopt;
}

std::optional<int> FunctionTable::argToInt(const std::shared_ptr<SymbolToken>& token, DatabaseQuery& query) const {
    if (token->getType() == TokenType::kContext) {
        const auto& contextToken = std::static_pointer_cast<TokenContext>(token);
        const std::shared_ptr<ContextTable>& contextTable = query.getContextTable(contextToken->getTable());
        if (contextTable == nullptr) {
            return std::nullopt;
        }
        return contextTable->getInt(contextToken->getKey());
    }
    if (token->getType() == TokenType::kFunction) {
        const auto& functionToken = std::static_pointer_cast<TokenFunction>(token);
        FunctionVal result = call(functionToken->getName(), functionToken->getArgs(), query);
        if (!result.error) {
            if (result.type == TokenType::kInt) {
                return result.intVal;
            }
            if (result.type == TokenType::kFloat) {
                return result.floatVal;
            }
        }
        return std::nullopt;
    }
    if (token->getType() == TokenType::kInt) {
        const auto& intToken = std::static_pointer_cast<TokenInt>(token);
        return intToken->getValue();
    }
    if (token->getType() == TokenType::kFloat) {
        const auto& floatToken = std::static_pointer_cast<TokenFloat>(token);
        return floatToken->getValue();
    }
    return std::nullopt;
}

std::optional<float> FunctionTable::argToFloat(const std::shared_ptr<SymbolToken>& token, DatabaseQuery& query) const {
    if (token->getType() == TokenType::kContext) {
        const auto& contextToken = std::static_pointer_cast<TokenContext>(token);
        const std::shared_ptr<ContextTable>& contextTable = query.getContextTable(contextToken->getTable());
        if (contextTable == nullptr) {
            return std::nullopt;
        }
        return contextTable->getFloat(contextToken->getKey());
    }
    if (token->getType() == TokenType::kFunction) {
        const auto& functionToken = std::static_pointer_cast<TokenFunction>(token);
        FunctionVal result = call(functionToken->getName(), functionToken->getArgs(), query);
        if (!result.error) {
            if (result.type == TokenType::kInt) {
                return result.intVal;
            }
            if (result.type == TokenType::kFloat) {
                return result.floatVal;
            }
        }
        return std::nullopt;
    }
    if (token->getType() == TokenType::kInt) {
        const auto& intToken = std::static_pointer_cast<TokenInt>(token);
        return intToken->getValue();
    }
    if (token->getType() == TokenType::kFloat) {
        const auto& floatToken = std::static_pointer_cast<TokenFloat>(token);
        return floatToken->getValue();
    }
    return std::nullopt;
}

std::optional<bool> FunctionTable::argToBool(const std::shared_ptr<SymbolToken>& token, DatabaseQuery& query) const {
    if (token->getType() == TokenType::kContext) {
        const auto& contextToken = std::static_pointer_cast<TokenContext>(token);
        const std::shared_ptr<ContextTable>& contextTable = query.getContextTable(contextToken->getTable());
        if (contextTable == nullptr) {
            return std::nullopt;
        }
        return contextTable->getBool(contextToken->getKey());
    }
    if (token->getType() == TokenType::kFunction) {
        const auto& functionToken = std::static_pointer_cast<TokenFunction>(token);
        FunctionVal result = call(functionToken->getName(), functionToken->getArgs(), query);
        if (!result.error) {
            if (result.type == TokenType::kBool) {
                return result.boolVal;
            }
        }
        return std::nullopt;
    }
    if (token->getType() == TokenType::kBool) {
        const auto& boolToken = std::static_pointer_cast<TokenBoolean>(token);
        return boolToken->getValue();
    }
    return std::nullopt;
}

}  // namespace Contextual