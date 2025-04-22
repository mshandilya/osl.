#include "resolver.hpp"

int count = 0;

void Resolver::resolveNext(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve next")
    LOG(node->type())
    switch(node->type()) {
        case ast::PROG_AST:
            return resolveProg(node, isDecl, std::move(dt), ac);
        case ast::BLOCK_AST:
            return resolveBlock(node, isDecl, std::move(dt), ac);
        case ast::LOOP_AST:
            return resolveLoop(node, isDecl, std::move(dt), ac);
        case ast::COND_AST:
            return resolveCond(node, isDecl, std::move(dt), ac);
        case ast::RET_AST:
            return resolveReturn(node, isDecl, std::move(dt), ac);
        case ast::LOG_AST:
            return resolveLog(node, isDecl, std::move(dt), ac);
        case ast::LETFUN_AST:
            return resolveLetFun(node, isDecl, std::move(dt), ac);
        case ast::LETARR_AST:
            return resolveLetArr(node, isDecl, std::move(dt), ac);
        case ast::LETVAR_AST:
            return resolveLetVar(node, isDecl, std::move(dt), ac);
        case ast::LETCONST_AST:
            return resolveLetConst(node, isDecl, std::move(dt), ac);
        case ast::ASSIGN_AST:
            return resolveAssign(node, isDecl, std::move(dt), ac);
        case ast::BOP_AST:
            return resolveBinOp(node, isDecl, std::move(dt), ac);
        case ast::UOP_AST:
            return resolveUnOp(node, isDecl, std::move(dt), ac);
        case ast::FUNCALL_AST:
            return resolveFunCall(node, isDecl, std::move(dt), ac);
        case ast::ARR_AST:
            return resolveArray(node, isDecl, std::move(dt), ac);
        case ast::NUM_AST:
        case ast::CHAR_AST:
        case ast::BOOL_AST:
        case ast::NULL_AST:
            // nothing to do for atomic AST nodes
            return;
        case ast::IDEN_AST:
            return resolveIden(node, isDecl, std::move(dt), ac);
    }
}

void Resolver::resolveProg(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve prog")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: A block or program cannot be a declaration.");
    }
    auto nnode = dynamic_cast<ast::Prog*>(node.get());
    for(auto& child : nnode->decls) {
        resolveNext(child);
    }
}

void Resolver::resolveBlock(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve block")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: A block or program cannot be a declaration.");
    }
    // scope enhances
    currentScope++;
    auto nnode = dynamic_cast<ast::Block*>(node.get());
    for(auto& child : nnode->decls) {
        resolveNext(child);
    }
    while(!identifiers.empty() and ((identifiers.back()))->scopeId == currentScope) {
        identifierIds[identifiers.back()->idenName].pop_back();
        identifiers.pop_back();
    }
    // scope retreats
    currentScope--;
}

void Resolver::resolveLoop(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve if")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Declaration may not be inside an if statement.");
    }
    // must not be a declaration
    auto nnode = dynamic_cast<ast::Loop*>(node.get());
    resolveNext(nnode->cond);
    // scope enhances
    currentScope++;
    resolveNext(nnode->body);
    while(!identifiers.empty() and ((identifiers.back()))->scopeId == currentScope) {
        identifierIds[identifiers.back()->idenName].pop_back();
        identifiers.pop_back();
    }
    // scope retreats
    currentScope--;
}

void Resolver::resolveCond(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve if")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Declaration may not be inside an if statement.");
    }
    // must not be a declaration
    auto nnode = dynamic_cast<ast::Conditional*>(node.get());
    resolveNext(nnode->cond);
    // scope enhances
    currentScope++;
    resolveNext(nnode->thenBody);
    while(!identifiers.empty() and ((identifiers.back()))->scopeId == currentScope) {
        identifierIds[identifiers.back()->idenName].pop_back();
        identifiers.pop_back();
    }
    if(nnode->elseBody) {
        // else body is present
        resolveNext(nnode->elseBody);
        while(!identifiers.empty() and ((identifiers.back()))->scopeId == currentScope) {
            identifierIds[identifiers.back()->idenName].pop_back();
            identifiers.pop_back();
        }
    }
    // scope retreats
    currentScope--;
}

