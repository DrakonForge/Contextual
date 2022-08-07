#pragma once

#include <memory>
#include <vector>

#include "TextToken.h"

// TODO: Might not actually be used
namespace Contextual {

struct SpeechResult {
    std::vector<std::shared_ptr<TextToken>> textTokens;
    int priority;
};

}