#pragma once

#include <memory>
#include <vector>

#include "DatabaseQuery.h"
#include "ResponseSpeech.h"
#include "SpeechToken.h"
#include "TextToken.h"

namespace Contextual::SpeechGenerator {

enum class SpeechGeneratorReturnCode {
    kSuccess,
    kGenerationError,
    kSelectionError
};
SpeechGeneratorReturnCode performSpeechResponse(std::vector<std::shared_ptr<TextToken>>& speechLine,
                           std::shared_ptr<ResponseSpeech>& speechResponse, DatabaseQuery& query,
                           const std::shared_ptr<Response>& response);
bool generateLineFromResponse(std::vector<std::shared_ptr<TextToken>>& speechLine, DatabaseQuery& query,
                              const std::shared_ptr<ResponseSpeech>& speechResponse);
bool generateLineFromTokens(std::vector<std::shared_ptr<TextToken>>& speechLine, DatabaseQuery& query,
                            const std::vector<std::shared_ptr<SpeechToken>>& speechTokens);
std::string getRawSpeechLine(const std::vector<std::shared_ptr<TextToken>>& speechLine);
std::string integerToOrdinal(int num);
std::string integerToWord(int num);

}  // namespace Contextual::SpeechGenerator