void Resolver::resolveReturn(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve return")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Declaration may not be inside a return statement.");
    }
    auto nnode = dynamic_cast<ast::Return*>(node.get());
    resolveNext(nnode->val);
}

void Resolver::resolveLog(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve log")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Declaration may not be inside a log statement.");
    }
    auto nnode = dynamic_cast<ast::Log*>(node.get());
    resolveNext(nnode->val);
}

void Resolver::resolveLetFun(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve let fun")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Function Declaration may not be inside another declaration.");
    }
    auto nnode = dynamic_cast<ast::LetFun*>(node.get());
    auto rt = types::gtc(*(nnode->retType));
    LOG("return type constructed")
    std::vector<std::unique_ptr<types::Type>> pts;
    for(auto& param : nnode->params) {
        LOG("about to copy a param type")
        pts.push_back(types::gtc(*(param.first)));
        LOG("copied the param")
    }
    LOG("parameters pushed")
    std::unique_ptr<types::Type> ft = std::make_unique<types::FunctionType>(std::move(rt), std::move(pts));
    LOG("function type constructed")
    resolveNext(nnode->name, true, std::move(ft), ast::CONST);
    // scope enhances
    LOG("about to resolve the body")
    currentScope++;
    for(auto& param : nnode->params) {
        RLOG("param passed")
        resolveNext(param.second, true, types::gtc(*(param.first)), ast::VAR);
    }
    // no declaration in body
    resolveNext(nnode->body);
    while(!identifiers.empty() and ((identifiers.back()))->scopeId == currentScope) {
        RLOG("param popped")
        identifierIds[identifiers.back()->idenName].pop_back();
        identifiers.pop_back();
    }
    // scope retreats
    currentScope--;
}

void Resolver::resolveLetArr(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve let arr")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Function Declaration may not be inside another declaration.");
    }
    // first resolve the type
    auto nnode = dynamic_cast<ast::LetArray*>(node.get());
    resolveDeclType(nnode->myType);
    // then resolve the val (without declaration)
    resolveNext(nnode->val);
    // finally resolve the name with declaration (at this point copy arraydecltype)
    resolveNext(nnode->name, true, types::gtc(*(nnode->myType)), nnode->access);
}

void Resolver::resolveLetVar(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve let var")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Declaration may not be inside another declaration.");
    }
    // these variables are newly declared, first the value is resolved and then the identifier is declared.
    auto nnode = dynamic_cast<ast::LetVar*>(node.get());
    if(nnode->val->type() == ast::ASSIGN_AST)
        resolveNext(nnode->val, true, types::gtc(*(nnode->myType)), ast::VAR);
    else
        resolveNext(nnode->val);
    resolveNext(nnode->var, true, types::gtc(*(nnode->myType)), ast::VAR);
}

void Resolver::resolveLetConst(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve let const")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Declaration may not be inside another declaration.");
    }
    // these variables are newly declared, first the value is resolved and then the identifier is declared.
    auto nnode = dynamic_cast<ast::LetConst*>(node.get());
    if(nnode->val->type() == ast::ASSIGN_AST)
        resolveNext(nnode->val, true, types::gtc(*(nnode->myType)), ast::CONST);
    else
        resolveNext(nnode->val);
    resolveNext(nnode->var, true, types::gtc(*(nnode->myType)), ast::CONST);    
}

void Resolver::resolveAssign(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve assign")
    auto nnode = dynamic_cast<ast::Assign*>(node.get());
    if(isDecl) {
        auto dt2 = types::gtc(*dt);
        resolveNext(nnode->val, isDecl, std::move(dt), ac);
        resolveNext(nnode->var, isDecl, std::move(dt2), ac);
    }
    else {
        resolveNext(nnode->val);
        resolveNext(nnode->var);
    }
}

