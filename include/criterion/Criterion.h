#pragma once

namespace Contextual {

class Criterion {
public:
    static int getCount();
    virtual int getPriority() const = 0;
protected:
    Criterion();
    ~Criterion();
private:
    static int count;
};

}