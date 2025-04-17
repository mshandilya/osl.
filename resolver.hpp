#ifndef RES_H
#define RES_H

#include "utils.hpp"
#include "ast.hpp"

class Resolver {
    int currentId, currentScope;
    ast::ASTNode root;

    std::vector<std::shared_ptr<ast::Identifier>> identifiers;
    std::unordered_map<std::string, std::vector<std::shared_ptr<ast::Identifier>>> identifierIds;

    void resolveNext(std::unique_ptr<ast::ASTNode>& node);
    void resolveProg(std::unique_ptr<ast::Prog>& node);
    void resolveLet(std::unique_ptr<ast::Let>& node);
    void resolveLetFun(std::unique_ptr<ast::LetFun>& node);
    void resolveIf(std::unique_ptr<ast::If>& node);
    void resolveLog(std::unique_ptr<ast::Log>& node);
    void resolveReturn(std::unique_ptr<ast::Return>& node);
    void resolveAssign(std::unique_ptr<ast::Assign>& node);
    void resolveBinOp(std::unique_ptr<ast::BinaryOperator>& node);
    void resolveUnOp(std::unique_ptr<ast::UnaryOperator>& node);
    void resolveIden(std::unique_ptr<ast::Identifier>& node);
};

#endif