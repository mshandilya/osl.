#include "ast.hpp"

#define LOG(x) //std::cout<<x<<std::endl;

// void ast::vizTree(const std::unique_ptr<ASTNode>& node, const std::string &prefix, bool isLast){
//     LOG("inside viztree")
//     std::cout << prefix;
//     if (!prefix.empty())
//         std::cout << "+-";
//     std::string newPrefix = prefix.empty() ? "   " : prefix + (isLast ? "   " : "| ");
//     switch(node->type()){
//         case PROG_AST: {
//             std::cout << "Prog" << std::endl;
//             const Prog* p = dynamic_cast<const Prog*>(node.get());
//             for(size_t i = 0;i < p->decls.size();i++){
//                 vizTree(p->decls[i], newPrefix, i==p->decls.size()-1);
//             }
//             break;
//         }
//         case LET_AST: {
//             std::cout << "Let(";
//             const Let* l = dynamic_cast<const Let*>(node.get());
//             std::cout << "type=";
//             switch(l->typ) {
//                 case types::INT_8:
//                     std::cout << "int8";
//                     break;
//                 case types::INT_16:
//                     std::cout << "int16";
//                     break;
//                 case types::INT_32:
//                     std::cout << "int32";
//                     break;
//                 case types::INT_64:
//                     std::cout << "int64";
//                     break;
//                 case types::INT_128:
//                     std::cout << "int128";
//                     break;
//                 case types::BOOL:
//                     std::cout << "boolean";
//                     break;
//                 case types::CHAR_8:
//                     std::cout << "char8";
//                     break;
//                 default:
//                     std::cout << "unknown";
//             }  
//             std::cout << ", access=" << l->acc << ")" << std::endl;
//             if(l->val != nullptr){
//                 vizTree(l->var, newPrefix, false);
//                 vizTree(l->val, newPrefix, true);
//             }else{
//                 vizTree(l->var, newPrefix, true);
//             }
//             break;
//         }
//         case IDEN_AST: {
//             const Identifier* v = dynamic_cast<const Identifier*>(node.get());
//             std::cout << "Variable(varName=" << v->idenName << ")" << std::endl;
//             break;
//         }
//         case NUM_AST: {
//             const AtomicASTNode* n = dynamic_cast<const AtomicASTNode*>(node.get());
//             std::cout << "NumValue(dataType=";
//             switch(n->dataType()){
//                 case types::INT_8:
//                     std::cout << "int8";
//                     break;
//                 case types::INT_16:
//                     std::cout << "int16";
//                     break;
//                 case types::INT_32:
//                     std::cout << "int32";
//                     break;
//                 case types::INT_64:
//                     std::cout << "int64";
//                     break;
//                 case types::INT_128:
//                     std::cout << "int128";
//                     break;
//                 default:
//                     std::cout << "unknown";
//             }
//             std::cout << ")" << std::endl;
//             break;
//         }
//         case NULL_AST: {
//             std::cout << "NullValue()" << std::endl;
//             break;
//         }
//         case UOP_AST: {
//             const UnaryOperator* u = dynamic_cast<const UnaryOperator*>(node.get());
//             std::cout << "UnaryOperator(op=" << u->op << ")" << std::endl;
//             vizTree(u->child, newPrefix, true);
//             break;
//         }
//         case BOP_AST: {
//             const BinaryOperator* b = dynamic_cast<const BinaryOperator*>(node.get());
//             std::cout << "BinaryOperator(op=" << b->op << ")" << std::endl;
//             vizTree(b->leftChild, newPrefix, false);
//             vizTree(b->rightChild, newPrefix, true);
//             break;
//         }
//         case ASSIGN_AST: {
//             const Assign* a = dynamic_cast<const Assign*>(node.get());
//             std::cout << "Assign" << std::endl;
//             vizTree(a->var, newPrefix, false);
//             vizTree(a->val, newPrefix, true);
//             break;
//         }
//         case IF_AST: {
//             const If* i = dynamic_cast<const If*>(node.get());
//             std::cout << "If" << std::endl;
//             vizTree(i->cond, newPrefix, false);
//             if(i->elseBody == nullptr){
//                 vizTree(i->thenBody, newPrefix, true);    
//             }else{
//                 vizTree(i->thenBody, newPrefix, false);
//                 vizTree(i->elseBody, newPrefix, true);
//             }
//             break;
//         }
//         case LOG_AST: {
//             const Log* l = dynamic_cast<const Log*>(node.get());
//             std::cout << "Log" << std::endl;
//             vizTree(l->val, newPrefix, true);
//             break;
//         }
//         case RET_AST: {
//             const Return* r = dynamic_cast<const Return*>(node.get());
//             std::cout << "Return" << std::endl;
//             vizTree(r->val, newPrefix, true);
//             break;
//         }
//         case LETFUN_AST: {
//             const LetFun* l = dynamic_cast<const LetFun*>(node.get());
//             const Identifier* i = dynamic_cast<const Identifier*>(l->name.get());
//             std::cout << "LetFun(name=" << i->idenName << ", retType=";
//             switch(l->retType){
//                 case types::INT_8:
//                     std::cout << "int8";
//                     break;
//                 case types::INT_16:
//                     std::cout << "int16";
//                     break;
//                 case types::INT_32:
//                     std::cout << "int32";
//                     break;
//                 case types::INT_64:
//                     std::cout << "int64";
//                     break;
//                 case types::INT_128:
//                     std::cout << "int128";
//                     break;
//                 case types::BOOL:
//                     std::cout << "boolean";
//                     break;
//                 case types::CHAR_8:
//                     std::cout << "char8";
//                     break;
//                 default:
//                     std::cout << "unknown";
//             }  
//             std::cout << ")" << std::endl;
//             std::cout << prefix << "+-Params=" << std::endl;
//             for(size_t i = 0;i < l->params.size();i++){
//                 vizTree(l->params[i].second, newPrefix, i==l->params.size()-1);
//             }
//             std::cout << prefix << "+-Body=" << std::endl;
//             vizTree(l->body, newPrefix, true);
//             break;
//         }
//         case WHILE_AST: {
//             const While* w = dynamic_cast<const While*>(node.get());
//             std::cout << "While" << std::endl;
//             std::cout << prefix << "+-Cond=" << std::endl;
//             vizTree(w->cond, newPrefix, true);
//             std::cout << prefix << "+-Body=" << std::endl;
//             vizTree(w->body, newPrefix, true);
//             break;
//         }
//         case FUNCALL_AST: {
//             const FunCall* f = dynamic_cast<const FunCall*>(node.get());
//             std::cout << "FunCall" << std::endl;
//             std::cout << prefix << "+-Name=" << std::endl;
//             vizTree(f->name, newPrefix, true);
//             std::cout << prefix << "+-Params=" << std::endl;
//             for(size_t i = 0;i < f->params.size();i++){
//                 vizTree(f->params[i], newPrefix, i==f->params.size()-1);
//             }
//             break;
//         }
//     }
// }

