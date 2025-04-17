#include "resolver.hpp"

void Resolver::resolveNext(std::unique_ptr<ast::ASTNode>& node) {
    switch(node->type()) {
        case ast::NUM_AST:
        case ast::BOOL_AST:
        case ast::CHAR_AST:
        case ast::NULL_AST:
        // all of these are atomic ASTs
            return;
            break;
        case ast::BOP_AST:
            return resolveBinOp(dynamic_cast<std::unique_ptr<ast::BinaryOperator>&>(*node));
        case ast::UOP_AST:
            return resolveUnOp(dynamic_cast<std::unique_ptr<ast::UnaryOperator>&>(*node));
        case ast::PROG_AST:
            return resolveProg(dynamic_cast<std::unique_ptr<ast::Prog>&>(*node));
        case ast::LET_AST:
            return resolveLet(dynamic_cast<std::unique_ptr<ast::Let>&>(*node));
        case ast::ASSIGN_AST:
            return resolveAssign(dynamic_cast<std::unique_ptr<ast::Assign>&>(*node));
        case ast::IDEN_AST:
            return resolveIden(dynamic_cast<std::unique_ptr<ast::Identifier>&>(*node));
        case ast::IF_AST:
            return resolveIf(dynamic_cast<std::unique_ptr<ast::If>&>(*node));
        case ast::LOG_AST:
            return resolveLog(dynamic_cast<std::unique_ptr<ast::Log>&>(*node));
        case ast::RET_AST:
            return resolveReturn(dynamic_cast<std::unique_ptr<ast::Return>&>(*node));
        case ast::LETFUN_AST:
            return resolveLetFun(dynamic_cast<std::unique_ptr<ast::LetFun>&>(*node));
    }
}

void Resolver::resolveProg(std::unique_ptr<ast::Prog>& node) {
    for(auto& child : node->decls) {

    }
}