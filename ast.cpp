#include "ast.hpp"

void ast::vizTree(const std::unique_ptr<ASTNode>& node, const std::string &prefix, bool isLast){
    std::cout << prefix;
    if (!prefix.empty())
        std::cout << "+-";
    std::string newPrefix = prefix.empty() ? "   " : prefix + (isLast ? "   " : "| ");
    switch(node->type()){
        case PROG_AST: {
            std::cout << "Prog" << std::endl;
            const Prog* p = dynamic_cast<const Prog*>(node.get());
            for(size_t i = 0;i < p->decls.size();i++){
                vizTree(p->decls[i], newPrefix, i==p->decls.size()-1);
            }
            break;
        }
        case LET_AST: {
            std::cout << "Let(";
            const Let* l = dynamic_cast<const Let*>(node.get());
            std::cout << "type=" << l->typ.type << ", size=" << l->typ.size << ", access=" << l->acc << ")" << std::endl;
            if(l->val != nullptr){
                vizTree(l->var, newPrefix, false);
                vizTree(l->val, newPrefix, true);
            }else{
                vizTree(l->var, newPrefix, true);
            }
            break;
        }
        case VAR_AST: {
            const Variable* v = dynamic_cast<const Variable*>(node.get());
            std::cout << "Variable(varName=" << v->varName << ")" << std::endl;
            break;
        }
        case NUM_AST: {
            std::cout << "NumValue()" << std::endl;
            break;
        }
        case UOP_AST: {
            const UnaryOperator* u = dynamic_cast<const UnaryOperator*>(node.get());
            std::cout << "UnaryOperator(op=" << u->op << ")" << std::endl;
            vizTree(u->child, newPrefix, true);
            break;
        }
        case BOP_AST: {
            const BinaryOperator* b = dynamic_cast<const BinaryOperator*>(node.get());
            std::cout << "BinaryOperator(op=" << b->op << ")" << std::endl;
            vizTree(b->leftChild, newPrefix, false);
            vizTree(b->rightChild, newPrefix, true);
            break;
        }
    }
}

ast::VarType ast::convertType(std::string type){
    types::TYPES typ;
    types::MAX_BITS size;
    if(type[1] == 'I'){
        typ = types::INT_UNRESOLVED;
    }else if(type[1] == 'U'){
        typ = types::UINT_UNRESOLVED;
    }else if(type[1] == 'F'){
        typ = types::FLOAT_UNRESOLVED;
    }else if(type[1] == 'C'){
        typ = types::CHAR_UNRESOLVED;
    }else if(type[1] == 'B'){
        typ = types::BOOL;
    }
    std::string bits = type.substr(2,type.length()-2);
    if(bits == "8"){
        size = types::B8;
    }else if(bits == "16"){
        size = types::B16;
    }else if(bits == "32"){
        size = types::B32;
    }else if(bits == "64"){
        size = types::B64;
    }else if(bits == "128"){
        size = types::B128;
    }
    if(typ == types::CHAR_UNRESOLVED || typ == types::BOOL){
        return VarType(typ);
    }else{
        return VarType(typ, size);
    }
}

