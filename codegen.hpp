#ifndef CGEN_H
#define CGEN_H

#include "utils.hpp"
#include "ast.hpp"

enum OPCODES {
    PUSH,
    POP,
    DUP,
    SWAP,
    OVER,

    ATH_ADD,
    ATH_SUB,
    ATH_MUL,
    ATH_DIV,
    ATH_MOD,
    ATH_POW, 
    ATH_NEG,

    BW_OR,
    BW_AND,
    BW_XOR,
    BW_XAND,
    BW_LSHFT,
    BW_RSHFT,
    BW_NOT,

    LOG_OR,
    LOG_AND,
    LOG_XOR,
    LOG_XAND,
    LOG_NOT,

    REL_NEQ,
    REL_EQ,
    REL_GT,
    REL_LT,
    REL_GEQ,
    REL_LEQ,

    ARR_GET,
    PTR_REF,
    PTR_AT
};

class CodeGenerator {
    std::vector<unsigned char> instructions;
    std::unique_ptr<ast::ASTNode>& root;

    void codifyNext(std::unique_ptr<ast::ASTNode>& node);
    void codifyProg(std::unique_ptr<ast::ASTNode>& node);
    void codifyBlock(std::unique_ptr<ast::ASTNode>& node);
    void codifyLoop(std::unique_ptr<ast::ASTNode>& node);
    void codifyCond(std::unique_ptr<ast::ASTNode>& node);
    void codifyReturn(std::unique_ptr<ast::ASTNode>& node);
    void codifyLog(std::unique_ptr<ast::ASTNode>& node);
    void codifyLetFun(std::unique_ptr<ast::ASTNode>& node);
    void codifyLetVar(std::unique_ptr<ast::ASTNode>& node);
    void codifyLetConst(std::unique_ptr<ast::ASTNode>& node);
    void codifyValue(std::unique_ptr<ast::ASTNode>& node);
    void codifyAssign(std::unique_ptr<ast::ASTNode>& node);
    void codifyLocation(std::unique_ptr<ast::ASTNode>& node);
    void codifyBinOp(std::unique_ptr<ast::ASTNode>& node);
    void codifyUnOp(std::unique_ptr<ast::ASTNode>& node);
    void codifyFunCall(std::unique_ptr<ast::ASTNode>& node);
    void codifyArray(std::unique_ptr<ast::ASTNode>& node);
    void codifyNumber(std::unique_ptr<ast::ASTNode>& node);
    void codifyChar(std::unique_ptr<ast::ASTNode>& node);
    void codifyBool(std::unique_ptr<ast::ASTNode>& node);
    void codifyNull(std::unique_ptr<ast::ASTNode>& node);
    void codifyIden(std::unique_ptr<ast::ASTNode>& node);

public:
    CodeGenerator(std::unique_ptr<ast::ASTNode>& root) : root(root) {}

    inline void generate() {
        codifyNext(root);
    }

    void dumpBinary(const std::string& fileName) {
        // unimplemented
    }
};

#endif