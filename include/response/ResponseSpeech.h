#pragma once
#include <memory>
#include <vector>

#include "DatabaseQuery.h"
#include "Response.h"
#include "SpeechToken.h"

namespace Contextual {

class ResponseSpeech : public Response {
public:
    explicit ResponseSpeech(std::vector<std::vector<std::shared_ptr<SpeechToken>>> speechLines);
    [[nodiscard]] const std::vector<std::shared_ptr<SpeechToken>>& getRandomLine() const;
private:
    std::vector<std::vector<std::shared_ptr<SpeechToken>>> m_speechLines;
};

}