std::unique_ptr<types::Type> ast::convertType(std::string type){
    // to be added: functions and arrays and pointers
    LOG("inside type")
    LOG(type)
    std::string bits = type.substr(2, type.length()-2);
    if(type[1] == 'I') {
        if(bits == "8")
            return std::make_unique<types::i8>();
        else if(bits == "16")
            return std::make_unique<types::i16>();
        else if(bits == "32")
            return std::make_unique<types::i32>();
        else if(bits == "64")
            return std::make_unique<types::i64>();
        else
            return std::make_unique<types::i128>();
    }
    else if(type[1] == 'U') {
        if(bits == "8")
            return std::make_unique<types::u8>();
        else if(bits == "16")
            return std::make_unique<types::u16>();
        else if(bits == "32")
            return std::make_unique<types::u32>();
        else if(bits == "64")
            return std::make_unique<types::u64>();
        else
            return std::make_unique<types::u128>();
    }
    else if(type[1] == 'F') {
        if(bits == "16")
            return std::make_unique<types::f16>();
        else if(bits == "32")
            return std::make_unique<types::f32>();
        else if(bits == "64")
            return std::make_unique<types::f64>();
        else
            return std::make_unique<types::f128>();
    }
    else if(type[1] == 'C') {
        if(bits == "8")
            return std::make_unique<types::c8>();
    }
    else if(type[1] == 'B') {
        return std::make_unique<types::Boolean>();
    }
    else if(type[1] == 'F') {
        // a function
    }
}

