#pragma once

namespace Contextual {

class Criterion {
public:
    virtual int getPriority() const = 0;
};

}