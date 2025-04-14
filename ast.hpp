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
        VAR_AST
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

    /*class AtomicASTNode : public ASTNode {
    public:
        AtomicASTNode& const_resolve() override {
            return *this;
        }
    };*/

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
    class NumValue : public ASTNode {
        T value;
    public:
        NumValue(T val): value(val) {
        }

        NodeType type() const override {
            return NUM_AST;
        }
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

    std::unique_ptr<ast::ASTNode> parseTreeToAST(parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertDecl(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertVarDecl(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertVal(int node, parser::parseTree &tree);
    VarType convertType(std::string type);
    std::unique_ptr<ast::ASTNode> convertExp(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertAssn(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertUnAmb(int node, parser::parseTree &tree);

    void vizTree(const std::unique_ptr<ASTNode>& node, const std::string &prefix, bool isLast);
}

#endif