std::unique_ptr<ast::ASTNode> ast::convertVal(int node, parser::parseTree &tree){
    LOG("inside val")
    for(int nNode: tree.adj[node]){
        std::string name = tree.id[nNode];
        if(name == "<Exp>"){
            return convertExp(nNode, tree);
        }else if(name == "<Assn>"){
            return convertAssn(nNode, tree);
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertUnaryOp(int node, parser::parseTree &tree){
    // to be added: <PtrDeref> | <FunCall> | <ArrAcc> | <Ptr>
    LOG("inside unary op")
    OperatorType typ;
    if(tree.id[node] == "<UnNot>") {
        typ = UNOT_OP;
    }
    else if(tree.id[node] == "<UnSign>" and tree.val[node] == "NEG") {
        typ = UNEG_OP;
    }
    else if(tree.id[node] == "<UnSign>") {
        for(int nNode: tree.adj[node]){
            if(tree.id[nNode] == "<UnAmb>") {
                return convertUnAmb(nNode, tree);
            }
            else if(tree.id[nNode] == tree.id[node]) {
                return convertUnaryOp(nNode, tree);
            }
        }
    }
    for(int nNode: tree.adj[node]) {
        if(tree.id[nNode] == "<UnAmb>") {
            return std::make_unique<UnaryOperator>(typ, convertUnAmb(nNode, tree));
        }
        else if(tree.id[nNode] == tree.id[node]) {
            return std::make_unique<UnaryOperator>(typ, convertUnaryOp(nNode, tree));
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertBinaryOp(int node, parser::parseTree &tree){
    LOG("inside binary op")
    std::string opNames[] = {"<Add>", "<Subtract>", "<Multiply>", "<Divide>", "<Power>", "<Modulo>", "<Or>", "<And>", "<Xor>", "<Xand>", "<LeftShift>", "<RightShift>", "<Eq>", "<NotEq>", "<Lesser>", "<LessEqual>", "<Greater>", "<GreatEqual>"};
    OperatorType typs[] = {ADD_OP, SUB_OP, MUL_OP, DIV_OP, POW_OP, MOD_OP, OR_OP, AND_OP, XOR_OP, XAND_OP, LSHIFT_OP, RSHIFT_OP, EQ_OP, NEQ_OP, LT_OP, LEQ_OP, GT_OP, GEQ_OP};
    std::string name = tree.id[node];
    OperatorType typ;
    for(int i = 0;i < sizeof(opNames)/sizeof(opNames[0]);i++) {
        if(opNames[i] == name) {
            typ = typs[i];
            break;
        }
    }
    std::unique_ptr<ASTNode> lc = nullptr, rc = nullptr;
    for(int nNode: tree.adj[node]) {
        if(tree.id[nNode][0] == '<') {
            if(lc == nullptr) {
                if(tree.id[nNode] == "<UnAmb>") {
                    lc = std::move(convertUnAmb(nNode, tree));
                }
                else {
                    lc = std::move(convertBinaryOp(nNode, tree));
                }
            }
            else {
                if(tree.id[nNode] == "<UnAmb>") {
                    rc = std::move(convertUnAmb(nNode, tree));
                }
                else {
                    rc = std::move(convertBinaryOp(nNode, tree));
                }
            }
        }
    }
    return std::make_unique<BinaryOperator>(typ, std::move(lc), std::move(rc));
}

std::unique_ptr<ast::ASTNode> ast::convertExp(int node, parser::parseTree &tree){
    LOG("inside exp")
    for(int nNode: tree.adj[node]){
        std::string name = tree.id[nNode];
        if(name == "<UnAmb>"){
            return convertUnAmb(nNode, tree);
        }else if(name == "<UOp>"){
            // unimplemented
            //return convertUnaryOp(nNode, tree);
        }else if(name == "<BinOp>"){
            // unimplemented
            //return convertBinaryOp(nNode, tree);
        }
    }
}

std::vector<std::unique_ptr<ast::ASTNode>> ast::convertCallParams(int node, parser::parseTree &tree){
    std::vector<std::unique_ptr<ASTNode>> params;
    int current = node;
    while(true){
        for(int child: tree.adj[current]){
            if(tree.id[child] == "<Val>"){
                params.push_back(convertVal(child, tree));
                break;
            }
        }
        bool foundComma = false;
        int nextCallParams = -1;
        for(size_t i = 0; i < tree.adj[current].size(); i++){
            int child = tree.adj[current][i];
            if(tree.id[child] == "COMMA"){
                foundComma = true;
                if(i+1 < tree.adj[current].size()){
                    nextCallParams = tree.adj[current][i+1];
                }
                break;
            }
        }
        if(foundComma && nextCallParams != -1){
            current = nextCallParams;
        }else{
            break;
        }
    }
    return params;
}

std::unique_ptr<ast::ASTNode> ast::convertFunCall(int node, parser::parseTree &tree){
    std::unique_ptr<ASTNode> callee;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "IDEN"){
            callee = std::make_unique<Identifier>(tree.val[nNode]);
        }
    }
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Calls>"){
            int callsNode = nNode;
            while(true){
                bool processedCall = false;
                for(int child: tree.adj[callsNode]) {
                    if(tree.id[child] == "<Call>"){
                        std::vector<std::unique_ptr<ASTNode>> params;
                        for(int callChild : tree.adj[child]){
                            if(tree.id[callChild] == "<CallParams>"){
                                params = convertCallParams(callChild, tree);
                            }
                        }
                        auto newCall = std::make_unique<FunctionCall>(std::move(callee));
                        for(auto& param: params){
                            newCall->addParam(std::move(param));
                        }
                        callee = std::move(newCall);
                        processedCall = true;
                    }
                }
                bool foundNextCalls = false;
                for(int child: tree.adj[callsNode]){
                    if(tree.id[child] == "<Calls>"){
                        callsNode = child;
                        foundNextCalls = true;
                        break;
                    }
                }
                if(!foundNextCalls){
                    break;
                }
            }
        }
    }
    return callee;
}

std::unique_ptr<ast::ASTNode> ast::convertAtom(int node, parser::parseTree &tree){
    LOG("inside atom")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "NULL"){
            // unimplemented
        }else if(tree.id[nNode] == "<Bool>"){
            // unimplemented
        }else if(tree.id[nNode] == "<Char>"){
            // unimplemented
        }else if(tree.id[nNode] == "<Number>"){
            // unimplemented
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertUnAmb(int node, parser::parseTree &tree){
    LOG("inside unamb")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Atom>"){
            return convertAtom(nNode, tree);
            // unimplemented
        }else if(tree.id[nNode] == "<Arr>"){
            // unimplemented
        }else if(tree.id[nNode] == "<Ptr>"){
            // unimplemented
        }else if(tree.id[nNode] == "IDEN"){
            return std::make_unique<Identifier>(tree.val[nNode]);
        }else if(tree.id[nNode] == "<ArrAcc>"){
            // unimplemented
        }else if(tree.id[nNode] == "<Exp>"){
            return convertExp(nNode, tree);
        }
        /*if(name == "IDEN") {
            return std::make_unique<Identifier>(tree.val[nNode]);
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
        else if(name == "<Exp>"){
            return convertExp(nNode, tree);
        } 
        else if(name == "<Bool>"){
            // unimplemented    
        } 
        else if(name == "<FunCall>"){
            return convertFunCall(nNode, tree);
        }*/
    }
}

std::unique_ptr<ast::ASTNode> ast::convertAssn(int node, parser::parseTree &tree){
    LOG("inside assn")
    std::unique_ptr<ASTNode> var,val;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Loc>") {
            // unimplemented
        }else if(tree.id[nNode] == "<Val>"){
            val = std::move(convertVal(nNode, tree));
        }
    }
    return std::make_unique<Assign>(std::move(var), std::move(val));
}

