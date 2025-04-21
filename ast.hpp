#ifndef AST_H
#define AST_H

#include "utils.hpp"
#include "parser.hpp"

namespace ast{

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

        types::ATOMTYPES dataType() const override {
            return types::ATOMTYPES::UNRESOLVED;
        };
    };
    
    class NullValue : public AtomicASTNode {
        std::unique_ptr<types::Type> dt;
    public:
        NullValue() : dt(std::make_unique<types::Type>(types::Null())) {}

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
    public:
        std::unique_ptr<types::Boolean> val;
        BoolValue(std::unique_ptr<types::Boolean>&& val): val(std::move(val)) {}

        NodeType type() const override {
            return BOOL_AST;
        }

        types::ATOMTYPES dataType() const override {
            return types::BOOL;
        }
    };

    class CharValue : public AtomicASTNode {
    public:
        std::unique_ptr<types::Character> val;
        CharValue(std::unique_ptr<types::Character>&& val): val(std::move(val)) {}

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
    class NumValue : public AtomicASTNode {
    public:
        std::unique_ptr<types::NumberType> val;
        NumValue(std::unique_ptr<types::NumberType>&& val): val(std::move(val)) {
        }

        NodeType type() const override {
            return NUM_AST;
        }

        types::ATOMTYPES dataType() const override {
            return val->atomicName();
        };
    };

    class ArrValue : public ASTNode {
    public:
        std::vector<std::unique_ptr<ast::ASTNode>> elems;
        
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
        std::unique_ptr<types::Type> myType;
        std::unique_ptr<ASTNode> var;
        std::unique_ptr<ASTNode> val;

        LetConst(std::unique_ptr<ASTNode>&& var, std::unique_ptr<types::Type>&& type, std::unique_ptr<ASTNode>&& val): var(std::move(var)), myType(std::move(type)), val(std::move(val)) {};
        
        NodeType type() const override {
            return LETCONST_AST;
        }
    };
    
    class LetVar: public ASTNode {
    public:
        std::unique_ptr<types::Type> myType;
        std::unique_ptr<ASTNode> var;
        std::unique_ptr<ASTNode> val;

        LetVar(std::unique_ptr<ASTNode>&& var, std::unique_ptr<types::Type>&& type): var(std::move(var)), myType(std::move(type)), val(std::make_unique<NullValue>()) {};
        LetVar(std::unique_ptr<ASTNode>&& var, std::unique_ptr<types::Type>&& type, std::unique_ptr<ASTNode>&& val): var(std::move(var)), myType(std::move(type)), val(std::move(val)) {};
        
        NodeType type() const override {
            return LETVAR_AST;
        }
    };
    
    class LetArray: public ASTNode {
    public:
        std::unique_ptr<types::ArrayDeclType> myType;
        std::unique_ptr<ASTNode> name;
        Access access;
        std::unique_ptr<ASTNode> val;

        NodeType type() const override {
            return LETARR_AST;
        }

        LetArray(std::unique_ptr<types::ArrayDeclType>&& type, std::unique_ptr<ASTNode>&& name, Access& access): myType(std::move(type)), name(std::move(name)), access(access), val(std::make_unique<NullValue>()) {};
        LetArray(std::unique_ptr<types::ArrayDeclType>&& type, std::unique_ptr<ASTNode>&& name, Access& access, std::unique_ptr<ASTNode>&& val): myType(std::move(type)), name(std::move(name)), access(access), val(std::move(val)) {};
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
            params.push_back({std::move(typ), std::move(param)});
            LOG("params.back().first->name()")
            LOG(params.back().first->name())
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

        Conditional(std::unique_ptr<ASTNode>&& cond, std::unique_ptr<ASTNode>&& thenBody): cond(std::move(cond)), thenBody(std::move(thenBody)), elseBody(std::make_unique<NullValue>()) {};
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

    class Block: public ASTNode {
    public:
        std::vector<std::unique_ptr<ASTNode>> decls;

        Block() : decls(std::vector<std::unique_ptr<ASTNode>>()) {};
        
        void addDecl(std::unique_ptr<ASTNode>&& decl){
            decls.push_back(std::move(decl));
        }

        NodeType type() const override {
            return BLOCK_AST;
        }
    };

    class Prog: public ASTNode {
    public:
        std::vector<std::unique_ptr<ASTNode>> decls;

        Prog() : decls(std::vector<std::unique_ptr<ASTNode>>()) {};
        
        void addDecl(std::unique_ptr<ASTNode>&& decl){
            decls.push_back(std::move(decl));
        }

        NodeType type() const override {
            return PROG_AST;
        }
    };

    std::unique_ptr<ASTNode> parseTreeToAST(parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertProg(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertBlock(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertDecl(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertStmt(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertLoop(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertCond(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertRetStmt(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertLogStmt(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertExpStmt(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertFunDecl(int node, parser::parseTree &tree);
    std::pair<std::unique_ptr<types::Type>, std::unique_ptr<ASTNode>> convertDeclParam(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertVarDecl(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertConstDecl(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertVal(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertAssn(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertLoc(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertExp(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertBinaryOp(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertBinaryOpHelper(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertUnaryOp(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertFunCall(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertUnAmb(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertAtom(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertElse(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertElifs(int node, parser::parseTree &tree, std::unique_ptr<ASTNode> finElse=nullptr);
    std::unique_ptr<types::Type> convertDeclType(int node, parser::parseTree &tree);
    std::unique_ptr<types::Type> convertAtomType(int node, parser::parseTree &tree);
    std::unique_ptr<types::Type> convertFunType(int node, parser::parseTree &tree);
    std::vector<std::unique_ptr<types::Type>> convertSigParams(int node, parser::parseTree &tree);
    std::unique_ptr<types::Type> convertType(int node, parser::parseTree &tree);
    std::unique_ptr<types::Type> convertCompType(int node, parser::parseTree &tree);
    std::unique_ptr<types::Type> convertArrType(int node, parser::parseTree &tree);
    std::unique_ptr<types::Type> convertPtrType(int node, parser::parseTree &tree);
    std::unique_ptr<types::Type> convertArrDeclType(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertPtrDeref(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertArrAcc(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertArr(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertPtr(int node, parser::parseTree &tree);
    std::unique_ptr<ASTNode> convertChain(int node, parser::parseTree &tree);

    void vizTree(const std::unique_ptr<ASTNode>& node, const std::string &prefix, bool isLast, bool postResolve = false);
}

#endif