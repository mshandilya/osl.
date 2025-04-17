#include "resolver.hpp"

void Resolver::resolveNext(std::unique_ptr<ast::ASTNode>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    switch(node->type()) {
        case ast::NUM_AST:
        case ast::BOOL_AST:
        case ast::CHAR_AST:
        case ast::NULL_AST:
        // all of these are atomic ASTs
            return;
            break;
        case ast::BOP_AST:
            // this must be the value of the declaration, therefore, it is not part of the declaration
            return resolveBinOp(dynamic_cast<std::unique_ptr<ast::BinaryOperator>&>(*node));
        case ast::UOP_AST:
            // this must be the value of the declaration, therefore, it is not part of the declaration
            return resolveUnOp(dynamic_cast<std::unique_ptr<ast::UnaryOperator>&>(*node));
        case ast::PROG_AST:
            return resolveProg(dynamic_cast<std::unique_ptr<ast::Prog>&>(*node), isDecl, dt, ac);
        case ast::LET_AST:
            return resolveLet(dynamic_cast<std::unique_ptr<ast::Let>&>(*node), isDecl, dt, ac);
        case ast::ASSIGN_AST:
            return resolveAssign(dynamic_cast<std::unique_ptr<ast::Assign>&>(*node), isDecl, dt, ac);
        case ast::IDEN_AST:
            return resolveIden(dynamic_cast<std::unique_ptr<ast::Identifier>&>(*node), isDecl, dt, ac);
        case ast::IF_AST:
            return resolveIf(dynamic_cast<std::unique_ptr<ast::If>&>(*node), isDecl, dt, ac);
        case ast::LOG_AST:
            return resolveLog(dynamic_cast<std::unique_ptr<ast::Log>&>(*node), isDecl, dt, ac);
        case ast::RET_AST:
            return resolveReturn(dynamic_cast<std::unique_ptr<ast::Return>&>(*node), isDecl, dt, ac);
        case ast::LETFUN_AST:
            return resolveLetFun(dynamic_cast<std::unique_ptr<ast::LetFun>&>(*node), isDecl, dt, ac);
    }
}

void Resolver::resolveProg(std::unique_ptr<ast::Prog>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: A block or program cannot be a declaration.");
    }
    for(auto& child : node->decls) {
        resolveNext(child);
    }
}

void Resolver::resolveLet(std::unique_ptr<ast::Let>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Declaration may not be inside another declaration.");
    }
    // these variables are newly declared, first the value is declared and then the identifier is declared.
    resolveNext(node->val, true, node->typ, node->acc);
    resolveNext(node->var, true, node->typ, node->acc);
}

void Resolver::resolveLetFun(std::unique_ptr<ast::LetFun>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Function Declaration may not be inside another declaration.");
    }
    resolveNext(node->name, true, types::FUNCTION, ast::CONST);
    // scope enhances
    currentScope++;
    for(auto& param : node->params) {
        resolveNext(param.second, true, param.first);
    }
    // no declaration in body
    resolveNext(node->body);
    while(!identifiers.empty() and identifiers.back().get()->scopeId == currentScope) {
        identifiers.pop_back();
    }
    // scope retreats
    currentScope--;
}

void Resolver::resolveIf(std::unique_ptr<ast::If>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Declaration may not be inside an if statement.");
    }
    // must not be a declaration
    resolveNext(node->cond);
    // scope enhances
    currentScope++;
    resolveNext(node->thenBody);
    while(!identifiers.empty() and identifiers.back().get()->scopeId == currentScope) {
        identifiers.pop_back();
    }
    if(node->elseBody) {
        // else body is present
        resolveNext(node->elseBody);
        while(!identifiers.empty() and identifiers.back().get()->scopeId == currentScope) {
            identifiers.pop_back();
        }
    }
    // scope retreats
    currentScope--;
}

void Resolver::resolveLog(std::unique_ptr<ast::Log>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Declaration may not be inside a log statement.");
    }
    resolveNext(node->val);
}

void Resolver::resolveReturn(std::unique_ptr<ast::Return>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Declaration may not be inside a return statement.");
    }
    resolveNext(node->val);
}

void Resolver::resolveAssign(std::unique_ptr<ast::Assign>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    resolveNext(node->val, isDecl, dt, ac);
    resolveNext(node->var, isDecl, dt, ac);
}

void Resolver::resolveBinOp(std::unique_ptr<ast::BinaryOperator>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    // must not be a declaration (ensured in resolveNext)
    resolveNext(node->leftChild);
    resolveNext(node->rightChild);
}

void Resolver::resolveUnOp(std::unique_ptr<ast::UnaryOperator>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    // must not be a declaration (ensured in resolveNext)
    resolveNext(node->child);
}

void Resolver::resolveIden(std::unique_ptr<ast::Identifier>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    auto iden = identifierIds.find(node->idenName);
    if(isDecl) {
        if(iden != identifierIds.end() and iden->second.back().get()->scopeId == currentScope) {
            // identifier is present
            if(ac == ast::VAR)
                std::cout << "code.osl: \033[31mResolutionError\033[0m: Variable `" << node->idenName << "` has been declared again in the same scope." << std::endl;
            else
                std::cout << "code.osl: \033[31mResolutionError\033[0m: Constant `" << node->idenName << "` has been declared again in the same scope." << std::endl;
            exit(0);
        }
        else if(iden == identifierIds.end()) {
            identifierIds[node->idenName] = std::vector<std::reference_wrapper<std::unique_ptr<ast::Identifier>>>();
            node->bind(dt, ac, currentId++, currentScope);
            identifierIds[node->idenName].emplace_back(node);
            identifiers.emplace_back(node);
        }
        else {
            node->bind(dt, ac, currentId++, currentScope);
            identifierIds[node->idenName].emplace_back(node);
            identifiers.emplace_back(node);
        }
    }
    else {
        if(iden != identifierIds.end() and !iden->second.empty()) {
            node->bind(iden->second.back().get()->boundDataType, iden->second.back().get()->access, iden->second.back().get()->id, iden->second.back().get()->scopeId);
        }
        else {
            // identifier is not present
            std::cout << "code.osl: \033[31mResolutionError\033[0m: Identifier `" << node->idenName << "` cannot be resolved (found) in the current scope." << std::endl;
            exit(0);
        }
    }
}