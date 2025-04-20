#ifndef AST_H
#define AST_H

#include "utils.hpp"
#include "parser.hpp"

namespace ast{

    enum NodeType {
        DEF_AST,
        PROG_AST,
        BLOCK_AST,
        LOOP_AST,
        COND_AST,
        RET_AST,
        LOG_AST,
        LETFUN_AST,
        LETVAR_AST,
        LETCONST_AST,
        VAL_AST,
        ASSIGN_AST,
        LOC_AST,
        BOP_AST,
        UOP_AST,
        FUNCALL_AST,
        ARR_AST,
        NUM_AST,
        CHAR_AST,
        BOOL_AST,
        NULL_AST,
        IDEN_AST,
    };

    enum OperatorType {
        GT_OP,       // Binary Greater Than
        GEQ_OP,      // Binary Greater Than Equal To
        LT_OP,       // Binary Less Than
        LEQ_OP,      // Binary Less Than Equal To
        NEQ_OP,      // Binary Inequality
        EQ_OP,       // Binary Equality
        MUL_OP,      // Binary Multiplication
        ADD_OP,      // Binary Add
        DIV_OP,      // Binary Division
        SUB_OP,      // Binary Subtraction
        POW_OP,      // Binary Exponentiation
        MOD_OP,      // Binary Modulo
        OR_OP,       // Binary Or
        AND_OP,      // Binary And
        XOR_OP,      // Binary Xor
        XAND_OP,     // Binary Xand
        LSHIFT_OP,   // Binary Left Shift Operator
        RSHIFT_OP,   // Binary Right Shift Operator
        ARRACC_OP,   // Binary Operator for Array Access
        UNEG_OP,     // Unary Neg
        UNOT_OP,     // Unary Not
        PTRREF_OP,   // Unary Reference
        PTRDEREF_OP, // Unary Pointer Dereferencing
    };

    enum Access{
        CONST,
        VAR
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
        virtual types::ATOMTYPES dataType() const = 0;

        // AtomicASTNode& const_resolve() override {
        //     return *this;
        // }
    };

    class Identifier: public AtomicASTNode {
    public:
        std::string idenName;
        std::unique_ptr<types::Type> boundDataType;
        ast::Access access;
        int id, scopeId;

        Identifier(std::string idenName): idenName(idenName) {};
        
        void bind(std::unique_ptr<types::Type>&& boundDataType, Access access, int id, int scopeId) {
            this->boundDataType = std::move(boundDataType);
            this->access = access;
            this->id = id;
            this->scopeId = scopeId;
        }

        NodeType type() const override {
            return IDEN_AST;
        }
    };
    
    class NullValue : public AtomicASTNode {
        std::unique_ptr<types::Type> dt;
    public:
        NullValue(std::unique_ptr<types::Type>&& dt) : dt(std::move(dt)) {}

        NodeType type() const override {
            return NULL_AST;
        }

        types::ATOMTYPES dataType() const override {
            return types::NULLV;
        }

        // functions for more information on what the enclosing data type is (dt)

    };
    
    class BoolValue : public AtomicASTNode {
        types::Boolean val;
    public:
        BoolValue() : val(false) {}

        BoolValue(bool val) : val(val) {}

        NodeType type() const override {
            return BOOL_AST;
        }

        types::ATOMTYPES dataType() const override {
            return types::BOOL;
        }
    };

    class CharValue : public AtomicASTNode {
        types::Character val;
    public:
        CharValue() : val(0) {}

        CharValue(types::Character& val): val(val) {}

        NodeType type() const override {
            return CHAR_AST;
        }

        types::ATOMTYPES dataType() const override {
            return types::CHAR_8;
        }
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

        types::ATOMTYPES dataType() const override {
            return T::dt;
        };
    };

    class ArrValue : public ASTNode {
        std::vector<std::unique_ptr<ast::ASTNode>> elems;
    public:
        ArrValue() {}    

        void addElem(std::unique_ptr<ast::ASTNode>&& elem) {
            elems.push_back(std::move(elem));
        }

        NodeType type() const override {
            return ARR_AST;
        }
    };