void Resolver::resolveBinOp(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve bin op")
    auto nnode = dynamic_cast<ast::BinaryOperator*>(node.get());
    if(isDecl) {
        auto dt2 = types::gtc(*dt);
        resolveNext(nnode->leftChild, isDecl, std::move(dt), ac);
        resolveNext(nnode->rightChild, isDecl, std::move(dt2), ac);
    }
    else {
        resolveNext(nnode->leftChild);
        resolveNext(nnode->rightChild);
    }
}

void Resolver::resolveUnOp(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve un op")
    auto nnode = dynamic_cast<ast::UnaryOperator*>(node.get());
    resolveNext(nnode->child, isDecl, std::move(dt), ac);
}

void Resolver::resolveFunCall(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve fun call")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Function Calls may not be inside a declaration.");
    }
    auto nnode = dynamic_cast<ast::FunctionCall*>(node.get());
    for(auto& param : nnode->params)
        resolveNext(param);
    resolveNext(nnode->name);
}

void Resolver::resolveArray(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve array")
    if(isDecl) {
        throw std::logic_error("Illegal Declaration: Array Value cannot be inside a declared.");
    }
    auto nnode = dynamic_cast<ast::ArrValue*>(node.get());
    for(auto& elem : nnode->elems)
        resolveNext(elem);
}

void Resolver::resolveIden(std::unique_ptr<ast::ASTNode>& node, bool isDecl, std::unique_ptr<types::Type>&& dt, ast::Access ac) {
    RLOG("inside resolve iden")
    auto nnode = dynamic_cast<ast::Identifier*>(node.get());
    auto iden = identifierIds.find(nnode->idenName);
    if(isDecl) {
        if(iden != identifierIds.end() and !(iden->second.empty()) and ((iden->second.back()))->scopeId == currentScope) {
            // identifier is present
            if(ac == ast::VAR)
                std::cout << "code.osl: \033[31mResolutionError\033[0m: Variable `" << nnode->idenName << "` has been declared again in the same scope." << std::endl;
            else
                std::cout << "code.osl: \033[31mResolutionError\033[0m: Constant `" << nnode->idenName << "` has been declared again in the same scope." << std::endl;
            exit(0);
        }
        else if(iden == identifierIds.end()) {
            identifierIds[nnode->idenName] = std::vector<ast::Identifier*>();
            nnode->bind(std::move(dt), ac, currentId++, currentScope);
            identifierIds[nnode->idenName].push_back(nnode);
            identifiers.push_back(nnode);
        }
        else {
            nnode->bind(std::move(dt), ac, currentId++, currentScope);
            identifierIds[nnode->idenName].push_back(nnode);
            identifiers.push_back(nnode);
        }
    }
    else {
        if(iden != identifierIds.end() and !iden->second.empty()) {
            nnode->bind(types::gtc(*(iden->second.back()->boundDataType)), ((iden->second.back()))->access, ((iden->second.back()))->id, ((iden->second.back()))->scopeId);
        }
        else {
            // identifier is not present
            std::cout << "code.osl: \033[31mResolutionError\033[0m: Identifier `" << nnode->idenName << "` cannot be resolved (found) in the current scope." << std::endl;
            exit(0);
        }
    }
}

void Resolver::resolveDeclType(std::unique_ptr<types::ArrayDeclType>& node) {
    // the size must be resolved first
    resolveNext(node->size);
    resolveDeclType(node->underlyingType);
}

void Resolver::resolveDeclType(std::unique_ptr<types::Type>& node) {
    // the size must be resolved first
    if(node->name() == types::ARRD) {
        auto nnode = dynamic_cast<types::ArrayDeclType*>(node.get());
        resolveNext(nnode->size);
        resolveDeclType(nnode->underlyingType);
    }
    // else nothing to do
}