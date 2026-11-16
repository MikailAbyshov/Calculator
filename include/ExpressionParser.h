#pragma once

#include "PluginManager.h"

#include <string>

class ExpressionParser {
public:
    explicit ExpressionParser(const PluginManager &plugins);

    double evaluate(const std::string &input);

private:
    void skipSpaces();

    bool match(char symbol);

    double parseExpression();

    double parseTerm();

    double parseFactor();

    double parseUnary();

    double parsePrimary();

    double parseNumber();

    double parseParenthesized();

    double parseFunctionCall();

    std::string parseIdentifier();

    [[noreturn]] void fail(const std::string &message) const;

    const PluginManager &plugins_;
    std::string source_;
    std::size_t position_ = 0;
};