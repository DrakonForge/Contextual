#pragma once

namespace Contextual {

enum class ResponseType {
    kMultiple,
    kSimple,
    kSpeech,
    kEvent
};
class Response {
public:
    [[nodiscard]] virtual ResponseType getType() const = 0;
protected:
    Response() = default;
};

}  // namespace Contextual