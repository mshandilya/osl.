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
        case LET_AST:
            std::cout << "Let" << std::endl;
            break;
        case VAR_AST:
            std::cout << "Variable" << std::endl;
            break;
    }
}

ast::VarType ast::convertType(std::string type){
    types::Type typ;
    types::MAX_BITS size;
    if(type[1] == 'I'){
        typ = types::INT;
    }else if(type[1] == 'U'){
        typ = types::UNSIGNED;
    }else if(type[1] == 'F'){
        typ = types::FLOAT;
    }else if(type[1] == 'C'){
        typ = types::CHAR;
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
    if(typ == types::CHAR || typ == types::BOOL){
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

std::unique_ptr<ast::ASTNode> ast::convertExp(int node, parser::parseTree &tree){
    for(int nNode: tree.adj[node]){
        std::string name = tree.id[nNode];
        if(name == "<UnAmb>"){
            return convertUnAmb(nNode, tree);
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertUnAmb(int node, parser::parseTree &tree){
    for(int nNode: tree.adj[node]){
        std::string name = tree.id[nNode];
        if(name == "IDEN"){
            return std::make_unique<Variable>(tree.val[nNode]);
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertAssn(int node, parser::parseTree &tree){
    
}

std::unique_ptr<ast::ASTNode> ast::convertVarDecl(int node, parser::parseTree &tree){
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode][0] == '<'){
            std::string iden;
            ast::VarType type;
            int val = -1, access = VAR;
            if(tree.id[nNode] == "<Immutable>"){
                access = CONST;
            }
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
            Variable var = Variable(iden);
            if(val != -1){
                auto value = convertVal(val, tree);
                return std::make_unique<Let>(var, type, *value);
            }else{
                return std::make_unique<Let>(var, type);
            }
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertDecl(int node, parser::parseTree &tree){
    for(auto nNode: tree.adj[node]){
        if(tree.id[nNode] == "<VarDecl>"){
            return convertVarDecl(nNode, tree);
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::parseTreeToAST(parser::parseTree &tree) {
    std::unique_ptr<Prog> root;
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