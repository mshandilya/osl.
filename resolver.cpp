#include "resolver.hpp"

#define LOG(x) std::cout<<x<<std::endl;

void Resolver::resolveNext(std::unique_ptr<ast::ASTNode>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    LOG("inside resolve next")
    LOG(node->type())
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
            return resolveBinOp(node);
        case ast::UOP_AST:
            // this must be the value of the declaration, therefore, it is not part of the declaration
            return resolveUnOp(node);
        case ast::PROG_AST:
            return resolveProg(node, isDecl, dt, ac);
        case ast::LET_AST:
            return resolveLet(node, isDecl, dt, ac);
        case ast::ASSIGN_AST:
            return resolveAssign(node, isDecl, dt, ac);
        case ast::IDEN_AST:
            return resolveIden(node, isDecl, dt, ac);
        case ast::IF_AST:
            return resolveIf(node, isDecl, dt, ac);
        case ast::LOG_AST:
            return resolveLog(node, isDecl, dt, ac);
        case ast::RET_AST:
            return resolveReturn(node, isDecl, dt, ac);
        case ast::LETFUN_AST:
            return resolveLetFun(node, isDecl, dt, ac);
    }
}

void Resolver::resolveProg(std::unique_ptr<ast::ASTNode>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    LOG("inside resolve prog")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: A block or program cannot be a declaration.");
    }
    auto nnode = dynamic_cast<ast::Prog*>(node.get());
    for(auto& child : nnode->decls) {
        resolveNext(child);
    }
}

void Resolver::resolveLet(std::unique_ptr<ast::ASTNode>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    LOG("inside resolve let")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Declaration may not be inside another declaration.");
    }
    // these variables are newly declared, first the value is declared and then the identifier is declared.
    auto nnode = dynamic_cast<ast::Let*>(node.get());
    resolveNext(nnode->val, true, nnode->typ, nnode->acc);
    resolveNext(nnode->var, true, nnode->typ, nnode->acc);
}

void Resolver::resolveLetFun(std::unique_ptr<ast::ASTNode>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    LOG("inside resolve let fun")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Function Declaration may not be inside another declaration.");
    }
    auto nnode = dynamic_cast<ast::LetFun*>(node.get());
    resolveNext(nnode->name, true, types::FUNCTION, ast::CONST);
    // scope enhances
    currentScope++;
    for(auto& param : nnode->params) {
        resolveNext(param.second, true, param.first);
    }
    // no declaration in body
    resolveNext(nnode->body);
    while(!identifiers.empty() and ((identifiers.back()))->scopeId == currentScope) {
        identifiers.pop_back();
    }
    // scope retreats
    currentScope--;
}

void Resolver::resolveIf(std::unique_ptr<ast::ASTNode>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    LOG("inside resolve if")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Declaration may not be inside an if statement.");
    }
    // must not be a declaration
    auto nnode = dynamic_cast<ast::If*>(node.get());
    resolveNext(nnode->cond);
    // scope enhances
    currentScope++;
    resolveNext(nnode->thenBody);
    while(!identifiers.empty() and ((identifiers.back()))->scopeId == currentScope) {
        identifiers.pop_back();
    }
    if(nnode->elseBody) {
        // else body is present
        resolveNext(nnode->elseBody);
        while(!identifiers.empty() and ((identifiers.back()))->scopeId == currentScope) {
            identifiers.pop_back();
        }
    }
    // scope retreats
    currentScope--;
}

void Resolver::resolveLog(std::unique_ptr<ast::ASTNode>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    LOG("inside resolve log")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Declaration may not be inside a log statement.");
    }
    auto nnode = dynamic_cast<ast::Log*>(node.get());
    resolveNext(nnode->val);
}

void Resolver::resolveReturn(std::unique_ptr<ast::ASTNode>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    LOG("inside resolve return")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Declaration may not be inside a return statement.");
    }
    auto nnode = dynamic_cast<ast::Return*>(node.get());
    resolveNext(nnode->val);
}

void Resolver::resolveAssign(std::unique_ptr<ast::ASTNode>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    LOG("inside resolve assign")
    auto nnode = dynamic_cast<ast::Assign*>(node.get());
    resolveNext(nnode->val, isDecl, dt, ac);
    resolveNext(nnode->var, isDecl, dt, ac);
}

void Resolver::resolveBinOp(std::unique_ptr<ast::ASTNode>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    LOG("inside resolve bin op")
    // must not be a declaration (ensured in resolveNext)
    auto nnode = dynamic_cast<ast::BinaryOperator*>(node.get());
    resolveNext(nnode->leftChild);
    resolveNext(nnode->rightChild);
}

void Resolver::resolveUnOp(std::unique_ptr<ast::ASTNode>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    LOG("inside resolve un op")
    // must not be a declaration (ensured in resolveNext)
    auto nnode = dynamic_cast<ast::UnaryOperator*>(node.get());
    resolveNext(nnode->child);
}

void Resolver::resolveIden(std::unique_ptr<ast::ASTNode>& node, bool isDecl, types::TYPES dt, ast::Access ac) {
    LOG("inside resolve iden")
    auto nnode = dynamic_cast<ast::Identifier*>(node.get());
    auto iden = identifierIds.find(nnode->idenName);
    if(isDecl) {
        if(iden != identifierIds.end() and ((iden->second.back()))->scopeId == currentScope) {
            // identifier is present
            if(ac == ast::VAR)
                std::cout << "code.osl: \033[31mResolutionError\033[0m: Variable `" << nnode->idenName << "` has been declared again in the same scope." << std::endl;
            else
                std::cout << "code.osl: \033[31mResolutionError\033[0m: Constant `" << nnode->idenName << "` has been declared again in the same scope." << std::endl;
            exit(0);
        }
        else if(iden == identifierIds.end()) {
            identifierIds[nnode->idenName] = std::vector<ast::Identifier*>();
            nnode->bind(dt, ac, currentId++, currentScope);
            identifierIds[nnode->idenName].push_back(nnode);
            identifiers.push_back(nnode);
        }
        else {
            nnode->bind(dt, ac, currentId++, currentScope);
            identifierIds[nnode->idenName].push_back(nnode);
            identifiers.push_back(nnode);
        }
    }
    else {
        if(iden != identifierIds.end() and !iden->second.empty()) {
            nnode->bind(((iden->second.back()))->boundDataType, ((iden->second.back()))->access, ((iden->second.back()))->id, ((iden->second.back()))->scopeId);
        }
        else {
            // identifier is not present
            std::cout << "code.osl: \033[31mResolutionError\033[0m: Identifier `" << nnode->idenName << "` cannot be resolved (found) in the current scope." << std::endl;
            exit(0);
        }
    }
}