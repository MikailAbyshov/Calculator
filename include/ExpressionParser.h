#pragma once

#include "PluginManager.h"
#include <string>

class ExpressionParser {
public:
    explicit ExpressionParser(const PluginManager &plugins);

    double evaluate(const std::string &input);

private:
    void skipSpaces();

    bool match(char c);

    double parseExpression();

    double parseTerm();

    double parseFactor();

    double parseUnary();

    double parsePrimary();

    double parseFunctionCall(const std::string &name);

    std::string parseIdentifier();

    [[noreturn]] void fail(const std::string &msg) const;

    const PluginManager &plugins_;
    std::string src_;
    size_t pos_ = 0;
};
