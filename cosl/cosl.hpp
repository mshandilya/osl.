#ifndef COSL_H
/* The cosl header file not only contains all the necessary libraries for the "compiler for osl" but 
also defines all the necessary structures for osl including types, AST structures, ABT structures,
and token structures.
*/
#define COSL_H

// Necessary libraries used by the compiler.
#include <cctype>
#include <string>
#include <vector>
#include <utility>
#include <memory>

enum ERRORS {
    OK,
    INVALID_TOKEN,
    EXPECTED_NUM_ERR,
    UNEXPECTED_OP,
    UNMATCHED_BKT_ERR,
    NUM_TOO_BIG,
    IMMATURE_EOF
};

/* The `types` namespace contains structures for all the primitive types in the osl language.
*/
namespace types {

    enum MAX_BITS {
        B8 = 8,
        B16 = 16,
        B32 = 32,
        B64 = 64,
        B128 = 128
    };

    /* Signed integers are defined by using the templated `Integer` class which also takes in the number
    of bits used to store the integer. Type aliases for specific bit sizes have been made as `i8` all
    the way to `i128` at powers of 2.
    */
    template<MAX_BITS bitSize>
    class Integer {
        static_assert(bitSize%8==0, "Bit Size for class Integer must be a multiple of 8");
        static_assert(bitSize>0, "Bit Size for class Integer must be a positive integer");
        // Values as stored in the Little Endian System
        unsigned char value[bitSize/8];

    public:
        inline Integer() {
            for(unsigned char& b : value)
                b = 0;
        }

        inline Integer(std::vector<unsigned char>& value) {
            for(unsigned short int i = 0, bytes = bitSize/8; i < bytes; i++)
                this->value[i] = value[i];
        }


    };

    typedef Integer<B8> i8;
    typedef Integer<B16> i16;
    typedef Integer<B32> i32;
    typedef Integer<B64> i64;
    typedef Integer<B128> i128;

    /* Unsigned integers are defined by using the templated `UnsignedInteger` class which also takes in 
    the number of bits used to store the unsigned integer as a template parameter. Type aliases for 
    specific bit sizes have been made as `u8` all the way to `u128` at powers of 2.
    */
    template<MAX_BITS bitSize>
    class UnsignedInteger {
        static_assert(bitSize%8==0, "Bit Size for class UnsignedInteger must be a multiple of 8");
        static_assert(bitSize>0, "Bit Size for class UnsignedInteger must be a positive integer");
        // Values as stored in the Little Endian System
        unsigned char value[bitSize/8];

    public:
        inline UnsignedInteger() {
            for(unsigned char& b : value)
                b = 0;
        }

        inline UnsignedInteger(std::vector<unsigned char>& value) {
            for(unsigned short int i = 0, bytes = bitSize/8; i < bytes; i++)
                this->value[i] = value[i];
        }
    };

    typedef UnsignedInteger<B8> u8;
    typedef UnsignedInteger<B16> u16;
    typedef UnsignedInteger<B32> u32;
    typedef UnsignedInteger<B64> u64;
    typedef UnsignedInteger<B128> u128;

    template<MAX_BITS bitSize>
    class Float {
        static_assert(bitSize%8==0, "Bit Size for class Float must be a multiple of 8");
        static_assert(bitSize>0, "Bit Size for class Float must be a positive integer");
        // Values as stored in the Little Endian System
        unsigned char value[bitSize/8];

    public:
        inline Float() {
            for(unsigned char& b : value)
                b = 0;
        }
    };

    typedef Float<B8> f8;
    typedef Float<B16> f16;
    typedef Float<B32> f32;
    typedef Float<B64> f64;
    typedef Float<B128> f128;

//    c8 // UTF 8
//    c16 // UTF 16 --
}

namespace utils {