std::unique_ptr<ast::ASTNode> ast::convertVal(int node, parser::parseTree &tree){
    for(int nNode: tree.adj[node]){
        std::string name = tree.id[nNode];
        if(name == "<Exp>"){
            return convertExp(nNode, tree);
        }else if(name == "<Assn>"){
            return convertAssn(nNode, tree);
        }else if(name == "NULL"){

        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertUnaryOp(int node, parser::parseTree &tree){
    OperatorType typ;
    if(tree.id[node] == "<Not>"){
        typ = NOT_OP;
    }else if(tree.id[node] == "<UnaryNeg>"){
        typ = UNEG_OP;
    }
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<UnAmb>"){
            return std::make_unique<UnaryOperator>(typ, convertUnAmb(nNode, tree));
        }else if(tree.id[nNode] == tree.id[node]){
            return std::make_unique<UnaryOperator>(typ, convertUnaryOp(nNode, tree));
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertBinaryOp(int node, parser::parseTree &tree){
    std::string opNames[2] = {"<Add>", "<Subtract>"};
    OperatorType typs[2] = {ADD_OP, SUB_OP};
    std::string name = tree.id[node];
    OperatorType typ;
    for(int i = 0;i < 2;i++){
        if(opNames[i] == name){
            typ = typs[i];
            break;
        }
    }
    std::unique_ptr<ASTNode> lc = nullptr,rc = nullptr;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<UnAmb>"){
            if(lc == nullptr){
                lc = std::move(convertUnAmb(nNode, tree));
            }else{
                rc = std::move(convertUnAmb(nNode, tree));
            }
        }else if(tree.id[nNode] == tree.id[node]){
            rc = std::move(convertBinaryOp(nNode, tree));
        }
    }
    return std::make_unique<BinaryOperator>(typ, std::move(lc), std::move(rc));
}

std::unique_ptr<ast::ASTNode> ast::convertExp(int node, parser::parseTree &tree){
    for(int nNode: tree.adj[node]){
        std::string name = tree.id[nNode];
        if(name == "<UnAmb>"){
            return convertUnAmb(nNode, tree);
        }else if(name == "<Not>" || name == "<UnaryNeg>"){
            return convertUnaryOp(nNode, tree);
        }else{
            return convertBinaryOp(nNode, tree);
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertUnAmb(int node, parser::parseTree &tree) {
    for(int nNode: tree.adj[node]){
        std::string name = tree.id[nNode];
        if(name == "IDEN"){
            return std::make_unique<Variable>(tree.val[nNode]);
        }else if(name == "<Number>"){
            auto numUtil = utils::stringToNumberUtil(tree.val[tree.adj[nNode][0]]);
            switch (numUtil.first) {
                case types::B8:
                    return std::make_unique<NumValue<types::i8>>(types::i8(numUtil.second));
                case types::B16:
                    return std::make_unique<NumValue<types::i16>>(types::i16(numUtil.second));
                case types::B32:
                    return std::make_unique<NumValue<types::i32>>(types::i32(numUtil.second));
                case types::B64:
                    return std::make_unique<NumValue<types::i64>>(types::i64(numUtil.second));
                case types::B128:
                    return std::make_unique<NumValue<types::i128>>(types::i128(numUtil.second));
            }
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertAssn(int node, parser::parseTree &tree){
    
}

std::unique_ptr<ast::ASTNode> ast::convertVarDecl(int node, parser::parseTree &tree){
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode][0] == '<'){
            int val = -1;
            Access access = VAR;
            if(tree.id[nNode] == "<Immutable>"){
                access = CONST;
            }
            VarType type;
            std::string iden;
            for(int tNode: tree.adj[nNode]){
                std::string name = tree.id[tNode];
                if(name == "<Type>"){
                    type = convertType(tree.id[tree.adj[tNode][0]]);
                }else if(name == "<Assn>"){
                    for(int iNode: tree.adj[tNode]){
                        if(tree.id[iNode] == "IDEN"){
                            iden = tree.val[iNode];
                        }else if(tree.id[iNode] == "<Val>"){
                            val = iNode;
                        }
                    }
                }else if(name == "IDEN"){
                    iden = tree.val[tNode];
                }
            }
            auto ptr = std::make_unique<ast::Variable>(iden);
            if(val != -1){
                auto value = convertVal(val, tree);
                return std::make_unique<Let>(std::move(ptr), type, access, std::move(value));
            }else{
                return std::make_unique<Let>(std::move(ptr), type, access);
            }
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertDecl(int node, parser::parseTree &tree){
    for(auto nNode: tree.adj[node]){
        if(tree.id[nNode] == "<VarDecl>") {
            auto ret = convertVarDecl(nNode, tree);
            return ret;
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::parseTreeToAST(parser::parseTree &tree) {
    std::unique_ptr<ast::Prog> root = std::make_unique<ast::Prog>();
    int curNode = 0;
    while(tree.adj[curNode].size() > 1){
        auto d = convertDecl(tree.adj[curNode][0], tree);
        root->addDecl(std::move(d));
        curNode = tree.adj[curNode][1];
    }
    auto d = convertDecl(tree.adj[curNode][0], tree);
    root->addDecl(std::move(d));
    return root;
}