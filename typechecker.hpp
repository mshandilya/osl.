#ifndef TYPEC_H
#define TYPEC_H

#include "ast.hpp"
#include "utils.hpp"

namespace codetree {

    enum UnaryOperation {
        RET_CTN,    //
        LOG_CTN,    //
        GET_CTN,    //
        ANEG_CTN,   //
        BNOT_CTN,   //
        LNOT_CTN,   //
        PTRREF_CTN, //
        PTRAT_CTN   //
    };

    enum BinaryOperation {
        LOOP_CTN,   //
        PCOND_CTN,  //
        SET_CTN,    //
        BIND_CTN,   //
        AADD_CTN,   //
        ASUB_CTN,   //
        AMUL_CTN,   //
        ADIV_CTN,   //
        AMOD_CTN,   //
        APOW_CTN,   //
        BOR_CTN,    //
        BAND_CTN,   //
        BXOR_CTN,   //
        BXAND_CTN,  //
        BLSHFT_CTN, //
        BRSHFT_CTN, //
        LOR_CTN,    //
        LAND_CTN,   //
        LXOR_CTN,   //
        LXAND_CTN,  //
        RNEQ_CTN,   //
        REQ_CTN,    //
        RGT_CTN,    //
        RLT_CTN,    //
        RGEQ_CTN,   //
        RLEQ_CTN,   //
        ARRGET_CTN, //
    };

    enum TernaryOperation {
        COND_CTN,   //
    };

    enum NnaryOperation {
        PROG_CTN,   //
        BLOCK_CTN,  //
        MKFUN_CTN,  //
        MKARR_CTN,  //
        CALL_CTN,   //
    };

    enum ValueType {
        NUM_CTN,    //
        CHAR_CTN,   //
        BOOL_CTN,   //
        NULL_CTN,   //
        IDEN_CTN,   //
    };

    class CodeTreeNode {
    public:
        std::unique_ptr<types::Type> resultingType;
        bool isLocation;

        CodeTreeNode() : resultingType(nullptr) {}

        CodeTreeNode(std::unique_ptr<types::Type>&& rt) : resultingType(std::move(rt)) {}

        void setResultingType(std::unique_ptr<types::Type>&& rt) {
            resultingType = std::move(rt);
        }
    };

    class UnaryCTN : public CodeTreeNode {
    public:
        UnaryOperation op;
        std::unique_ptr<CodeTreeNode> child;

        UnaryCTN(UnaryOperation op, std::unique_ptr<CodeTreeNode>&& child) : op(op), child(std::move(child)), CodeTreeNode(nullptr) {}
    };

    class BinaryCTN : public CodeTreeNode {
    public:
        BinaryOperation op; 
        std::unique_ptr<CodeTreeNode> lchild, rchild;

        BinaryCTN(BinaryOperation op, std::unique_ptr<CodeTreeNode>&& lc, std::unique_ptr<CodeTreeNode>&& rc) : op(op), lchild(std::move(lc)), rchild(std::move(rc)), CodeTreeNode(nullptr) {}
    };

    class TernaryCTN : public CodeTreeNode {
    public:
        TernaryOperation op; 
        std::unique_ptr<CodeTreeNode> lchild, mchild, rchild;

        TernaryCTN(TernaryOperation op, std::unique_ptr<CodeTreeNode>&& lc, std::unique_ptr<CodeTreeNode>&& mc, std::unique_ptr<CodeTreeNode>&& rc) : op(op), lchild(std::move(lc)), mchild(std::move(mc)), rchild(std::move(rc)), CodeTreeNode(nullptr) {}
    };

    class NnaryCTN : public CodeTreeNode {
    public:
        NnaryOperation op;
        std::vector<std::unique_ptr<CodeTreeNode>> children;

        NnaryCTN(NnaryOperation op, std::vector<std::unique_ptr<CodeTreeNode>>&& children) : op(op), children(std::move(children)), CodeTreeNode(nullptr) {}
    };

    class AtomCTN : public CodeTreeNode {
    public:
        ValueType val_t;
        std::unique_ptr<types::AtomicType> val;
    };

    class IdenCTN : public CodeTreeNode {
    public:
        ValueType val_t = IDEN_CTN;
    };

}

std::unique_ptr<types::Type> defaultRetType = nullptr;

class TypeChecker {
public:
    std::unique_ptr<codetree::CodeTreeNode> root;

    TypeChecker(std::unique_ptr<ast::ASTNode>&& root) {
        this->root = typecheckNext(std::move(root));
    }

    std::unique_ptr<codetree::CodeTreeNode> typecheckNext(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckProg(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckBlock(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckLoop(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckCond(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckReturn(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckLog(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckLetFun(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckLetArr(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckLetVar(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckLetConst(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckAssign(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckBinOp(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckUnOp(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckFunCall(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckArray(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckNum(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckChar(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckBool(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckNull(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckIden(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
    std::unique_ptr<codetree::CodeTreeNode> typecheckDeclType(std::unique_ptr<types::ArrayDeclType>& node); // can never have declarations
    std::unique_ptr<codetree::CodeTreeNode> typecheckDeclType(std::unique_ptr<types::Type>& node); // can never have declarations
};

#endif