    std::pair<types::MAX_BITS, std::vector<unsigned char>> stringToNumberUtil(std::string& source) {
        unsigned short int bitSize = 8;
        size_t s = source.length();
        int base = 10, startPos = 0, neg = 0;
        if(s>0 and source[0]=='-') {
            neg = 1;
            startPos++;
        }
        if(s-startPos>1 and source[startPos] == '0') {
            if(source[startPos+1] == 'b' or source[startPos+1] == 'B') {
                base = 2;
                startPos += 2;
            }
            else if(source[startPos+1] == 'o' or source[startPos+1] == 'O') {
                base = 8;
                startPos += 2;
            }
            else if(source[startPos+1] == 'x' or source[startPos+1] == 'X') {
                base = 16;
                startPos += 2;
            }
        }

        std::vector<unsigned char> value(1);
        if(base==16) {
            int bitPos = 0, bytePos = 0, carry = 0, newVal;
            while(startPos < s) {
                carry = 0;
                if(bytePos==(bitSize/8 - 1) and bitPos == 7) {
                    bitSize *= 2;
                }
                for(int curPos = startPos; curPos < s; curPos++) {
                    if(source[curPos]>='a' and source[curPos]<='f') {
                        newVal = (source[curPos] - 'a' + 10 + carry * base) / 2;
                        carry = newVal/base;
                        newVal %= base;
                        if(newVal<10)
                            source[curPos] = (char)(newVal+'0');
                        else
                            source[curPos] = (char)(newVal-10+'a');
                        if(newVal == 0 and curPos == startPos)
                            startPos++;
                    }
                    else if(source[curPos] >= 'A' and source[curPos] <= 'F') {
                        newVal = (source[curPos] - 'A' + 10 + carry * base) / 2;
                        carry = newVal/base;
                        newVal %= base;
                        if(newVal<10)
                            source[curPos] = (char)(newVal+'0');
                        else
                            source[curPos] = (char)(newVal-10+'a');
                        if(newVal == 0 and curPos == startPos)
                            startPos++;
                    }
                    else if(source[curPos]>='0' and source[curPos]<='9') {
                        newVal = (source[curPos] - '0' + carry * base) / 2;
                        carry = newVal/base;
                        newVal %= base;
                        if(newVal<10)
                            source[curPos] = (char)(newVal+'0');
                        else
                            source[curPos] = (char)(newVal-10+'a');
                        if(newVal == 0 and curPos == startPos)
                            startPos++;
                    }
                    else {
                        // at this instant fraction or exponent must have occurred
                        break;
                    }
                }
                value[bytePos] |= (carry<<(bitPos++));
                if(bitPos==8) {
                    bytePos++;
                    value.push_back(0);
                }
                bitPos %= 8;
            }
        }
        else {
            int bitPos = 0, bytePos = 0, carry = 0, newVal;
            while(startPos < s) {
                carry = 0;
                if(bytePos==(bitSize/8 - 1) and bitPos == 7) {
                    bitSize *= 2;
                }
                for(int curPos = startPos; curPos < s; curPos++) {
                    if(source[curPos]>='0' and source[curPos]<=('0'+base-1)) {
                        newVal = (source[curPos] - 'a' + 10 + carry * base) / 2;
                        carry = newVal/base;
                        newVal %= base;
                        if(newVal<10)
                            source[curPos] = (char)(newVal+'0');
                        else
                            source[curPos] = (char)(newVal-10+'a');
                        if(newVal == 0 and curPos == startPos)
                            startPos++;
                    }
                    else {
                        throw INVALID_TOKEN;
                    }
                }
                value[bytePos] |= (carry<<(bitPos++));
                if(bitPos==8) {
                    bytePos++;
                    value.push_back(0);
                }
                bitPos %= 8;
            }
        }
        if(neg) {
            for(unsigned char& b : value) {
                b = ~b;
                if(neg) {
                    neg = (int)(b==255);
                    b++;
                }
            }
        }
        switch (bitSize) {
            case 8:
                return std::make_pair(types::B8, value);
            case 16:
                return std::make_pair(types::B16, value);
            case 32:
                return std::make_pair(types::B32, value);
            case 64:
                return std::make_pair(types::B64, value);
            case 128:
                return std::make_pair(types::B128, value);
            default:
                throw NUM_TOO_BIG;
        }
    }

}

namespace ast {
    enum NodeType {
        DEF_AST,
        NUM_AST,
        BOOL_AST,
        BOP_AST,
        UOP_AST
    };

    class AtomicASTNode;
    
    class ASTNode {
    public:

        virtual NodeType type() {
            return DEF_AST;
        }

        virtual AtomicASTNode& const_resolve() {
            // unimplemented
        }
    };

