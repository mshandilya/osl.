#include "typechecker.hpp"

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckNext(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
    switch(root->type()) {
        case ast::PROG_AST:
            return typecheckProg(std::move(root), expectedReturnType);
        case ast::BLOCK_AST:
            return typecheckBlock(std::move(root), expectedReturnType);
        case ast::LOOP_AST:
            return typecheckLoop(std::move(root), expectedReturnType);
        case ast::COND_AST:
            return typecheckCond(std::move(root), expectedReturnType);
        case ast::RET_AST:
            return typecheckReturn(std::move(root), expectedReturnType);
        case ast::LOG_AST:
            // log's value cannot have a return statement
            return typecheckLog(std::move(root));
        case ast::LETFUN_AST:
            // return statement after this would link to another function
            return typecheckLetFun(std::move(root));
        case ast::LETARR_AST:
            // let array won't have a return statement
            return typecheckLetArr(std::move(root));
        case ast::LETVAR_AST:
            // let var won't have a return statement
            return typecheckLetVar(std::move(root));
        case ast::LETCONST_AST:
            // let const won't have a return statement
            return typecheckLetConst(std::move(root));
        case ast::ASSIGN_AST:
            // let assign won't have a return statement
            return typecheckAssign(std::move(root));
        case ast::BOP_AST:
            return typecheckBinOp(std::move(root));
        case ast::UOP_AST:
            return typecheckUnOp(std::move(root));
        case ast::FUNCALL_AST:
            return typecheckFunCall(std::move(root));
        case ast::ARR_AST:
            return typecheckArray(std::move(root));
        case ast::NUM_AST:
            return typecheckNum(std::move(root));
        case ast::CHAR_AST:
            return typecheckChar(std::move(root));
        case ast::BOOL_AST:
            return typecheckBool(std::move(root));
        case ast::NULL_AST:
            return typecheckNull(std::move(root));
        case ast::IDEN_AST:
            return typecheckIden(std::move(root));
    }
}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckProg(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
    auto node = dynamic_cast<ast::Prog*>(root.get());
    std::vector<std::unique_ptr<codetree::CodeTreeNode>> children;
    for(auto&& decl : node->decls) {
        children.push_back(typecheckNext(std::move(decl), expectedReturnType));
    }
    return std::make_unique<codetree::NnaryCTN>(codetree::PROG_CTN, std::move(children));
}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckBlock(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
    auto node = dynamic_cast<ast::Block*>(root.get());
    std::vector<std::unique_ptr<codetree::CodeTreeNode>> children;
    for(auto&& decl : node->decls) {
        children.push_back(typecheckNext(std::move(decl), expectedReturnType));
    }
    return std::make_unique<codetree::NnaryCTN>(codetree::BLOCK_CTN, std::move(children));
}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckLoop(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
    auto node = dynamic_cast<ast::Loop*>(root.get());
    // condition can't have a return statement
    auto lc = typecheckNext(std::move(node->cond));
    if(lc->resultingType->name() == types::ATOM) {
        auto lcat = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
        if(lcat->atomicName() != types::BOOL or lcat->atomicName() != types::ANY) {
            std::cout << "code.osl: \033[31mTypeError\033[0m: Condition to a while loop must resolve to a `bool` type." << std::endl;
            exit(0);
        }
    }
    else {
        std::cout << "code.osl: \033[31mTypeError\033[0m: Condition to a while loop must resolve to a `bool` type." << std::endl;
        exit(0);
    }
    auto rc = typecheckNext(std::move(node->body), expectedReturnType);
    return std::make_unique<codetree::BinaryCTN>(codetree::LOOP_CTN, std::move(lc), std::move(rc));
}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckCond(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
    auto node = dynamic_cast<ast::Conditional*>(root.get());
    // condition can't have a return statement
    auto lc = typecheckNext(std::move(node->cond));
    if(lc->resultingType->name() == types::ATOM) {
        auto lcat = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
        if(lcat->atomicName() != types::BOOL or lcat->atomicName() != types::ANY) {
            std::cout << "code.osl: \033[31mTypeError\033[0m: Condition to an if statement must resolve to a `bool` type." << std::endl;
            exit(0);
        }
    }
    else {
        std::cout << "code.osl: \033[31mTypeError\033[0m: Condition to an if statement must resolve to a `bool` type." << std::endl;
        exit(0);
    }
    auto mc = typecheckNext(std::move(node->thenBody), expectedReturnType);
    std::unique_ptr<codetree::CodeTreeNode> rc;
    if(node->elseBody != nullptr)
        return std::make_unique<codetree::TernaryCTN>(codetree::COND_CTN, std::move(lc), std::move(mc), typecheckNext(std::move(node->elseBody), expectedReturnType));
    else
        return std::make_unique<codetree::BinaryCTN>(codetree::PCOND_CTN, std::move(lc), std::move(mc));
}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckReturn(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
    auto node = dynamic_cast<ast::Return*>(root.get());
    auto child = typecheckNext(std::move(node->val));
    
    // child's type must be that of the function's return type
    if(expectedReturnType != child->resultingType) {
        std::cout << "code.osl: \033[31mTypeError\033[0m: Return type of the function was expected to be something else." << std::endl;
        exit(0);
    }

    return std::make_unique<codetree::UnaryCTN>(codetree::RET_CTN, std::move(child));
}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckLog(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
    auto node = dynamic_cast<ast::Return*>(root.get());
    auto child = typecheckNext(std::move(node->val));
    
    // child's type must be that of the function's return type
    if(child->resultingType->name() == types::ATOM) {
        std::cout << "code.osl: \033[31mTypeError\033[0m: Only atomic types are allowed to be logged." << std::endl;
        exit(0);
    }

    return std::make_unique<codetree::UnaryCTN>(codetree::LOG_CTN, std::move(child));
}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckLetFun(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
    
}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckLetArr(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {

}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckLetVar(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {

}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckLetConst(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {

}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckAssign(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {

}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckBinOp(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {

}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckUnOp(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {

}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckFunCall(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {

}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckArray(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {

}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckNum(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {

}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckChar(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {

}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckBool(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {

}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckNull(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {

}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckIden(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {

}

std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckDeclType(std::unique_ptr<types::ArrayDeclType>& node); // can never have declarations
std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckDeclType(std::unique_ptr<types::Type>& node); // can never have declarations