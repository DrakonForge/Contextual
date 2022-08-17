#include "ResponseSpeech.h"

#include "MathUtils.h"

namespace Contextual {

namespace {
const std::vector<std::shared_ptr<SpeechToken>> g_EMPTY_LINE;
}

ResponseSpeech::ResponseSpeech(std::vector<std::vector<std::shared_ptr<SpeechToken>>> speechLines)
    : m_speechLines(std::move(speechLines)) {}

const std::vector<std::shared_ptr<SpeechToken>>& ResponseSpeech::getRandomLine() const {
    if (m_speechLines.empty()) {
        return g_EMPTY_LINE;
    }

    // Select random speech line
    size_t index = MathUtils::randUInt(0, m_speechLines.size() - 1);
    return m_speechLines[index];
}

ResponseType ResponseSpeech::getType() const {
    return ResponseType::kSpeech;
}

}  // namespace Contextual
