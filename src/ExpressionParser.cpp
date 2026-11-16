#include "ExpressionParser.h"

#include <cctype>
#include <cmath>
#include <stdexcept>
#include <string>

ExpressionParser::ExpressionParser(const PluginManager &plugins)
    : plugins_(plugins) {}

void ExpressionParser::fail(const std::string &message) const {
    std::string text = "Parse error at position ";
    text += std::to_string(position_);
    text += ": ";
    text += message;
    throw std::runtime_error(text);
}

void ExpressionParser::skipSpaces() {
    while (position_ < source_.size() &&
           std::isspace(static_cast<unsigned char>(source_[position_])) != 0) {
        ++position_;
    }
}

bool ExpressionParser::match(char symbol) {
    skipSpaces();
    if (position_ < source_.size() && source_[position_] == symbol) {
        ++position_;
        return true;
    }
    return false;
}

double ExpressionParser::evaluate(const std::string &input) {
    source_ = input;
    position_ = 0;
    double value = parseExpression();
    skipSpaces();
    if (position_ != source_.size()) {
        fail("unexpected trailing characters");
    }
    return value;
}

double ExpressionParser::parseExpression() {
    double value = parseTerm();
    while (true) {
        skipSpaces();
        if (position_ >= source_.size()) {
            break;
        }
        char symbol = source_[position_];
        if (symbol == '+') {
            ++position_;
            value += parseTerm();
        } else if (symbol == '-') {
            ++position_;
            value -= parseTerm();
        } else {
            break;
        }
    }
    return value;
}

double ExpressionParser::parseTerm() {
    double value = parseFactor();
    while (true) {
        skipSpaces();
        if (position_ >= source_.size()) {
            break;
        }
        char symbol = source_[position_];
        if (symbol == '*') {
            ++position_;
            value *= parseFactor();
        } else if (symbol == '/') {
            ++position_;
            double right = parseFactor();
            if (right == 0.0) {
                fail("division by zero");
            }
            value /= right;
        } else {
            break;
        }
    }
    return value;
}

double ExpressionParser::parseFactor() {
    double base = parseUnary();
    skipSpaces();
    if (position_ < source_.size() && source_[position_] == '^') {
        ++position_;
        double exponent = parseFactor();
        return std::pow(base, exponent);
    }
    return base;
}

double ExpressionParser::parseUnary() {
    skipSpaces();
    if (position_ < source_.size()) {
        if (source_[position_] == '+') {
            ++position_;
            return parseUnary();
        }
        if (source_[position_] == '-') {
            ++position_;
            return -parseUnary();
        }
    }
    return parsePrimary();
}

double ExpressionParser::parsePrimary() {
    skipSpaces();
    if (position_ >= source_.size()) {
        fail("unexpected end of input");
    }

    char symbol = source_[position_];

    if (std::isdigit(static_cast<unsigned char>(symbol)) != 0 ||
        symbol == '.') {
        return parseNumber();
    }

    if (symbol == '(') {
        return parseParenthesized();
    }

    if (std::isalpha(static_cast<unsigned char>(symbol)) != 0 ||
        symbol == '_') {
        return parseFunctionCall();
    }

    std::string text = "unexpected character '";
    text += symbol;
    text += "'";
    fail(text);
}

double ExpressionParser::parseNumber() {
    std::size_t start = position_;
    bool seenDot = false;
    while (position_ < source_.size()) {
        char digit = source_[position_];
        if (std::isdigit(static_cast<unsigned char>(digit)) != 0) {
            ++position_;
        } else if (digit == '.' && !seenDot) {
            seenDot = true;
            ++position_;
        } else {
            break;
        }
    }
    try {
        return std::stod(source_.substr(start, position_ - start));
    } catch (const std::exception &) {
        position_ = start;
        fail("invalid number");
    }
}

double ExpressionParser::parseParenthesized() {
    ++position_;  // '('
    double value = parseExpression();
    if (!match(')')) {
        fail("expected ')'");
    }
    return value;
}

double ExpressionParser::parseFunctionCall() {
    std::string name = parseIdentifier();
    if (!match('(')) {
        fail("expected '(' after function name '" + name + "'");
    }
    double argument = parseExpression();
    if (!match(')')) {
        fail("expected ')' after argument of '" + name + "'");
    }
    if (!plugins_.has(name)) {
        fail("unknown function '" + name + "'");
    }
    try {
        return plugins_.call(name, argument);
    } catch (const std::exception &exception) {
        std::string text = "Function '";
        text += name;
        text += "' failed: ";
        text += exception.what();
        throw std::runtime_error(text);
    }
}

std::string ExpressionParser::parseIdentifier() {
    std::size_t start = position_;
    while (position_ < source_.size() &&
           (std::isalnum(static_cast<unsigned char>(source_[position_])) != 0 ||
            source_[position_] == '_')) {
        ++position_;
    }
    return source_.substr(start, position_ - start);
}