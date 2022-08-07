#pragma once

namespace Contextual {

class SpeechToken {
public:
    [[nodiscard]] virtual std::string toString() const = 0;
protected:
    SpeechToken() = default;
};

}