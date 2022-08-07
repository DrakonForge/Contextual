#pragma once

#include <string>
#include <vector>

#include "Response.h"

namespace Contextual {

class ResponseSimple : public Response {
public:
    explicit ResponseSimple(std::vector<std::string> options);
    [[nodiscard]] const std::string& getRandomOption() const;
    [[nodiscard]] const std::vector<std::string>& getOptions() const;
private:
    const std::vector<std::string> m_options;

};

}