    class FunctionCall : public ASTNode {
    public:
        std::unique_ptr<ASTNode> name;
        std::vector<std::unique_ptr<ASTNode>> params;
        
        FunctionCall(std::unique_ptr<ASTNode>&& name): name(std::move(name)) {};
    
        NodeType type() const override {
            return FUNCALL_AST;
        }

        void addParam(std::unique_ptr<ASTNode>&& param){
            params.push_back(std::move(param));
        }
    };

    class UnaryOperator : public ASTNode {
    public:
        OperatorType op;
        std::unique_ptr<ASTNode> child;

        UnaryOperator(OperatorType operation, std::unique_ptr<ASTNode>&& c): op(operation), child(std::move(c)) {};

        NodeType type() const override {
            return UOP_AST;
        }
    };
    
    class BinaryOperator : public ASTNode {
    public:
        OperatorType op;
        std::unique_ptr<ASTNode> leftChild, rightChild;

        BinaryOperator(OperatorType operation, std::unique_ptr<ASTNode>&& lc, std::unique_ptr<ASTNode>&& rc): op(operation), leftChild(std::move(lc)), rightChild(std::move(rc)) {};

        NodeType type() const override {
            return BOP_AST;
        }
    };

    class Assign : public ASTNode {
    public:
        std::unique_ptr<ASTNode> var;
        std::unique_ptr<ASTNode> val;

        Assign(std::unique_ptr<ASTNode>&& var, std::unique_ptr<ASTNode>&& val): var(std::move(var)), val(std::move(val)) {};
        
        NodeType type() const override {
            return ASSIGN_AST;
        }
    };

    class LetConst: public ASTNode {
    public:
        std::unique_ptr<types::Type> typ;
        std::unique_ptr<ASTNode> var;
        std::unique_ptr<ASTNode> val;

        LetConst(std::unique_ptr<ASTNode>&& var, std::unique_ptr<types::Type>&& type, std::unique_ptr<ASTNode>&& val): var(std::move(var)), typ(type), val(std::move(val)) {};
        
        NodeType type() const override {
            return LETCONST_AST;
        }
    };
    
    class LetVar: public ASTNode {
    public:
        std::unique_ptr<types::Type> typ;
        std::unique_ptr<ASTNode> var;
        std::unique_ptr<ASTNode> val;

        LetVar(std::unique_ptr<ASTNode>&& var, std::unique_ptr<types::Type>&& type): var(std::move(var)), typ(type), val(std::make_unique<NullValue>()) {};
        LetVar(std::unique_ptr<ASTNode>&& var, std::unique_ptr<types::Type>&& type, std::unique_ptr<ASTNode>&& val): var(std::move(var)), typ(type), val(std::move(val)) {};
        
        NodeType type() const override {
            return LETVAR_AST;
        }
    };
    
    class LetFun: public ASTNode {
    public:
        std::unique_ptr<types::Type> retType;
        std::unique_ptr<ASTNode> name;
        std::vector<std::pair<std::unique_ptr<types::Type>, std::unique_ptr<ASTNode>>> params;
        std::unique_ptr<ASTNode> body;

        LetFun(std::unique_ptr<types::Type>&& retType, std::unique_ptr<ASTNode>&& name, std::unique_ptr<ASTNode>&& body): retType(std::move(retType)), name(std::move(name)), body(std::move(body)) {};
        
        NodeType type() const override {
            return LETFUN_AST;
        }

        void addParam(std::unique_ptr<types::Type>&& typ, std::unique_ptr<ASTNode>&& param){
            params.emplace_back(std::move(typ), std::move(param));
        }
    };

    class Log: public ASTNode {
    public:
        std::unique_ptr<ASTNode> val;

        Log(std::unique_ptr<ASTNode>&& val): val(std::move(val)) {};

        NodeType type() const override {
            return LOG_AST;
        }
    };

    class Return: public ASTNode {
    public:
        std::unique_ptr<ASTNode> val;

        Return(std::unique_ptr<ASTNode>&& val): val(std::move(val)) {};

        NodeType type() const override {
            return RET_AST;
        }
    };

