#pragma once
#include <memory>
#include <vector>

#include "DatabaseQuery.h"
#include "Response.h"
#include "SpeechToken.h"

namespace Contextual {

class ResponseEvent : public Response {
public:
    ResponseEvent(std::string name, std::vector<std::string> args);
    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] const std::vector<std::string>& getArgs() const;
    [[nodiscard]] ResponseType getType() const override;

private:
    std::string m_name;
    std::vector<std::string> m_args;
};

}  // namespace Contextual