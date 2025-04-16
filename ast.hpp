#ifndef AST_H
#define AST_H

#include "utils.hpp"
#include "parser.hpp"

namespace ast{

    enum NodeType {
        DEF_AST,
        NUM_AST,
        BOOL_AST,
        CHAR_AST,
        NULL_AST,
        BOP_AST,
        UOP_AST,
        PROG_AST,
        LET_AST,
        ASSIGN_AST,
        VAR_AST,
        IF_AST,
        LOG_AST,
        RET_AST,
        LETFUN_AST
    };

    class AtomicASTNode;
    
    class ASTNode {
    public:

        virtual NodeType type() const {
            return DEF_AST;
        }

        /*virtual AtomicASTNode& const_resolve() {
            // unimplemented
        }*/
    };

    class AtomicASTNode : public ASTNode {
    public:
        virtual types::TYPES dataType() const = 0;

        // AtomicASTNode& const_resolve() override {
        //     return *this;
        // }
    };

    // When storing the numerical values, it is important that the value we store has a type.
    // We shall, by default, analyse the numerical token and assign it a type that takes the
    // least amount of memory.
    template <typename T>
    class NumValue : public AtomicASTNode {
    public:
        T value;
        NumValue(T val): value(val) {
        }

        NodeType type() const override {
            return NUM_AST;
        }

        types::TYPES dataType() const override {
            return T::dt;
        };
    };

    class BoolValue : public AtomicASTNode {
    public:
        NodeType type() const override {
            return BOOL_AST;
        }

        types::TYPES dataType() const override {
            return types::BOOL;
        }
    };

    class CharValue : public AtomicASTNode {
    public:
        NodeType type() const override {
            return CHAR_AST;
        }

        types::TYPES dataType() const override {
            return types::CHAR_8;
        }
    };

    class NullValue : public AtomicASTNode {
    public:
        NodeType type() const override {
            return NULL_AST;
        }

    };

    enum OperatorType {
        UNEG_OP,   // Unary Neg
        NOT_OP,    // Unary Not
        ADD_OP,    // Binary Add
        SUB_OP,    // Binary Subtraction
        MUL_OP,    // Binary Multiplication
        DIV_OP,    // Binary Division
        POW_OP,    // Binary Exponentiation
        MOD_OP,    // Binary Modulo
        OR_OP,     // Binary Or
        AND_OP,    // Binary And
        XOR_OP,    // Binary Xor
        XAND_OP,   // Binary Xand
        LSHIFT_OP, // Binary Left Shift Operator
        RSHIFT_OP, // Binary Right Shift Operator
        EQ_OP,     // Binary Equality
        NEQ_OP,    // Binary Inequality
        LE_OP,     // Binary Less Than
        LEQ_OP,    // Binary Less Than Equal To
        GE_OP,     // Binary Greater Than
        GEQ_OP,    // Binary Greater Than Equal To
    };

    class BinaryOperator : public ASTNode {
    public:
        OperatorType op;
        std::unique_ptr<ASTNode> leftChild, rightChild;

        BinaryOperator(OperatorType operation, std::unique_ptr<ASTNode> lc, std::unique_ptr<ASTNode> rc): op(operation), leftChild(std::move(lc)), rightChild(std::move(rc)) {};

        NodeType type() const override {
            return BOP_AST;
        }
    };

    class UnaryOperator : public ASTNode {
    public:
        OperatorType op;
        std::unique_ptr<ASTNode> child;

        UnaryOperator(OperatorType operation, std::unique_ptr<ASTNode> c): op(operation), child(std::move(c)) {};

        NodeType type() const override {
            return UOP_AST;
        }
    };

    class Prog: public ASTNode{
    public:
        std::vector<std::unique_ptr<ASTNode>> decls;

        Prog(){};
        
        void addDecl(std::unique_ptr<ASTNode>&& decl){
            decls.push_back(std::move(decl));
        }

