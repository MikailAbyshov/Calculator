#include "ExpressionParser.h"

#include <cctype>
#include <cmath>
#include <stdexcept>

ExpressionParser::ExpressionParser(const PluginManager& plugins)
    : plugins_(plugins) {}

void ExpressionParser::fail(const std::string& msg) const {
    throw std::runtime_error("Parse error at position " +
                             std::to_string(pos_) + ": " + msg);
}

void ExpressionParser::skipSpaces() {
    while (pos_ < src_.size() &&
           std::isspace(static_cast<unsigned char>(src_[pos_]))) {
        ++pos_;
    }
}

bool ExpressionParser::match(char c) {
    skipSpaces();
    if (pos_ < src_.size() && src_[pos_] == c) {
        ++pos_;
        return true;
    }
    return false;
}

double ExpressionParser::evaluate(const std::string& input) {
    src_ = input;
    pos_ = 0;
    double v = parseExpression();
    skipSpaces();
    if (pos_ != src_.size()) {
        fail("unexpected trailing characters");
    }
    return v;
}

double ExpressionParser::parseExpression() {
    double v = parseTerm();
    while (true) {
        skipSpaces();
        if (pos_ >= src_.size()) break;
        char c = src_[pos_];
        if (c == '+') { ++pos_; v += parseTerm(); }
        else if (c == '-') { ++pos_; v -= parseTerm(); }
        else break;
    }
    return v;
}

double ExpressionParser::parseTerm() {
    double v = parseFactor();
    while (true) {
        skipSpaces();
        if (pos_ >= src_.size()) break;
        char c = src_[pos_];
        if (c == '*') {
            ++pos_;
            v *= parseFactor();
        } else if (c == '/') {
            ++pos_;
            double rhs = parseFactor();
            if (rhs == 0.0) fail("division by zero");
            v /= rhs;
        } else break;
    }
    return v;
}

double ExpressionParser::parseFactor() {
    double base = parseUnary();
    skipSpaces();
    if (pos_ < src_.size() && src_[pos_] == '^') {
        ++pos_;
        double exp = parseFactor();
        return std::pow(base, exp);
    }
    return base;
}

double ExpressionParser::parseUnary() {
    skipSpaces();
    if (pos_ < src_.size()) {
        if (src_[pos_] == '+') { ++pos_; return parseUnary(); }
        if (src_[pos_] == '-') { ++pos_; return -parseUnary(); }
    }
    return parsePrimary();
}

double ExpressionParser::parsePrimary() {
    skipSpaces();
    if (pos_ >= src_.size()) fail("unexpected end of input");

    char c = src_[pos_];

    if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
        size_t start = pos_;
        bool seenDot = false;
        while (pos_ < src_.size()) {
            char d = src_[pos_];
            if (std::isdigit(static_cast<unsigned char>(d))) { ++pos_; }
            else if (d == '.' && !seenDot) { seenDot = true; ++pos_; }
            else break;
        }
        try {
            return std::stod(src_.substr(start, pos_ - start));
        } catch (const std::exception&) {
            pos_ = start;
            fail("invalid number");
        }
    }

    if (c == '(') {
        ++pos_;
        double v = parseExpression();
        if (!match(')')) fail("expected ')'");
        return v;
    }

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
        std::string name = parseIdentifier();
        if (!match('(')) fail("expected '(' after function name '" + name + "'");
        double arg = parseExpression();
        if (!match(')')) fail("expected ')' after argument of '" + name + "'");
        if (!plugins_.has(name)) {
            fail("unknown function '" + name + "'");
        }
        try {
            return plugins_.call(name, arg);
        } catch (const std::exception& e) {
            throw std::runtime_error("Function '" + name + "' failed: " + e.what());
        }
    }

    fail(std::string("unexpected character '") + c + "'");
}

std::string ExpressionParser::parseIdentifier() {
    size_t start = pos_;
    while (pos_ < src_.size() &&
           (std::isalnum(static_cast<unsigned char>(src_[pos_])) ||
            src_[pos_] == '_')) {
        ++pos_;
    }
    return src_.substr(start, pos_ - start);
}