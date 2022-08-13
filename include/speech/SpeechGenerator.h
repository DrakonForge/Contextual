#pragma once

#include <memory>
#include <vector>

#include "DatabaseQuery.h"
#include "SpeechToken.h"
#include "TextToken.h"

namespace Contextual {

enum class SpeechGeneratorReturnCode { kSuccess, kFailure };

namespace SpeechGenerator {

SpeechGeneratorReturnCode generateLine(std::vector<std::shared_ptr<TextToken>>& speechLine, DatabaseQuery& query,
                                   const std::vector<std::shared_ptr<SpeechToken>>& speechTokens);
std::string getRawSpeechLine(const std::vector<std::shared_ptr<TextToken>>& speechLine);
std::string integerToOrdinal(int num);
std::string integerToWord(int num);

}  // namespace SpeechGenerator

}  // namespace Contextual