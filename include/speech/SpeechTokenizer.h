#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "SymbolToken.h"

namespace Contextual {

enum class SpeechTokenizerReturnCode : uint32_t { kSuccess, kInvalidSyntax, kInvalidFormat, kInvalidSymbol };

struct SpeechTokenizerResult {
    SpeechTokenizerReturnCode code;
    std::string errorMsg;
};

namespace SpeechTokenizer {

SpeechTokenizerResult tokenize(std::vector<std::shared_ptr<SpeechToken>>& tokens, const std::string& text,
                               const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& symbols,
                               const std::unordered_map<std::string, std::shared_ptr<SymbolToken>>& localSymbols,
                               const FunctionTable& functionTable);

}

}  // namespace Contextual