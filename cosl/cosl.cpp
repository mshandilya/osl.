#include <cosl.hpp>

// Program --> Lexer --> Parser --> AST --> Constant Expression Optimization
// --> ABT --> Function Optimization ??--> Executable

ast::ASTNode Parser::parseAtomicNumber(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {

}

ast::ASTNode Parser::parseArithmeticExpression(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {
    switch((*begin)->type()) {
        case ast::OP_TOKEN:
            // prefix operation
            switch((*begin)->val) {
                case "+":
                case "-":
                    return parseUnNeg(begin, end);
                case "~":
                    return parseUnNot(begin, end);
            }
            // check for the operation (~, -, or +), and go to necessary parse
            break;
        default:
            // all other cases first expect an UnAmb
            ast::ASTNode lhs = parseUnAmb(begin, end);
            switch((*begin)->val) {
                case "+":
                    ast::ASTNode rhs = parseAdd(++begin, end);
                    lhs = ast::BinaryOperator op(ast::ATH_ADD, lhs, rhs);
                    switch((*begin)->val) {
                        case "-":
                            rhs = parseUnAmb(++begin, end);
                            return ast::BinaryOperator op(ast::ATH_SUB, lhs, rhs);
                        default:
                            return lhs;
                    }
                case "-":
                    ast::ASTNode rhs = parseUnAmb(++begin, end);
                    return ast::BinaryOperator op(ast::ATH_SUB, lhs, rhs);
                case "*":
                    ast::ASTNode rhs = parseMul(++begin, end);
                    lhs = ast::BinaryOperator op(ast::ATH_MUL, lhs, rhs);
                    switch((*begin)->val) {
                        case "/":
                            rhs = parseUnAmb(++begin, end);
                            return ast::BinaryOperator op(ast::ATH_DIV, lhs, rhs);
                        default:
                            return lhs;
                    }
                case "/":
                    ast::ASTNode rhs = parseUnAmb(++begin, end);
                    return ast::BinaryOperator op(ast::ATH_DIV, lhs, rhs);
                case "^":
                    ast::ASTNode rhs = parseUnAmb(++begin, end);
                    return ast::BinaryOperator op(ast::ATH_POW, lhs, rhs);
                case "%":
                    ast::ASTNode rhs = parseUnAmb(++begin, end);
                    return ast::BinaryOperator op(ast::ATH_MOD, lhs, rhs);
                case ">>":
                    ast::ASTNode rhs = parseUnAmb(++begin, end);
                    return ast::BinaryOperator op(ast::ATH_RS, lhs, rhs);
                case "<<":
                    ast::ASTNode rhs = parseUnAmb(++begin, end);
                    return ast::BinaryOperator op(ast::ATH_LS, lhs, rhs);
                case "|":
                    ast::ASTNode rhs = parseOr(++begin, end);
                    return ast::BinaryOperator op(ast::ATH_OR, lhs, rhs);
                case "&":
                    ast::ASTNode rhs = parseAnd(++begin, end);
                    return ast::BinaryOperator op(ast::ATH_AND, lhs, rhs);
                case "!|":
                    ast::ASTNode rhs = parseXor(++begin, end);
                    return ast::BinaryOperator op(ast::ATH_XOR, lhs, rhs);
                case "!&":
                    ast::ASTNode rhs = parseXnor(++begin, end);
                    return ast::BinaryOperator op(ast::ATH_XNOR, lhs, rhs);
            }
    }
}

//    static std::pair<int, int> bindingPower(ast::OperatorType op) {
//        switch (op) {
//            case ast::ATH_ADD:
//            case ast::ATH_SUB:
//                return std::make_pair(2, 1);
//            case ast::ATH_MUL:
//            case ast::ATH_DIV:
//                return std::make_pair(4, 3);
//
//        }
//    }
//
//    ast::ASTNode parseAtomicNumber(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {
//        if(begin == end) {
//            // error: expected a number but found nothing
//        }
//        switch((**begin).type()) {
//            case ast::TokenType::NUM_TOKEN:
//                return ast::NumericalValue(*begin);
//                break;
//            default:
//                // error: expected an atomic value but found something else
//        }
//    }
//
//    ast::ASTNode parseNumericalExpression(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end) {
//        if(begin == end) {
//            // error: expected an expression but found nothing
//        }
//        ast::ASTNode lhs;
//        switch((**begin).type()) {
//            case ast::TokenType::SEP_TOKEN:
//                if((**begin).getVal() == "(") {
//                    ast::ASTNode child = parseNumericalExpression(++begin, end);
//                    if(begin != end and (**begin).type() == ast::TokenType::SEP_TOKEN and (**begin).getVal() == ")") {
//                        ++begin;
//                        return ast::UnaryOperator(ast::OperatorType::BKT_PREC, child);
//                    }
//                    else {
//                        // error: unclosed bracket
//                    }
//                }
//                else {
//                    // error: unexpected token for parsing expressions
//                }
//                break;
//            case ast::TokenType::OP_TOKEN:
//
//                lhs = parseAtomicNumber(++begin, end);
//                break;
//            default:
//
//                break;
//        }

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
                                // unexpected token
                                break;
                        }
                    }
                    else {
                        // unexpected token
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
                                // error: unexpected token
                                break;
                        }
                    }
                    else {
                        // error: unexpected token
                    }
                    break;
                case '(':
                case ')':
                    tokenString[0] = source[curPos++];
                    tokenString[1] = '\0';
                    tokenVector.emplace_back(std::make_unique<ast::SeparatorToken>(ast::SeparatorToken(std::move(std::string(tokenString)))));
                    break;
                default:
                    // error: unexpected token
                    break;
            }
        }
    }
}