    class Conditional: public ASTNode {
    public:
        std::unique_ptr<ASTNode> cond;
        std::unique_ptr<ASTNode> thenBody;
        std::unique_ptr<ASTNode> elseBody;

        Conditional(std::unique_ptr<ASTNode>&& cond, std::unique_ptr<ASTNode>&& thenBody): cond(std::move(cond)), thenBody(std::move(thenBody)), elseBody(nullptr) {};
        Conditional(std::unique_ptr<ASTNode>&& cond, std::unique_ptr<ASTNode>&& thenBody, std::unique_ptr<ASTNode>&& elseBody): cond(std::move(cond)), thenBody(std::move(thenBody)), elseBody(std::move(elseBody)) {};
    
        NodeType type() const override {
            return COND_AST;
        }
    };

    class Loop: public ASTNode {
    public:
        std::unique_ptr<ASTNode> cond;
        std::unique_ptr<ASTNode> body;
        
        Loop(std::unique_ptr<ASTNode>&& cond, std::unique_ptr<ASTNode>&& body): cond(std::move(cond)), body(std::move(body)) {};
    
        NodeType type() const override {
            return LOOP_AST;
        }
    };

    class Block : public ASTNode {
    public:
        std::vector<std::unique_ptr<ASTNode>> decls;

        Block() {};
        
        void addDecl(std::unique_ptr<ASTNode>&& decl){
            decls.push_back(std::move(decl));
        }

        NodeType type() const override {
            return BLOCK_AST;
        }
    };

    class Prog : public ASTNode {
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

    std::unique_ptr<types::Type> convertType(std::string type);
    std::pair<std::unique_ptr<types::Type>,std::unique_ptr<ast::Identifier>> convertParam(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> parseTreeToAST(parser::parseTree &tree); //Done
    std::unique_ptr<ast::ASTNode> convertProg(int node, parser::parseTree &tree); //Done
    std::unique_ptr<ast::ASTNode> convertDecl(int node, parser::parseTree &tree); //Done
    std::unique_ptr<ast::ASTNode> convertConstDecl(int node, parser::parseTree &tree); //Add decltype
    std::unique_ptr<ast::ASTNode> convertVarDecl(int node, parser::parseTree &tree); //Add decltype
    std::unique_ptr<ast::ASTNode> convertVal(int node, parser::parseTree &tree); //Done
    std::unique_ptr<ast::ASTNode> convertExp(int node, parser::parseTree &tree); //Done
    std::unique_ptr<ast::ASTNode> convertAssn(int node, parser::parseTree &tree); //Done
    std::unique_ptr<ast::ASTNode> convertUnAmb(int node, parser::parseTree &tree); //Done
    std::unique_ptr<ast::ASTNode> convertAtom(int node, parser::parseTree &tree); //Done
    std::unique_ptr<ast::ASTNode> convertUnaryOp(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertBinaryOp(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertStmt(int node, parser::parseTree &tree); //Done
    std::unique_ptr<ast::ASTNode> convertExpStmt(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertBlock(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertCond(int node, parser::parseTree &tree); //Done
    std::unique_ptr<ast::ASTNode> convertElse(int node, parser::parseTree &tree); //Done
    std::unique_ptr<ast::ASTNode> convertElifs(int node, parser::parseTree &tree, std::unique_ptr<ASTNode> finElse=nullptr); //Done
    std::unique_ptr<ast::ASTNode> convertLogStmt(int node, parser::parseTree &tree); //Done
    std::unique_ptr<ast::ASTNode> convertRetStmt(int node, parser::parseTree &tree); //Done
    std::unique_ptr<ast::ASTNode> convertFunDecl(int node, parser::parseTree &tree);
    std::unique_ptr<ast::ASTNode> convertLoop(int node, parser::parseTree &tree); //Done
    std::unique_ptr<ast::ASTNode> convertFunCall(int node, parser::parseTree &tree);
    std::vector<std::unique_ptr<ASTNode>> convertCallParams(int node, parser::parseTree &tree);

    void vizTree(const std::unique_ptr<ASTNode>& node, const std::string &prefix, bool isLast);
}

#endif