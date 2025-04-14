#include "ast.hpp"

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

ast::ASTNode ast::convertVal(int node, parser::parseTree &tree){
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

ast::ASTNode ast::convertExp(int node, parser::parseTree &tree){
    for(int nNode: tree.adj[node]){
        std::string name = tree.id[nNode];
        if(name == "<UnAmb>"){
            return convertUnAmb(nNode, tree);
        }
    }
}

ast::ASTNode ast::convertUnAmb(int node, parser::parseTree &tree){
    for(int nNode: tree.adj[node]){
        std::string name = tree.id[nNode];
        if(name == "IDEN"){
            return Variable(tree.val[nNode]);
        }
    }
}

ast::ASTNode ast::convertAssn(int node, parser::parseTree &tree){
    
}

ast::ASTNode ast::convertVarDecl(int node, parser::parseTree &tree){
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode][1] == '<'){
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
                ASTNode value = convertVal(val, tree);
                return Let(var, type, value);
            }else{
                return Let(var, type);
            }
        }
    }
}

ast::ASTNode ast::convertDecl(int node, parser::parseTree &tree){
    for(auto nNode: tree.adj[node]){
        if(tree.id[nNode] == "<VarDecl>"){
            return convertVarDecl(nNode, tree);
        }
    }
}

ast::Prog ast::parseTreeToAST(parser::parseTree &tree){
    Prog root;
    int curNode = 0;
    do{
        ASTNode d = convertDecl(tree.adj[curNode][0], tree);
        root.addDecl(d);
        curNode = tree.adj[curNode][1];
    }while(tree.adj[curNode].size() > 1);
}