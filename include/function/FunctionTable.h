#pragma once

#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

#include "DatabaseQuery.h"
#include "SymbolToken.h"
#include "TokenType.h"

namespace Contextual {

class DatabaseQuery;
class SymbolToken;

struct FunctionVal {
    bool error;
    TokenType type;
    bool boolVal;
    int intVal;
    float floatVal;
    bool isStringList;
    std::string stringVal;
    std::vector<int> listVal;

    explicit FunctionVal()
        : error(true), type(TokenType::kContext), boolVal(false), intVal(0), floatVal(0), isStringList(false) {}
    explicit FunctionVal(bool boolVal)
        : error(false), type(TokenType::kBool), boolVal(boolVal), intVal(0), floatVal(0), isStringList(false) {}
    explicit FunctionVal(int intVal)
        : error(false), type(TokenType::kBool), boolVal(false), intVal(intVal), floatVal(0), isStringList(false) {}
    explicit FunctionVal(float floatVal)
        : error(false), type(TokenType::kBool), boolVal(false), intVal(0), floatVal(floatVal), isStringList(false) {}
    explicit FunctionVal(std::string stringVal)
        : error(false),
          type(TokenType::kBool),
          boolVal(false),
          intVal(0),
          floatVal(0),
          stringVal(std::move(stringVal)),
          isStringList(false) {}
    explicit FunctionVal(std::vector<int> listVal, bool isStringList)
        : error(false),
          type(TokenType::kBool),
          boolVal(false),
          intVal(0),
          floatVal(0),
          listVal(std::move(listVal)),
          isStringList(isStringList) {}
};

struct FunctionSig {
    TokenType returnType;
    std::vector<TokenType> argTypes;
    bool hasVarArgs;
};

class FunctionTable {
public:
    virtual ~FunctionTable() = default;
    static bool validateArgs(const std::unique_ptr<FunctionSig>& sig,
                             const std::vector<std::shared_ptr<SymbolToken>>& args);
    static bool matches(TokenType targetType, TokenType type);
    virtual void initialize() = 0;
    const std::unique_ptr<FunctionSig>& getSignature(const std::string& name) const;
    FunctionVal call(const std::string& name, const std::vector<std::shared_ptr<SymbolToken>>& args,
                     DatabaseQuery& query) const;

protected:
    void registerFunction(std::string, TokenType returnType, std::vector<TokenType> argTypes, bool hasVarArgs);
    virtual FunctionVal doCall(const std::string& name, const std::vector<std::shared_ptr<SymbolToken>>& args,
                               DatabaseQuery& query) const = 0;
    std::optional<std::string> argToString(const std::shared_ptr<SymbolToken>& token, DatabaseQuery& query) const;
    std::optional<std::pair<std::vector<int>, bool>> argToList(const std::shared_ptr<SymbolToken>& token, DatabaseQuery& query) const;
    std::optional<int> argToListItem(bool& isString, const std::shared_ptr<SymbolToken>& token, DatabaseQuery& query) const;
    std::optional<int> argToInt(const std::shared_ptr<SymbolToken>& token, DatabaseQuery& query) const;
    std::optional<float> argToFloat(const std::shared_ptr<SymbolToken>& token, DatabaseQuery& query) const;
    std::optional<bool> argToBool(const std::shared_ptr<SymbolToken>& token, DatabaseQuery& query) const;

private:
    std::unordered_map<std::string, std::unique_ptr<FunctionSig>> m_functionLookup;
};

}  // namespace Contextual