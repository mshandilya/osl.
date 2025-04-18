#ifndef RES_H
#define RES_H

#include "utils.hpp"
#include "ast.hpp"

#define LOG(x) std::cout<<x<<std::endl;

class Resolver {
public:
    int currentId, currentScope;
    std::unique_ptr<ast::ASTNode>& root;

    std::vector<ast::Identifier*> identifiers;
    std::unordered_map<std::string, std::vector<ast::Identifier*>> identifierIds;

    Resolver(std::unique_ptr<ast::ASTNode>& root) : root(root), currentId(0), currentScope(0) {
        LOG("inside constructor")
        resolveNext(root);
        LOG("escaped resolve next")
    }

    void resolveNext(std::unique_ptr<ast::ASTNode>& node, bool isDecl = false, types::TYPES dt = types::UNRESOLVED, ast::Access ac = ast::VAR);
    void resolveProg(std::unique_ptr<ast::ASTNode>& node, bool isDecl = false, types::TYPES dt = types::UNRESOLVED, ast::Access ac = ast::VAR);
    void resolveLet(std::unique_ptr<ast::ASTNode>& node, bool isDecl = false, types::TYPES dt = types::UNRESOLVED, ast::Access ac = ast::VAR);
    void resolveLetFun(std::unique_ptr<ast::ASTNode>& node, bool isDecl = false, types::TYPES dt = types::UNRESOLVED, ast::Access ac = ast::VAR);
    void resolveIf(std::unique_ptr<ast::ASTNode>& node, bool isDecl = false, types::TYPES dt = types::UNRESOLVED, ast::Access ac = ast::VAR);
    void resolveLog(std::unique_ptr<ast::ASTNode>& node, bool isDecl = false, types::TYPES dt = types::UNRESOLVED, ast::Access ac = ast::VAR);
    void resolveReturn(std::unique_ptr<ast::ASTNode>& node, bool isDecl = false, types::TYPES dt = types::UNRESOLVED, ast::Access ac = ast::VAR);
    void resolveAssign(std::unique_ptr<ast::ASTNode>& node, bool isDecl = false, types::TYPES dt = types::UNRESOLVED, ast::Access ac = ast::VAR);
    void resolveBinOp(std::unique_ptr<ast::ASTNode>& node, bool isDecl = false, types::TYPES dt = types::UNRESOLVED, ast::Access ac = ast::VAR);
    void resolveUnOp(std::unique_ptr<ast::ASTNode>& node, bool isDecl = false, types::TYPES dt = types::UNRESOLVED, ast::Access ac = ast::VAR);
    void resolveIden(std::unique_ptr<ast::ASTNode>& node, bool isDecl = false, types::TYPES dt = types::UNRESOLVED, ast::Access ac = ast::VAR);
};

#endif