        NodeType type() const override {
            return PROG_AST;
        }
    };

    class Variable: public ASTNode{
    public:
        std::string varName;

        Variable(std::string varName): varName(varName) {};
        
        NodeType type() const override {
            return VAR_AST;
        }
    };

    enum Access{
        VAR, CONST
    };

    class Let: public ASTNode{
    public:
        std::unique_ptr<ASTNode> var;
        types::TYPES typ;
        Access acc;
        std::unique_ptr<ASTNode> val;

        Let(std::unique_ptr<ASTNode> var, types::TYPES type, Access acc): var(std::move(var)), typ(type), acc(acc), val(nullptr) {};
        Let(std::unique_ptr<ASTNode> var, types::TYPES type, Access acc, std::unique_ptr<ASTNode> val): var(std::move(var)), typ(type), acc(acc), val(std::move(val)) {};
        
        NodeType type() const override {
            return LET_AST;
        }
    };

    class Assign: public ASTNode{
    public:
        std::unique_ptr<ASTNode> var;
        std::unique_ptr<ASTNode> val;

        Assign(std::unique_ptr<ASTNode> var, std::unique_ptr<ASTNode> val): var(std::move(var)), val(std::move(val)) {};
        
        NodeType type() const override {
            return ASSIGN_AST;
        }
    };

    class If: public ASTNode{
    public:
        std::unique_ptr<ASTNode> cond;
        std::unique_ptr<ASTNode> thenBody;
        std::unique_ptr<ASTNode> elseBody;

        If(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> thenBody): cond(std::move(cond)), thenBody(std::move(thenBody)), elseBody(nullptr) {};
        If(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> thenBody, std::unique_ptr<ASTNode> elseBody): cond(std::move(cond)), thenBody(std::move(thenBody)), elseBody(std::move(elseBody)) {};
    
        NodeType type() const override {
            return IF_AST;
        }
    };

    class Log: public ASTNode{
    public:
        std::unique_ptr<ASTNode> val;

        Log(std::unique_ptr<ASTNode> val): val(std::move(val)) {};

        NodeType type() const override {
            return LOG_AST;
        }
    };

    class Return: public ASTNode{
    public:
        std::unique_ptr<ASTNode> val;

        Return(std::unique_ptr<ASTNode> val): val(std::move(val)) {};

        NodeType type() const override {
            return RET_AST;
        }
    };

    class LetFun: public ASTNode{
    public:
        types::TYPES retType;
        std::string name;
        std::vector<std::pair<types::TYPES,std::unique_ptr<ASTNode>>> params;
        std::unique_ptr<ASTNode> body;

        LetFun(types::TYPES retType, std::string name, std::unique_ptr<ASTNode> body): retType(retType), name(name), body(std::move(body)) {};
        
        NodeType type() const override {
            return LETFUN_AST;
        }

        void addParam(types::TYPES typ, std::unique_ptr<ASTNode> param){
            params.push_back({typ,std::move(param)});
        }
    };

    std::unique_ptr<ast::ASTNode> parseTreeToAST(parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertProg(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertDecl(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertVarDecl(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertVal(int node, parser::parseTree &tree);
    VarType convertType(std::string type);
    std::unique_ptr<ast::ASTNode> convertExp(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertAssn(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertUnAmb(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertUnaryOp(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertBinaryOp(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertStmt(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertExpStmt(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertBlock(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertIfStmt(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertElseStmt(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertElifStmts(int node, parser::parseTree &tree, std::unique_ptr<ASTNode> finElse=nullptr);
    std::unique_ptr<ast::ASTNode> convertLogStmt(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertRetStmt(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertFunDecl(int node, parser::parseTree &tree);
    std::pair<types::TYPES,std::unique_ptr<ast::Variable>> convertParam(int node, parser::parseTree &tree);

    void vizTree(const std::unique_ptr<ASTNode>& node, const std::string &prefix, bool isLast);
}

#endif