    class AtomicASTNode : public ASTNode {
    public:
        AtomicASTNode& const_resolve() override {
            return *this;
        }
    };

    // When storing the numerical values, it is important that the value we store has a type.
    // We shall, by default, analyse the numerical token and assign it a type that takes the
    // least amount of memory.
    template <typename T>
    class NumericalValue : public AtomicASTNode {
        T value;
    public:
        NumericalValue(T val): value(val) {
        }

        NodeType type() override {
            return NUM_AST;
        }
    };

    class BooleanValue : public AtomicASTNode {
    public:
        NodeType type() override {
            return BOOL_AST;
        }
    };

    enum OperatorType {
        ATH_ADD,
        ATH_UADD,
        ATH_SUB,
        ATH_USUB,
        ATH_MUL,
        ATH_DIV,
        ATH_MOD,
        ATH_POW,
        ATH_NOT,
        ATH_AND,
        ATH_OR,
        ATH_XOR,
        ATH_XNOR,
        ATH_NOR,
        ATH_NAND,
        ATH_LS,
        ATH_RS,
        BKT_PREC, // Precedence forced by brackets operator
    };

    class BinaryOperator : public ASTNode {
        OperatorType op;
        std::unique_ptr<ast::ASTNode> leftChild, rightChild;
    public:
        BinaryOperator(OperatorType operation, ASTNode& lc, ASTNode& rc): op(operation), leftChild(std::make_unique<ASTNode>(lc)), rightChild(std::make_unique<ASTNode>(rc)) {};

        NodeType type() override {
            return BOP_AST;
        }
    };

    class UnaryOperator : public ASTNode {
        OperatorType op;
        std::unique_ptr<ASTNode> child;
    public:
        UnaryOperator(OperatorType operation, ASTNode& c): op(operation), child(std::make_unique<ASTNode>(c)) {};

        NodeType type() override {
            return UOP_AST;
        }
    };

    enum TokenType {
        DEF_TOKEN, // Reserved Word (typically tokens which do not have a meaning yet)
        NUM_TOKEN, // Numerical Token
        SEP_TOKEN, // Separator Token
        OP_TOKEN, // Operator Token
        KEY_TOKEN // Keyword Token
    };

    class Token {
        std::string value;
    public:        
        Token(std::string&& val) : value(std::move(val)) {}

        virtual TokenType type() = 0;

        std::string &getVal() {
            return value;
        }
    };

    class NumericalToken : public Token {
    public:
        NumericalToken(std::string&& val) : Token(std::move(val)) {}

        TokenType type() override {
            return TokenType::NUM_TOKEN;
        }
    };

    class SeparatorToken : public Token {
    public:
        SeparatorToken(std::string&& val) : Token(std::move(val)) {}

        TokenType type() override {
            return TokenType::SEP_TOKEN;
        }
    };
    
    class OperatorToken : public Token {
    public:
        OperatorToken(std::string&& val) : Token(std::move(val)) {}

        TokenType type() override {
            return TokenType::OP_TOKEN;
        }
    };

    class KeywordToken : public Token {
    public:
        KeywordToken(std::string&& val) : Token(std::move(val)) {}

        TokenType type() override {
            return TokenType::KEY_TOKEN;
        }
    };
}

class Parser {
    ast::ASTNode root;
    std::string source;
    std::vector<std::unique_ptr<ast::Token>> tokens;

    ast::ASTNode parseAtomicNumber(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end);
    ast::ASTNode parseUnAmb(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end);
    ast::ASTNode parseUnNeg(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end);
    ast::ASTNode parseUnNot(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end);
    ast::ASTNode parseAdd(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end);
    ast::ASTNode parseMul(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end);
    ast::ASTNode parseAnd(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end);
    ast::ASTNode parseOr(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end);
    ast::ASTNode parseXor(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end);
    ast::ASTNode parseXnor(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end);
    ast::ASTNode parseArithmeticExpression(std::vector<std::unique_ptr<ast::Token>>::iterator& begin, std::vector<std::unique_ptr<ast::Token>>::iterator& end);
public:
    Parser(std::string&& code) : source(std::move(code)) {

    }
};

void lexer(const std::string& source, std::vector<std::unique_ptr<ast::Token>>& tokenVector);

#endif