#include "cosl.hpp"

// Program --> Lexer --> Parser --> AST --> Constant Expression Optimization
// --> ABT --> Function Optimization ??--> Executable

ast::ASTNode Parser::parseAtomicNumber(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {
    if(begin != end) {
        if((*begin)->type() == ast::NUM_TOKEN) {
            auto numUtil = utils::stringToNumberUtil((*begin++)->getVal());
            switch (numUtil.first) {
                case types::B8:
                    return ast::NumericalValue<types::Integer<types::B8>>(types::Integer<types::B8>(numUtil.second));
                case types::B16:
                    return ast::NumericalValue<types::Integer<types::B16>>(types::Integer<types::B16>(numUtil.second));
                case types::B32:
                    return ast::NumericalValue<types::Integer<types::B32>>(types::Integer<types::B32>(numUtil.second));
                case types::B64:
                    return ast::NumericalValue<types::Integer<types::B64>>(types::Integer<types::B64>(numUtil.second));
                case types::B128:
                    return ast::NumericalValue<types::Integer<types::B128>>(types::Integer<types::B128>(numUtil.second));
            }
        }
        else
            throw EXPECTED_NUM_ERR;
    }
    else
        throw IMMATURE_EOF;
}

ast::ASTNode Parser::parseUnAmb(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {
    if(begin != end) {
        if((*begin)->type() == ast::SEP_TOKEN and (*begin)->getVal() == "(") {
            auto child = parseArithmeticExpression(++begin, end);
            if(begin != end and (*begin)->type() == ast::SEP_TOKEN and (*begin++)->getVal() == ")") {
                return ast::UnaryOperator(ast::BKT_PREC, child);
            }
            else {
                throw UNMATCHED_BKT_ERR;
            }
        }
        else {
            return parseAtomicNumber(++begin, end);
        }
    }
    else {
        throw IMMATURE_EOF;
    }
}

ast::ASTNode Parser::parseUnNeg(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {
    bool neg = false;
    while(begin != end and (*begin)->type() == ast::OP_TOKEN) {
        if((*begin)->getVal() == "+") {
            begin++;
        }
        else if((*begin)->getVal() == "-") {
            neg = !neg;
            begin++;
        }
        else {
            throw UNEXPECTED_OP;
        }
    }
    if(begin != end) {
        auto child = parseUnAmb(begin, end);
        if(neg) {
            return ast::UnaryOperator(ast::ATH_USUB, child);
        }
        else {
            return child;
        }
    }
    throw IMMATURE_EOF;
}

ast::ASTNode Parser::parseUnNot(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {
    bool knot = false;
    while(begin != end and (*begin)->type() == ast::OP_TOKEN) {
        if((*begin)->getVal() == "~") {
            knot = !knot;
            begin++;
        }
        else {
            throw UNEXPECTED_OP;
        }
    }
    if(begin != end) {
        auto child = parseUnAmb(begin, end);
        if(knot) {
            return ast::UnaryOperator(ast::ATH_NOT, child);
        }
        else {
            return child;
        }
    }
    throw IMMATURE_EOF;
}

ast::ASTNode Parser::parseAdd(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {
    if(begin != end) {
        auto lhs = parseUnAmb(begin, end);
        if (begin != end and (*begin)->type() == ast::OP_TOKEN and (*begin)->getVal() == "+") {
            auto rhs = parseAdd(++begin, end);
            lhs = ast::BinaryOperator(ast::ATH_ADD, lhs, rhs);
        }
        return lhs;
    }
    else {
        throw IMMATURE_EOF;
    }
}

ast::ASTNode Parser::parseMul(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {
    if(begin != end) {
        auto lhs = parseUnAmb(begin, end);
        if (begin != end and (*begin)->type() == ast::OP_TOKEN and (*begin)->getVal() == "*") {
            auto rhs = parseMul(++begin, end);
            lhs = ast::BinaryOperator(ast::ATH_MUL, lhs, rhs);
        }
        return lhs;
    }
    else {
        throw IMMATURE_EOF;
    }
}

ast::ASTNode Parser::parseAnd(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {
    if(begin != end) {
        auto lhs = parseUnAmb(begin, end);
        if (begin != end and (*begin)->type() == ast::OP_TOKEN and (*begin)->getVal() == "&") {
            auto rhs = parseAnd(++begin, end);
            lhs = ast::BinaryOperator(ast::ATH_AND, lhs, rhs);
        }
        return lhs;
    }
    else {
        throw IMMATURE_EOF;
    }
}

ast::ASTNode Parser::parseOr(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {
    if(begin != end) {
        auto lhs = parseUnAmb(begin, end);
        if (begin != end and (*begin)->type() == ast::OP_TOKEN and (*begin)->getVal() == "|") {
            auto rhs = parseOr(++begin, end);
            lhs = ast::BinaryOperator(ast::ATH_OR, lhs, rhs);
        }
        return lhs;
    }
    else {
        throw IMMATURE_EOF;
    }
}

ast::ASTNode Parser::parseXor(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {
    if(begin != end) {
        auto lhs = parseUnAmb(begin, end);
        if (begin != end and (*begin)->type() == ast::OP_TOKEN and (*begin)->getVal() == "!|") {
            auto rhs = parseXor(++begin, end);
            lhs = ast::BinaryOperator(ast::ATH_XOR, lhs, rhs);
        }
        return lhs;
    }
    else {
        throw IMMATURE_EOF;
    }
}
 
ast::ASTNode Parser::parseXnor(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {
    if(begin != end) {
        auto lhs = parseUnAmb(begin, end);
        if (begin != end and (*begin)->type() == ast::OP_TOKEN and (*begin)->getVal() == "!&") {
            auto rhs = parseXnor(++begin, end);
            lhs = ast::BinaryOperator(ast::ATH_XNOR, lhs, rhs);
        }
        return lhs;
    }
    else {
        throw IMMATURE_EOF;
    }
}

ast::ASTNode Parser::parseArithmeticExpression(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {
    if(begin != end) {
        if((*begin)->type() == ast::OP_TOKEN) {
            // prefix operation
            if ((*begin)->getVal() == "+" or (*begin)->getVal() == "-")
                return parseUnNeg(begin, end);
            else if ((*begin)->getVal() == "~")
                return parseUnNot(begin, end);
            // check for the operation (~, -, or +), and go to necessary parse
        }
        else {
            // all other cases first expect an UnAmb
            ast::ASTNode lhs = parseUnAmb(begin, end);
            if(begin != end) {    
                if ((*begin)->getVal() == "+") {
                    ast::ASTNode rhs = parseAdd(++begin, end);
                    lhs = ast::BinaryOperator(ast::ATH_ADD, lhs, rhs);
                    if (begin != end and (*begin)->getVal() == "-") {
                        rhs = parseUnAmb(++begin, end);
                        return ast::BinaryOperator(ast::ATH_SUB, lhs, rhs);
                    } else {
                        return lhs;
                    }
                } else if ((*begin)->getVal() == "-") {
                    ast::ASTNode rhs = parseUnAmb(++begin, end);
                    return ast::BinaryOperator(ast::ATH_SUB, lhs, rhs);
                } else if ((*begin)->getVal() == "*") {
                    ast::ASTNode rhs = parseMul(++begin, end);
                    lhs = ast::BinaryOperator(ast::ATH_MUL, lhs, rhs);
                    if (begin != end and (*begin)->getVal() == "/") {
                        rhs = parseUnAmb(++begin, end);
                        return ast::BinaryOperator(ast::ATH_DIV, lhs, rhs);
                    } else {
                        return lhs;
                    }
                } else if ((*begin)->getVal() == "/") {
                    ast::ASTNode rhs = parseUnAmb(++begin, end);
                    return ast::BinaryOperator(ast::ATH_DIV, lhs, rhs);
                } else if ((*begin)->getVal() == "^") {
                    ast::ASTNode rhs = parseUnAmb(++begin, end);
                    return ast::BinaryOperator(ast::ATH_POW, lhs, rhs);
                } else if ((*begin)->getVal() == "%") {
                    ast::ASTNode rhs = parseUnAmb(++begin, end);
                    return ast::BinaryOperator(ast::ATH_MOD, lhs, rhs);
                } else if ((*begin)->getVal() == ">>") {
                    ast::ASTNode rhs = parseUnAmb(++begin, end);
                    return ast::BinaryOperator(ast::ATH_RS, lhs, rhs);
                } else if ((*begin)->getVal() == "<<") {
                    ast::ASTNode rhs = parseUnAmb(++begin, end);
                    return ast::BinaryOperator(ast::ATH_LS, lhs, rhs);
                } else if ((*begin)->getVal() == "|") {
                    ast::ASTNode rhs = parseOr(++begin, end);
                    return ast::BinaryOperator(ast::ATH_OR, lhs, rhs);
                } else if ((*begin)->getVal() == "&") {
                    ast::ASTNode rhs = parseAnd(++begin, end);
                    return ast::BinaryOperator(ast::ATH_AND, lhs, rhs);
                } else if ((*begin)->getVal() == "!|") {
                    ast::ASTNode rhs = parseXor(++begin, end);
                    return ast::BinaryOperator(ast::ATH_XOR, lhs, rhs);
                } else if ((*begin)->getVal() == "!&") {
                    ast::ASTNode rhs = parseXnor(++begin, end);
                    return ast::BinaryOperator(ast::ATH_XNOR, lhs, rhs);
                }
            }
        }
    }
    else {
        throw IMMATURE_EOF;
    }
}

void lexer(const std::string& source, std::vector<std::unique_ptr<ast::Token>>& tokenVector) {
    size_t size = source.length(), curPos = 0, tokenPos = 0, base = 0;
    char tokenString[150];
    while(curPos < size) {
        while (curPos < size and isspace(source[curPos]))
            curPos++;

        if(curPos >= size)
            continue;

        // a digit indicates that a number is encountered.
        if(isdigit(source[curPos])) {
            tokenPos = 0;
            
            // by default, a decimal number is expected; we check the base next
            base = 10;
            if(source[curPos] == '0') {
                tokenString[tokenPos++] = source[curPos++];
                if(curPos<size and (source[curPos] == 'b' or source[curPos] == 'B')) {
                    tokenString[tokenPos++] = source[curPos++];
                    base = 2;
                }
                else if(curPos<size and (source[curPos] == 'o' or source[curPos] == 'O')) {
                    tokenString[tokenPos++] = source[curPos++];
                    base = 8;
                }
                else if(curPos<size and (source[curPos] == 'x' or source[curPos] == 'X')) {
                    tokenString[tokenPos++] = source[curPos++];
                    base = 16;
                }
            }
            
            // the integral part is lexed next
            switch (base) {
            case 2:
                while(curPos < size and (source[curPos] == '0' or source[curPos] == '1'))
                    tokenString[tokenPos++] = source[curPos++];
                break;
            case 8:
                while(curPos < size and source[curPos] >= '0' and source[curPos] <= '7')
                    tokenString[tokenPos++] = source[curPos++];
                break;
            case 10:
                while(curPos < size and isdigit(source[curPos]))
                    tokenString[tokenPos++] = source[curPos++];
                break;
            case 16:
                while (curPos < size and (isdigit(source[curPos]) or (source[curPos] >= 'a' and source[curPos] <= 'f') or (source[curPos] >= 'A' and source[curPos] <= 'F')))
                    tokenString[tokenPos++] = source[curPos++];
                break;
            }

            // check for fractional part
            if(curPos < size and source[curPos] == '.') {
                tokenString[tokenPos++] = source[curPos++];
                // the fractional part is lexed next
                switch (base) {
                case 2:
                    while(curPos < size and (source[curPos] == '0' or source[curPos] == '1'))
                        tokenString[tokenPos++] = source[curPos++];
                    break;
                case 8:
                    while(curPos < size and source[curPos] >= '0' and source[curPos] <= '7')
                        tokenString[tokenPos++] = source[curPos++];
                    break;
                case 10:
                    while(curPos < size and isdigit(source[curPos]))
                        tokenString[tokenPos++] = source[curPos++];
                    break;
                case 16:
                    while (curPos < size and (isdigit(source[curPos]) or (source[curPos] >= 'a' and source[curPos] <= 'f') or (source[curPos] >= 'A' and source[curPos] <= 'F')))
                        tokenString[tokenPos++] = source[curPos++];
                    break;
                }
            }

            // check for exponent part
            if(curPos < size and ((base != 16 and (source[curPos] == 'e' or source[curPos] == 'E')) or (base == 16 and (source[curPos] == 'p' or source[curPos] == 'P')))) {
                tokenString[tokenPos++] = source[curPos++];
                // the fractional part is lexed next
                switch (base) {
                case 2:
                    while(curPos < size and (source[curPos] == '0' or source[curPos] == '1'))
                        tokenString[tokenPos++] = source[curPos++];
                    break;
                case 8:
                    while(curPos < size and source[curPos] >= '0' and source[curPos] <= '7')
                        tokenString[tokenPos++] = source[curPos++];
                    break;
                case 10:
                    while(curPos < size and isdigit(source[curPos]))
                        tokenString[tokenPos++] = source[curPos++];
                    break;
                case 16:
                    while (curPos < size and (isdigit(source[curPos]) or (source[curPos] >= 'a' and source[curPos] <= 'f') or (source[curPos] >= 'A' and source[curPos] <= 'F')))
                        tokenString[tokenPos++] = source[curPos++];
                    break;
                }
            }
            
            // end the string
            tokenString[tokenPos] = '\0';
            tokenVector.emplace_back(std::make_unique<ast::NumericalToken>(ast::NumericalToken(std::move(std::string(tokenString)))));
        }

        else if(isalpha(source[curPos])) {
            // we have encountered either a keyword or an identifier
            tokenPos = 0;
            while (curPos < size and !isspace(source[curPos])) {
                tokenString[tokenPos++] = source[curPos++];
            }
            tokenString[tokenPos] = '\0';
            std::string tempTokenString = std::string(tokenString);
            if(tempTokenString == "true" or tempTokenString == "false") {
                tokenVector.emplace_back(std::make_unique<ast::BooleanToken>(ast::BooleanToken(tempTokenString)));
            }
            else if(tempTokenString == "if" or tempTokenString == "else" or tempTokenString == "var" or tempTokenString == "log" or tempTokenString == "fn") {
                tokenVector.emplace_back(std::make_unique<ast::KeywordToken>(ast::KeywordToken(tempTokenString)));
            }
            else {
                tokenVector.emplace_back(std::make_unique<ast::IdentifierToken>(ast::IdentifierToken(tempTokenString)));
            }
        }

        else {
            switch (source[curPos]) {
                case '+':
                case '*':
                case '/':
                case '-':
                case '^':
                case '%':
                case '|':
                case '&':
                    tokenString[0] = source[curPos++];
                    tokenString[1] = '\0';
                    tokenVector.emplace_back(std::make_unique<ast::OperatorToken>(ast::OperatorToken(std::move(std::string(tokenString)))));
                    break;
                case '~':
                    tokenString[0] = source[curPos++];
                    if(curPos < size) {
                        switch (source[curPos]) {
                            case '|':
                            case '&':
                                tokenString[1] = source[curPos++];
                                tokenString[2] = '\0';
                                tokenVector.emplace_back(std::make_unique<ast::OperatorToken>(ast::OperatorToken(std::move(std::string(tokenString)))));
                                break;
                            default:
                                tokenString[1] = '\0';
                                tokenVector.emplace_back(std::make_unique<ast::OperatorToken>(ast::OperatorToken(std::move(std::string(tokenString)))));
                                break;
                        }
                    }
                    else {
                        tokenString[1] = '\0';
                        tokenVector.emplace_back(std::make_unique<ast::OperatorToken>(ast::OperatorToken(std::move(std::string(tokenString)))));
                    }
                    break;
                case '!':
                    // the operator is an exclusive operator
                    tokenString[0] = source[curPos++];
                    if(curPos < size) {
                        switch (source[curPos]) {
                            case '|':
                            case '&':
                                tokenString[1] = source[curPos++];
                                tokenString[2] = '\0';
                                tokenVector.emplace_back(std::make_unique<ast::OperatorToken>(ast::OperatorToken(std::move(std::string(tokenString)))));
                                break;
                            default:
                                throw UNEXPECTED_TOKEN;
                                break;
                        }
                    }
                    else {
                        throw UNEXPECTED_TOKEN;
                    }
                    break;
                case '<':
                case '>':
                    tokenString[0] = source[curPos++];
                    if(curPos < size) {
                        if(source[curPos] == source[curPos-1] or source[curPos] == '=') {
                            tokenString[1] = source[curPos++];
                            tokenString[2] = '\0';
                            tokenVector.emplace_back(std::make_unique<ast::OperatorToken>(ast::OperatorToken(std::move(std::string(tokenString)))));
                        }
                        else {
                            tokenString[1] = '\0';
                            tokenVector.emplace_back(std::make_unique<ast::OperatorToken>(ast::OperatorToken(std::move(std::string(tokenString)))));
                        }
                    }
                    else {
                        tokenString[1] = '\0';
                        tokenVector.emplace_back(std::make_unique<ast::OperatorToken>(ast::OperatorToken(std::move(std::string(tokenString)))));
                    }
                    break;
                case '=':
                    tokenString[0] = source[curPos++];
                    if(curPos < size) {
                        switch (source[curPos]) {
                            case '>':
                                tokenString[1] = source[curPos++];
                                tokenString[2] = '\0';
                                tokenVector.emplace_back(std::make_unique<ast::OperatorToken>(ast::OperatorToken(std::move(std::string(tokenString)))));
                                break;
                            default:
                                tokenString[1] = '\0';
                                tokenVector.emplace_back(std::make_unique<ast::OperatorToken>(ast::OperatorToken(std::move(std::string(tokenString)))));
                                break;
                        }
                    }
                    else {
                        tokenString[1] = '\0';
                        tokenVector.emplace_back(std::make_unique<ast::OperatorToken>(ast::OperatorToken(std::move(std::string(tokenString)))));
                    }
                    break;
                case ':':
                    tokenString[0] = source[curPos++];
                    if(curPos < size) {
                        switch (source[curPos]) {
                            case '=':
                                tokenString[1] = source[curPos++];
                                tokenString[2] = '\0';
                                tokenVector.emplace_back(std::make_unique<ast::OperatorToken>(ast::OperatorToken(std::move(std::string(tokenString)))));
                                break;
                            default:
                                throw UNEXPECTED_TOKEN;
                                break;
                        }
                    }
                    else {
                        throw UNEXPECTED_TOKEN;
                    }
                    break;
                case '(':
                case ')':
                    tokenString[0] = source[curPos++];
                    tokenString[1] = '\0';
                    tokenVector.emplace_back(std::make_unique<ast::SeparatorToken>(ast::SeparatorToken(std::move(std::string(tokenString)))));
                    break;
                default:
                    throw UNEXPECTED_TOKEN;
                    break;
            }
        }
    }
}

int main() {
    return 0;
}