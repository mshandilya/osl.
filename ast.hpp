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
        BOP_AST,
        UOP_AST,
        PROG_AST,
        LET_AST,
        ASSIGN_AST,
        VAR_AST,
        IF_AST
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
    /*template <typename T>
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
    };*/
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

    class BoolValue : public ASTNode {
    public:
        NodeType type() const override {
            return BOOL_AST;
        }
    };

    class CharValue : public ASTNode {
    public:
        NodeType type() const override {
            return CHAR_AST;
        }
    };

    enum OperatorType {
        UNEG_OP, //Unary Neg
        NOT_OP, //Not
        ADD_OP,
        SUB_OP,
        MUL_OP,
        DIV_OP,
        POW_OP,
        MOD_OP,
        OR_OP,
        AND_OP,
        XOR_OP,
        XAND_OP,
        LSHIFT_OP,
        RSHIFT_OP,
        EQ_OP,
        NEQ_OP,
        LE_OP,
        LEQ_OP,
        GE_OP,
        GEQ_OP
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

    class VarType{
    public:
        types::TYPES type;
        types::MAX_BITS size;

        VarType() {};
        VarType(types::TYPES type): type(type) {};
        VarType(types::TYPES type, types::MAX_BITS size): type(type), size(size) {};
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
        VarType typ;
        Access acc;
        std::unique_ptr<ASTNode> val;

        Let(std::unique_ptr<ASTNode> var, VarType type, Access acc): var(std::move(var)), typ(type), acc(acc), val(nullptr) {};
        Let(std::unique_ptr<ASTNode> var, VarType type, Access acc, std::unique_ptr<ASTNode> val): var(std::move(var)), typ(type), acc(acc), val(std::move(val)) {};
        
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

    void vizTree(const std::unique_ptr<ASTNode>& node, const std::string &prefix, bool isLast);
}

#endif