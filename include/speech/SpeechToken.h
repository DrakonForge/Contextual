#pragma once

namespace Contextual {

class SpeechToken {
public:
    [[nodiscard]] virtual std::string toString() const = 0;
    [[nodiscard]] virtual bool isSymbolToken() const {
        return false;
    }

protected:
    SpeechToken() = default;
};

}  // namespace Contextual