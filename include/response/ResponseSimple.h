#pragma once

#include <string>
#include <vector>

#include "Response.h"

namespace Contextual {

class ResponseSimple : public Response {
public:
    ResponseSimple(std::vector<std::string> options);
    const std::string& getRandomOption() const;
    const std::vector<std::string>& getOptions() const;
private:
    const std::vector<std::string> m_options;

};

}