std::unique_ptr<ast::ASTNode> ast::convertExpStmt(int node, parser::parseTree &tree){
    LOG("inside exp stmt")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Val>"){
            return convertVal(nNode, tree);
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertBlock(int node, parser::parseTree &tree){
    LOG("inside block")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Prog>"){
            return convertProg(nNode, tree);
        }
    }
    return nullptr;
}

std::unique_ptr<ast::ASTNode> ast::convertElse(int node, parser::parseTree &tree){
    LOG("inside else")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Block>"){
            return convertBlock(nNode, tree);
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertElifs(int node, parser::parseTree &tree, std::unique_ptr<ASTNode> finElse){
    LOG("inside elif")
    std::unique_ptr<ASTNode> ptrIf;
    int elifNode = -1;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Elif>"){
            ptrIf = std::move(convertCond(nNode, tree));
        }else if(tree.id[nNode] == "<Elifs>"){
            elifNode = nNode;
        }
    }
    Conditional* ptr = dynamic_cast<Conditional*>(ptrIf.get());
    if(elifNode != -1){
        ptr->elseBody = std::move(convertElifs(elifNode, tree, std::move(finElse)));
    }else{
        ptr->elseBody = std::move(finElse);
    }
    return ptrIf;
}

std::unique_ptr<ast::ASTNode> ast::convertCond(int node, parser::parseTree &tree){
    LOG("inside cond")
    std::unique_ptr<ASTNode> cond, thenBody, elseBody = nullptr;
    int elifNode = -1;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Val>"){
            cond = convertVal(nNode, tree);
        }else if(tree.id[nNode] == "<Block>"){
            thenBody = convertBlock(nNode, tree);
        }else if(tree.id[nNode] == "<Else>"){
            elseBody = convertElse(nNode, tree);
        }else if(tree.id[nNode] == "<Elifs>"){
            elifNode = nNode;
        }
    }
    if(elifNode != -1){
        if(elseBody == nullptr){
            return std::make_unique<Conditional>(std::move(cond), std::move(thenBody), convertElifs(elifNode, tree));
        }else{
            return std::make_unique<Conditional>(std::move(cond), std::move(thenBody), convertElifs(elifNode, tree, std::move(elseBody)));
        }
    }else{
        if(elseBody == nullptr){
            return std::make_unique<Conditional>(std::move(cond), std::move(thenBody));
        }else{
            return std::make_unique<Conditional>(std::move(cond), std::move(thenBody), std::move(elseBody));
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertLogStmt(int node, parser::parseTree &tree){
    LOG("inside log stmt")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Val>"){
            return std::make_unique<Log>(convertVal(nNode, tree));
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertRetStmt(int node, parser::parseTree &tree){
    LOG("inside ret stmt")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Val>"){
            return std::make_unique<Return>(convertVal(nNode, tree));
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertLoop(int node, parser::parseTree &tree){
    std::unique_ptr<ASTNode> cond,body;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Val>"){
            cond = std::move(convertVal(nNode, tree));
        }else if(tree.id[nNode] == "<Block>"){
            body = std::move(convertBlock(nNode, tree));
        }
    }
    return std::make_unique<Loop>(std::move(cond), std::move(body));
}

std::unique_ptr<ast::ASTNode> ast::convertStmt(int node, parser::parseTree &tree){
    LOG("inside stmt")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<ExpStmt>"){
            // unimplemented
            //return convertExpStmt(nNode, tree);
        }else if(tree.id[nNode] == "<LogStmt>"){
            return convertLogStmt(nNode, tree);
        }else if(tree.id[nNode] == "<RetStmt>"){
            return convertRetStmt(nNode, tree);
        }else if(tree.id[nNode] == "<Cond>"){
            return convertCond(nNode, tree);
        }else if(tree.id[nNode] == "<Loop>"){
            return convertLoop(nNode, tree);
        }
    }
}

