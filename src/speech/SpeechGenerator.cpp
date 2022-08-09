#include "SpeechGenerator.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "SymbolToken.h"

namespace Contextual::SpeechGenerator {

SpeechGeneratorResult generateLine(std::vector<std::shared_ptr<TextToken>>& speechLine, const DatabaseQuery& query, const std::vector<std::shared_ptr<SpeechToken>>& speechTokens) {
    bool hasText = false;
    for(const auto& token : speechTokens) {
        if(token->isSymbolToken()) {
            hasText = true;
            const auto& symbolToken = std::static_pointer_cast<SymbolToken>(token);
            std::optional<std::string> nextTokenStr = symbolToken->evaluate(query);
            if(nextTokenStr) {
                speechLine.push_back(std::make_shared<TextLiteral>(std::move(*nextTokenStr)));
            } else {
                return SpeechGeneratorResult::kFailure;
            }
        } else {
            const auto& textToken = std::static_pointer_cast<TextToken>(token);
            speechLine.push_back(textToken);
        }
    }
    // Must have text to print properly
    if(!hasText) {
        return SpeechGeneratorResult::kFailure;
    }
    return SpeechGeneratorResult::kSuccess;
}

std::string getRawSpeechLine(const std::vector<std::shared_ptr<TextToken>>& speechLine) {
    std::string result;
    for(const auto& textToken : speechLine) {
        if(textToken->isLiteral) {
            const auto& textLiteral = std::static_pointer_cast<TextLiteral>(textToken);
            result += textLiteral->value;
        }
    }
    return result;
}

}