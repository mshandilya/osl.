#ifndef TYPEC_H
#define TYPEC_H

#include "utils.hpp"
#include "ast.hpp"

namespace codetree {

    enum UnaryOperation {
        RET_CTN,    //      // Done
        LOG_CTN,    //      // Done
        GET_CTN,    //
        ANEG_CTN,   //      // Done
        BNOT_CTN,   //      // Done
        LNOT_CTN,   //      // Done
        PTRREF_CTN, //      // Done
        PTRAT_CTN,   //     // Done
        UTS_CTN,    //
        STU_CTN,    //
        ITF_CTN,    //
        FTI_CTN,    //
        UTF_CTN,    //
        FTU_CTN,    //
    };

    enum BinaryOperation {
        LOOP_CTN,   //      // Done
        PCOND_CTN,  //      // Done
        DMKARR_CTN, //      // Done
        SET_CTN,    //
        SETASS_CTN, //      // Done
        BIND_CTN,   //
        APAD_CTN,   //
        LPAD_CTN,   //      
        AADD_CTN,   //      // Done
        ASUB_CTN,   //      // Done
        AMUL_CTN,   //      // Done
        ADIV_CTN,   //      // Done
        AMOD_CTN,   //      // Done
        APOW_CTN,   //      // Done
        BOR_CTN,    //      // Done
        BAND_CTN,   //      // Done
        BXOR_CTN,   //      // Done
        BXAND_CTN,  //      // Done
        BLSHFT_CTN, //      // Done
        BRSHFT_CTN, //      // Done
        LOR_CTN,    //      // Done
        LAND_CTN,   //      // Done
        LXOR_CTN,   //      // Done
        LXAND_CTN,  //      // Done
        RNEQ_CTN,   //      // Done
        REQ_CTN,    //      // Done
        RGT_CTN,    //      // Done
        RLT_CTN,    //      // Done
        RGEQ_CTN,   //      // Done
        RLEQ_CTN,   //      // Done
        ARRGET_CTN, //      // Done
    };

    enum TernaryOperation {
        COND_CTN,   //      // Done
    };

    enum NnaryOperation {
        PROG_CTN,   //      // Done
        BLOCK_CTN,  //      // Done
        MKFUN_CTN,  //      // Done
        MKARR_CTN,  //      // Done
        CALL_CTN,   //      // Done
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

        CodeTreeNode() : resultingType(nullptr), isLocation(false) {}

        CodeTreeNode(std::unique_ptr<types::Type>&& rt) : resultingType(std::move(rt)), isLocation(false) {}

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

        AtomCTN(ValueType val_t, std::unique_ptr<types::AtomicType>&& value) : val_t(val_t), val(std::move(value)) {}
    };

    class IdenCTN : public CodeTreeNode {
    public:
        ValueType val_t = IDEN_CTN;
        unsigned int id, scopeId;
        ast::Access ac;

        IdenCTN(unsigned int id, unsigned int scopeId, ast::Access ac) : id(id), scopeId(scopeId), ac(ac) {}
    };

}

namespace typecheck {

    extern std::unique_ptr<types::Type> defaultRetType;

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
        std::vector<std::unique_ptr<codetree::CodeTreeNode>> typecheckLetArr(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType = defaultRetType);
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
        std::unique_ptr<codetree::CodeTreeNode> typecheckDeclType(std::unique_ptr<types::ArrayDeclType>&& node); // can never have declarations
        std::unique_ptr<codetree::CodeTreeNode> typecheckDeclType(std::unique_ptr<types::Type>&& node); // can never have declarations
    };

    // The following is a casting utility that allows implicit casting of atomic types
    types::AtomicType castCopy(types::AtomicType& from, const types::AtomicType& to);

}

#endif