std::pair<std::unique_ptr<types::Type>, std::unique_ptr<ast::Identifier>> ast::convertParam(int node, parser::parseTree &tree){
    LOG("inside param")
    std::unique_ptr<types::Type> typ;
    std::string iden;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<GenType>"){
            typ = ast::convertType(tree.id[tree.adj[tree.adj[nNode][0]][0]]);
        }
        else if(tree.id[nNode] == "IDEN"){
            iden = tree.val[nNode];
        }
    }
    auto ptr = std::make_unique<ast::Identifier>(iden);
    return {std::move(typ), std::move(ptr)};
}

std::unique_ptr<ast::ASTNode> ast::convertFunDecl(int node, parser::parseTree &tree){
    LOG("inside fun decl")
    std::unique_ptr<types::Type> retType;
    std::string iden;
    std::unique_ptr<ASTNode> body;
    int paramNode = -1;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<GenType>"){
            retType = convertType(tree.id[tree.adj[tree.adj[nNode][0]][0]]);
        } 
        else if(tree.id[nNode] == "IDEN"){
            iden = tree.val[nNode];
        } 
        else if(tree.id[nNode] == "<Block>"){
            body = convertBlock(nNode, tree);
        } 
        else if(tree.id[nNode] == "<Params>"){
            paramNode = nNode;
        }
    }
    std::unique_ptr<LetFun> ptr = std::make_unique<LetFun>(std::move(retType), std::make_unique<Identifier>(iden), std::move(body));
    if(paramNode == -1){
        return ptr;
    }
    int curNode = paramNode;
    while(tree.adj[curNode].size() > 1){
        LOG("inside while at fun decl (trying to parse params)")
        auto d = convertParam(tree.adj[curNode][0], tree);
        LOG("parsed a param")
        ptr->addParam(std::move(d.first), std::move(d.second));
        LOG("added param")
        curNode = tree.adj[curNode][2];
    }
    LOG("outside while loop")
    auto d = convertParam(tree.adj[curNode][0], tree);
    LOG("another param parsed")
    ptr->addParam(std::move(d.first), std::move(d.second));
    LOG("another param added")
    return ptr;
}

