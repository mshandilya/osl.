#ifndef AST_H
#define AST_H

#include "utils.hpp"
#include "parser.hpp"

namespace types {
    enum Type{
        INT,
        UNSIGNED,
        FLOAT,
        CHAR,
        BOOL
    };

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

namespace ast{

    enum NodeType {
        DEF_AST,
        NUM_AST,
        BOOL_AST,
        BOP_AST,
        UOP_AST,
        PROG_AST,
        LET_AST,
        VAR_AST
    };

    class AtomicASTNode;
    
    class ASTNode {
    public:

        virtual NodeType type() const {
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

        NodeType type() const override {
            return NUM_AST;
        }
    };

    class BooleanValue : public AtomicASTNode {
    public:
        NodeType type() const override {
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
    public:
        OperatorType op;
        std::unique_ptr<ASTNode> leftChild, rightChild;

        BinaryOperator(OperatorType operation, ASTNode& lc, ASTNode& rc): op(operation), leftChild(std::make_unique<ASTNode>(lc)), rightChild(std::make_unique<ASTNode>(rc)) {};

        NodeType type() const override {
            return BOP_AST;
        }
    };

    class UnaryOperator : public ASTNode {
    public:
        OperatorType op;
        std::unique_ptr<ASTNode> child;

        UnaryOperator(OperatorType operation, ASTNode& c): op(operation), child(std::make_unique<ASTNode>(c)) {};

        NodeType type() const override {
            return UOP_AST;
        }
    };

    class Prog: public ASTNode{
    public:
        std::vector<std::shared_ptr<ASTNode>> decls;

        Prog(){};
        
        void addDecl(std::unique_ptr<ASTNode> decl){
            decls.push_back(std::move(decl));
        }

        NodeType type() const override {
            return PROG_AST;
        }

        friend std::ostream& operator<<(std::ostream& os, Prog const& p);
    };

    class VarType{
    public:
        types::Type type;
        types::MAX_BITS size;

        VarType() {};
        VarType(types::Type type): type(type) {};
        VarType(types::Type type, types::MAX_BITS size): type(type), size(size) {};
    };

    class Variable: public ASTNode{
    public:
        std::string varName;

        Variable(std::string varName): varName(varName) {};
        friend std::ostream& operator<<(std::ostream& os, Variable const& node);
        
        NodeType type() const override {
            return VAR_AST;
        }
    };

    class Let: public ASTNode{
    public:
        std::unique_ptr<Variable> var;
        VarType typ;
        std::unique_ptr<ASTNode> val;

        Let(Variable &var, VarType type): var(std::make_unique<Variable>(var)), typ(type) {};
        Let(Variable &var, VarType type, ASTNode &val): var(std::make_unique<Variable>(var)), typ(type), val(std::make_unique<ASTNode>(val)) {};
        friend std::ostream& operator<<(std::ostream& os, Let const& node);
        
        NodeType type() const override {
            return LET_AST;
        }
    };

    Prog parseTreeToAST(parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertDecl(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertVarDecl(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertVal(int node, parser::parseTree &tree);
    VarType convertType(std::string type);
    std::unique_ptr<ast::ASTNode> convertExp(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertAssn(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertUnAmb(int node, parser::parseTree &tree);
    
    enum ACCESS{
        VAR, CONST
    };

//    std::string opToString(OperatorType op);
//    std::string getNodeLabel(const ASTNode* node);
    void vizTree(const ASTNode* node, const std::string &prefix, bool isLast);
}

#endif