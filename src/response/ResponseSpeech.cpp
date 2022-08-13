#include "ResponseSpeech.h"

#include <random>

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
    static std::default_random_engine e;
    std::uniform_int_distribution<size_t> dis(0, m_speechLines.size() - 1);
    size_t index = dis(e);
    return m_speechLines[index];
}

ResponseType ResponseSpeech::getType() const {
    return ResponseType::kSpeech;
}

}  // namespace Contextual