std::unique_ptr<ast::ASTNode> ast::convertConstDecl(int node, parser::parseTree &tree){
    LOG("inside constdecl")
    std::unique_ptr<ASTNode> idenPtr, valPtr;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<DeclType>"){
            // unimplemented
        }else if(tree.id[nNode] == "IDEN"){
            idenPtr = std::make_unique<Identifier>(tree.val[nNode]);
        }else if(tree.id[nNode] == "<Val>"){
            valPtr = std::move(convertVal(nNode, tree));
        }
    }
    return std::make_unique<LetConst>(std::move(idenPtr), std::move(valPtr));
}

std::unique_ptr<ast::ASTNode> ast::convertVarDecl(int node, parser::parseTree &tree){
    LOG("inside vardecl")
    std::unique_ptr<ASTNode> idenPtr, valPtr=nullptr;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<DeclType>"){
            // unimplemented
        }else if(tree.id[nNode] == "IDEN"){
            idenPtr = std::make_unique<Identifier>(tree.val[nNode]);
        }else if(tree.id[nNode] == "<Val>"){
            valPtr = std::move(convertVal(nNode, tree));
        }
    }
    if(valPtr == nullptr){
        return std::make_unique<LetVar>(std::move(idenPtr));
    }else{
        return std::make_unique<LetVar>(std::move(idenPtr), std::move(valPtr));
    }
}

std::unique_ptr<ast::ASTNode> ast::convertDecl(int node, parser::parseTree &tree){
    LOG("inside decl")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<ConstDecl>"){
            return convertConstDecl(nNode, tree);
        }else if(tree.id[nNode] == "<VarDecl>"){
            return convertVarDecl(nNode, tree);
        }else if(tree.id[nNode] == "<Stmt>"){
            return convertStmt(nNode, tree);
        }else if(tree.id[nNode] == "<FunDecl>"){
            //unimplemented
            //return convertFunDecl(nNode, tree);
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertProg(int node, parser::parseTree &tree){
    std::unique_ptr<ast::Prog> root = std::make_unique<ast::Prog>();
    int curNode = node;
    while(tree.adj[curNode].size() > 1){
        LOG("branched at prog")
        auto d = convertDecl(tree.adj[curNode][0], tree);
        root->addDecl(std::move(d));
        curNode = tree.adj[curNode][1];
    }
    LOG("out of loop at prog")
    auto d = convertDecl(tree.adj[curNode][0], tree);
    LOG("decl done at prog")
    root->addDecl(std::move(d));
    LOG("decl added at prog")
    return root;
}

std::unique_ptr<ast::ASTNode> ast::parseTreeToAST(parser::parseTree &tree) {
    return convertProg(0, tree);
}