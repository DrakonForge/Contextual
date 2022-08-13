#pragma once

#include <string>

#include "SpeechToken.h"

namespace Contextual {

struct TextToken : public SpeechToken {
    virtual bool isLiteral() { return false; };
protected:
    TextToken() = default;
};

// Explicit constructors so that they can be used when making shared pointers

struct TextLiteral : TextToken {
    std::string value;
    explicit TextLiteral(std::string value) : value(std::move(value)) {}
    bool isLiteral() override { return true; }
    [[nodiscard]] std::string toString() const override {
        return "\"" + value + "\"";
    }
};

struct TextFormat : TextToken {
    std::string attribute;
    explicit TextFormat(std::string attribute) : attribute(std::move(attribute)) {}
    [[nodiscard]] std::string toString() const override {
        return "{" + attribute + "}";
    }
};

struct TextFormatBool : TextFormat {
    bool value = false;
    explicit TextFormatBool(std::string attribute, bool value) : TextFormat(std::move(attribute)), value(value) {}
    [[nodiscard]] std::string toString() const override {
        if (value) {
            return "{" + attribute + "=true}";
        }
        return "{" + attribute + "=false}";
    }
};

struct TextFormatFloat : TextFormat {
    float value = -1.0f;
    explicit TextFormatFloat(std::string attribute, float value) : TextFormat(std::move(attribute)), value(value) {}
    [[nodiscard]] std::string toString() const override {
        return "{" + attribute + "=" + std::to_string(value) + "}";
    }
};

struct TextFormatInt : TextFormat {
    int value = -1;
    explicit TextFormatInt(std::string attribute, int value) : TextFormat(std::move(attribute)), value(value) {}
    [[nodiscard]] std::string toString() const override {
        return "{" + attribute + "=" + std::to_string(value) + "}";
    }
};

struct TextFormatString : TextFormat {
    std::string value;
    explicit TextFormatString(std::string attribute, std::string value)
        : TextFormat(std::move(attribute)), value(std::move(value)) {}
    [[nodiscard]] std::string toString() const override {
        return "{" + attribute + "=\"" + value + "\"}";
    }
};

}  // namespace Contextual