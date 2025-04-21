#include "ast.hpp"

#define LOG(x) std::cout<<x<<std::endl;

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

void ast::vizTree(const std::unique_ptr<ASTNode>& node, const std::string &prefix, bool isLast, bool postResolve){
    std::cout << prefix;
    if (!prefix.empty()) std::cout << "+-";

    std::string newPrefix = prefix.empty() ? "   " : prefix + (isLast ? "   " : "| ");
    switch(node->type()){
        case PROG_AST: {
            const Prog* p = dynamic_cast<const Prog*>(node.get());
            std::cout << "Prog" << std::endl;
            for(size_t i = 0; i < p->decls.size(); ++i)
                vizTree(p->decls[i], newPrefix, i == p->decls.size() - 1, postResolve);
            break;
        }
        case BLOCK_AST: {
            const Block* b = dynamic_cast<const Block*>(node.get());
            std::cout << "Block" << std::endl;
            for(size_t i = 0; i < b->decls.size(); ++i)
                vizTree(b->decls[i], newPrefix, i == b->decls.size() - 1, postResolve);
            break;
        }
        case LOOP_AST: {
            const Loop* l = dynamic_cast<const Loop*>(node.get());
            std::cout << "Loop" << std::endl;
            // Condition
            std::cout << newPrefix << "+-Condition" << std::endl;
            {
                std::string childPref = newPrefix + "| ";
                vizTree(l->cond, childPref, false, postResolve);
            }
            // Body
            std::cout << newPrefix << "+-Body" << std::endl;
            {
                std::string childPref = newPrefix + "   ";
                vizTree(l->body, childPref, true, postResolve);
            }
            break;
        }
        case COND_AST: {
            const Conditional* c = dynamic_cast<const Conditional*>(node.get());
            std::cout << "Conditional" << std::endl;
            // Condition
            std::cout << newPrefix << "+-Condition" << std::endl;
            {
                std::string childPref = newPrefix + "| ";
                vizTree(c->cond, childPref, false, postResolve);
            }
            // Then
            std::cout << newPrefix << "+-Then" << std::endl;
            {
                bool hasElse = true;
                std::string childPref = newPrefix + (hasElse ? "| " : "   ");
                vizTree(c->thenBody, childPref, false, postResolve);
            }
            // Else
            std::cout << newPrefix << "+-Else" << std::endl;
            {
                std::string childPref = newPrefix + "   ";
                vizTree(c->elseBody, childPref, true, postResolve);
            }
            break;
        }
        case RET_AST: {
            const Return* r = dynamic_cast<const Return*>(node.get());
            std::cout << "Return" << std::endl;
            std::cout << newPrefix << "+-Value" << std::endl;
            {
                std::string childPref = newPrefix + "   ";
                vizTree(r->val, childPref, true, postResolve);
            }
            break;
        }
        case LOG_AST: {
            const Log* l = dynamic_cast<const Log*>(node.get());
            std::cout << "Log" << std::endl;
            std::cout << newPrefix << "+-Value" << std::endl;
            {
                std::string childPref = newPrefix + "   ";
                vizTree(l->val, childPref, true, postResolve);
            }
            break;
        }
        case LETCONST_AST: {
            const LetConst* lc = dynamic_cast<const LetConst*>(node.get());
            std::cout << "LetConst" << std::endl;
            std::cout << newPrefix << "+-Name" << std::endl;
            {
                std::string childPref = newPrefix + "| ";
                vizTree(lc->var, childPref, false);
            }
            std::cout << newPrefix << "+-Value" << std::endl;
            {
                std::string childPref = newPrefix + "   ";
                vizTree(lc->val, childPref, true, postResolve);
            }
            break;
        }
        case LETVAR_AST: {
            const LetVar* lv = dynamic_cast<const LetVar*>(node.get());
            std::cout << "LetVar" << std::endl;
            std::cout << newPrefix << "+-Name" << std::endl;
            {
                std::string childPref = newPrefix + "| ";
                vizTree(lv->var, childPref, false, postResolve);
            }
            std::cout << newPrefix << "+-Initializer" << std::endl;
            {
                std::string childPref = newPrefix + "   ";
                vizTree(lv->val, childPref, true, postResolve);
            }
            break;
        }
        case LETARR_AST: {
            const LetArray* la = dynamic_cast<const LetArray*>(node.get());
            std::cout << "LetArray" << std::endl;
            std::cout << newPrefix << "+-Name" << std::endl;
            {
                std::string childPref = newPrefix + "| ";
                vizTree(la->name, childPref, false, postResolve);
            }
            std::cout << newPrefix << "+-Initializer" << std::endl;
            {
                std::string childPref = newPrefix + "   ";
                vizTree(la->val, childPref, true, postResolve);
            }
            break;
        }
        case LETFUN_AST: {
            const LetFun* lf = dynamic_cast<const LetFun*>(node.get());
            std::cout << "LetFun" << std::endl;
            // Function name
            std::cout << newPrefix << "+-Name" << std::endl;
            {
                std::string childPref = newPrefix + "| ";
                vizTree(lf->name, childPref, !lf->params.empty(), postResolve);
            }
            // Parameters
            std::cout << newPrefix << "+-Parameters" << std::endl;
            {
                std::string paramPref = newPrefix + (lf->params.empty() ? "   " : "| ");
                for(size_t i = 0; i < lf->params.size(); ++i)
                    vizTree(lf->params[i].second, paramPref, i == lf->params.size() - 1, postResolve);
            }
            // Body
            std::cout << newPrefix << "+-Body" << std::endl;
            {
                std::string childPref = newPrefix + "   ";
                vizTree(lf->body, childPref, true, postResolve);
            }
            break;
        }
        case ASSIGN_AST: {
            const Assign* a = dynamic_cast<const Assign*>(node.get());
            std::cout << "Assign" << std::endl;
            std::cout << newPrefix << "+-LHS" << std::endl;
            {
                std::string childPref = newPrefix + "| ";
                vizTree(a->var, childPref, false, postResolve);
            }
            std::cout << newPrefix << "+-RHS" << std::endl;
            {
                std::string childPref = newPrefix + "   ";
                vizTree(a->val, childPref, true, postResolve);
            }
            break;
        }
        case BOP_AST: {
            const BinaryOperator* b = dynamic_cast<const BinaryOperator*>(node.get());
            std::cout << "BinaryOperator(op=" << b->op << ")" << std::endl;
            std::cout << newPrefix << "+-Left" << std::endl;
            {
                std::string childPref = newPrefix + "| ";
                vizTree(b->leftChild, childPref, false, postResolve);
            }
            std::cout << newPrefix << "+-Right" << std::endl;
            {
                std::string childPref = newPrefix + "   ";
                vizTree(b->rightChild, childPref, true, postResolve);
            }
            break;
        }
        case UOP_AST: {
            const UnaryOperator* u = dynamic_cast<const UnaryOperator*>(node.get());
            std::cout << "UnaryOperator(op=" << u->op << ")" << std::endl;
            std::cout << newPrefix << "+-Operand" << std::endl;
            {
                std::string childPref = newPrefix + "   ";
                vizTree(u->child, childPref, true, postResolve);
            }
            break;
        }
        case FUNCALL_AST: {
            const FunctionCall* fc = dynamic_cast<const FunctionCall*>(node.get());
            std::cout << "FunctionCall" << std::endl;
            // Callee name
            std::cout << newPrefix << "+-Function" << std::endl;
            {
                std::string childPref = newPrefix + "| ";
                vizTree(fc->name, childPref, !fc->params.empty(), postResolve);
            }
            // Arguments
            std::cout << newPrefix << "+-Arguments" << std::endl;
            {
                std::string argPref = newPrefix + (fc->params.empty() ? "   " : "| ");
                for(size_t i = 0; i < fc->params.size(); ++i)
                    vizTree(fc->params[i], argPref, i == fc->params.size() - 1, postResolve);
            }
            break;
        }
        case ARR_AST: {
            const ArrValue* av = dynamic_cast<const ArrValue*>(node.get());
            std::cout << "Array" << std::endl;
            std::cout << newPrefix << "+-Elements" << std::endl;
            {
                std::string elemPref = newPrefix + "| ";
                for(size_t i = 0; i < av->elems.size(); ++i)
                    vizTree(av->elems[i], elemPref, i == av->elems.size() - 1, postResolve);
            }
            break;
        }
        case NUM_AST:    std::cout << "Number" << std::endl;     break;
        case CHAR_AST:   std::cout << "Char" << std::endl;       break;
        case BOOL_AST:   std::cout << "Bool" << std::endl;       break;
        case NULL_AST:   std::cout << "Null" << std::endl;       break;
        case IDEN_AST: {
            const Identifier* v = dynamic_cast<const Identifier*>(node.get());
            if(postResolve)
                std::cout << "Identifier(name=" << v->idenName << ", id=" << v->id << ", type=" << v->boundDataType->name() << ")" << std::endl;
            else
                std::cout << "Identifier(name=" << v->idenName << ")" << std::endl;
            break;
        }
        default:
            std::cout << "<UnknownNode>" << std::endl;
            break;
    }
}

std::unique_ptr<ast::ASTNode> ast::convertFunCall(int node, parser::parseTree &tree){
    LOG("inside funcall")
    std::unique_ptr<ASTNode> iden;
    int cNode;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<UnAmb>"){
            iden = convertUnAmb(nNode, tree);
        }else if(tree.id[nNode] == "<Calls>"){
            cNode = nNode;
        }
    }
    auto ptr = std::make_unique<FunctionCall>(std::move(iden));
    int curNode = cNode;
    while(tree.adj[curNode].size() > 1){
        int aNode = tree.adj[tree.adj[curNode][0]][1];
        while(tree.adj[aNode].size() > 1){
            ptr->addParam(convertVal(tree.adj[aNode][0], tree));
            aNode = tree.adj[aNode][2];
        }
        ptr->addParam(convertVal(tree.adj[aNode][0], tree));
        ptr = std::make_unique<FunctionCall>(std::move(ptr));
        curNode = tree.adj[curNode][1];
    }
    int aNode = tree.adj[tree.adj[curNode][0]][1];
    while(tree.adj[aNode].size() > 1){
        ptr->addParam(convertVal(tree.adj[aNode][0], tree));
        aNode = tree.adj[aNode][2];
    }
    ptr->addParam(convertVal(tree.adj[aNode][0], tree));
    return ptr;
}

std::unique_ptr<ast::ASTNode> ast::convertUnaryOp(int node, parser::parseTree &tree){
    LOG("inside unary op")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<UnSign>" || tree.id[nNode] == "<UnNot>"){
            int curNode = nNode;
            int parity = 0;
            while(true){
                if(tree.id[tree.adj[curNode][0]] == "NEG" || tree.id[tree.adj[curNode][0]] == "NOT"){
                    parity ^= 1;
                }
                if(tree.id[tree.adj[curNode][1]] == "<UnAmb>"){
                    break;
                }
                curNode = tree.adj[curNode][1];
            }
            if(parity == 0){
                return convertUnAmb(tree.adj[curNode][1], tree);
            }else{
                if(tree.id[nNode] == "<UnSign>"){
                    return std::make_unique<UnaryOperator>(UNEG_OP, convertUnAmb(tree.adj[curNode][1], tree));
                }else{
                    return std::make_unique<UnaryOperator>(UNOT_OP, convertUnAmb(tree.adj[curNode][1], tree));
                }
            }
        }else if(tree.id[nNode] == "<PtrDeref>"){
            return convertPtrDeref(nNode, tree);
        }else if(tree.id[nNode] == "<FunCall>"){
            return convertFunCall(nNode, tree);
        }else if(tree.id[nNode] == "<Ptr>"){
            return convertPtr(nNode, tree);
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertAtom(int node, parser::parseTree &tree){
    LOG("inside atom")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "NULL"){
            return std::make_unique<NullValue>();
        }else if(tree.id[nNode] == "<Bool>"){
            if(tree.id[tree.adj[nNode][0]] == "TRUE"){
                return std::make_unique<BoolValue>(std::make_unique<types::Boolean>(true));
            }else{
                return std::make_unique<BoolValue>(std::make_unique<types::Boolean>(false));
            }
        }else if(tree.id[nNode] == "<Char>"){
            return std::make_unique<CharValue>(utils::stringToCharUtil(tree.val[tree.adj[nNode][0]]));
        }else if(tree.id[nNode] == "<Number>"){
            return std::make_unique<NumValue>(utils::stringToNumberUtil(tree.val[tree.adj[nNode][0]]));
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertPtr(int node, parser::parseTree &tree){
    LOG("inside ptr")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Loc>"){
            return std::make_unique<UnaryOperator>(PTRREF_OP, convertLoc(nNode, tree));
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertUnAmb(int node, parser::parseTree &tree){
    LOG("inside unamb")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Atom>"){
            return convertAtom(nNode, tree);
        }else if(tree.id[nNode] == "<Arr>"){
            return convertArr(nNode, tree);
        }else if(tree.id[nNode] == "<Ptr>"){
            return convertPtr(nNode, tree);
        }else if(tree.id[nNode] == "IDEN"){
            return std::make_unique<Identifier>(tree.val[nNode]);
        }else if(tree.id[nNode] == "<ArrAcc>"){
            return convertArrAcc(nNode, tree);
        }else if(tree.id[nNode] == "<Exp>"){
            return convertExp(nNode, tree);
        }
    }
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

std::unique_ptr<types::Type> ast::convertAtomType(int node, parser::parseTree &tree){
    LOG("inside atomtype")
    std::string type = tree.id[tree.adj[node][0]];
    std::string bits = type.substr(2, type.length()-2);
    if(type == "NULL")
        return std::make_unique<types::Null>();
    if(type[1] == 'I'){
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
    }else if(type[1] == 'U') {
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
    }else if(type[1] == 'F') {
        if(bits == "16")
            throw std::domain_error("Floats must be single or double point precision right now.");
            // return std::make_unique<types::f16>();
        else if(bits == "32")
            return std::make_unique<types::f32>();
        else if(bits == "64")
            return std::make_unique<types::f64>();
        else
            throw std::domain_error("Floats must be single or double point precision right now.");
            // return std::make_unique<types::f128>();
    }else if(type[1] == 'C') {
        if(bits == "8")
            return std::make_unique<types::c8>();
    }else if(type[1] == 'B') {
        return std::make_unique<types::Boolean>();
    }
}

std::vector<std::unique_ptr<types::Type>> ast::convertSigParams(int node, parser::parseTree &tree){
    LOG("inside sigparams")
    std::vector<std::unique_ptr<types::Type>> params;
    int curNode = node;
    while(tree.adj[curNode].size() > 1){
        LOG("branched at sigparams")
        params.push_back(convertType(tree.adj[curNode][0], tree));
        curNode = tree.adj[curNode][2];
    }
    LOG("out of loop at sigparams")
    params.push_back(convertType(tree.adj[curNode][0], tree));
    return params;
}

std::unique_ptr<types::Type> ast::convertPtrType(int node, parser::parseTree &tree){
    LOG("inside ptrtype")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Type>"){
            return std::make_unique<types::PointerType>(convertType(nNode, tree));
        }
    }
}

std::unique_ptr<types::Type> ast::convertArrType(int node, parser::parseTree &tree){
    LOG("inside arrtype")
    std::unique_ptr<types::Type> ut;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<AtomType>"){
            ut = convertAtomType(nNode, tree);
        }else if(tree.id[nNode] == "<FunType>"){
            ut = convertFunType(nNode, tree);
        }else if(tree.id[nNode] == "<PtrType>"){
            ut = convertPtrType(nNode, tree);
        }
    }
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Mats>"){
            int dim = 0;
            int curNode = nNode;
            while(tree.adj[curNode].size() > 2){
                dim++;
                for(int tNode: tree.adj[curNode]){
                    if(tree.id[tNode] == "<Mats>"){
                        curNode = tNode;
                        break;
                    }
                }
            }
            std::unique_ptr<types::ArrayType> ptr = std::make_unique<types::ArrayType>(std::move(ut));
            while(dim > 0){
                ptr = std::make_unique<types::ArrayType>(std::move(ptr));
                dim--;
            }
            return ptr;
        }
    }
}

std::unique_ptr<types::Type> ast::convertCompType(int node, parser::parseTree &tree){
    LOG("inside comptype")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<FunType>"){
            return convertFunType(nNode, tree);
        }else if(tree.id[nNode] == "<ArrType>"){
            return convertArrType(nNode, tree);
        }else if(tree.id[nNode] == "<PtrType>"){
            return convertPtrType(nNode, tree);
        }
    }
}

std::unique_ptr<types::Type> ast::convertType(int node, parser::parseTree &tree){
    LOG("inside type")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<AtomType>"){
            return convertAtomType(nNode, tree);
        }else if(tree.id[nNode] == "<CompType>"){
            return convertCompType(nNode, tree);
        }
    }
}

std::unique_ptr<types::Type> ast::convertFunType(int node, parser::parseTree &tree){
    LOG("inside funtype")
    std::unique_ptr<types::Type> returnType;
    std::vector<std::unique_ptr<types::Type>> paramTypes;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<SigParams>"){
            paramTypes = convertSigParams(nNode, tree);
        }else if(tree.id[nNode] == "<Type>"){
            returnType = convertType(nNode, tree);
        }
    }
    return std::make_unique<types::FunctionType>(std::move(returnType), std::move(paramTypes));
}

std::unique_ptr<types::Type> ast::convertArrDeclType(int node, parser::parseTree &tree){
    LOG("inside arrdecltype")
    std::unique_ptr<types::Type> ut;
    int vNode;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<AtomType>"){
            ut = convertAtomType(nNode, tree);
        }else if(tree.id[nNode] == "<FunType>"){
            ut = convertFunType(nNode, tree);
        }else if(tree.id[nNode] == "<PtrType>"){
            ut = convertPtrType(nNode, tree);
        }else if(tree.id[nNode] == "<VMats>"){
            vNode = nNode;
        }
    }
    std::stack<std::unique_ptr<ASTNode>> s;
    int curNode = vNode;
    while(tree.adj[curNode].size() > 1){
        s.push(convertVal(tree.adj[tree.adj[curNode][0]][1], tree));
        curNode = tree.adj[curNode][1];
    }
    std::unique_ptr<types::ArrayDeclType> ptr = std::make_unique<types::ArrayDeclType>(std::move(ut), convertVal(tree.adj[tree.adj[curNode][0]][1], tree));
    while(!s.empty()){
        auto& topref = s.top();
        s.pop();
        ptr = std::make_unique<types::ArrayDeclType>(std::move(ptr), std::move(topref));
    }
    return ptr;
}

std::unique_ptr<types::Type> ast::convertDeclType(int node, parser::parseTree &tree){
    LOG("inside decltype")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<AtomType>"){
            return convertAtomType(nNode, tree);
        }else if(tree.id[nNode] == "<FunType>"){
            return convertFunType(nNode, tree);
        }else if(tree.id[nNode] == "<ArrDeclType>"){
            return convertArrDeclType(nNode, tree);
        }else if(tree.id[nNode] == "<PtrType>"){
            return convertPtrType(nNode, tree);
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertBinaryOpHelper(int node, parser::parseTree &tree){
    LOG("inside binary op helper")
    if(tree.id[node] == "<Shift>" || tree.id[node] == "<Less>" || tree.id[node] == "<Great>"){
        return convertBinaryOpHelper(tree.adj[node][0], tree);
    }
    std::string opNames[] = {"<LShift>", "<RShift>", "<Xand>", "<Xor>", "<And>", "<Or>", "<Mod>", "<Pow>", "<Sub>", "<Div>", "<Add>", "<Mul>", "<Eq>", "<Neq>", "<Lesser>", "<LesserEqual>", "<Greater>", "<GreaterEqual>"};
    OperatorType opCodes[] = {LSHIFT_OP, RSHIFT_OP, XAND_OP, XOR_OP, AND_OP, OR_OP, MOD_OP, POW_OP, SUB_OP, DIV_OP, ADD_OP, MUL_OP, EQ_OP, NEQ_OP, LT_OP, LEQ_OP, GT_OP, GEQ_OP};
    for(size_t i = 0;i < sizeof(opCodes)/sizeof(opCodes[0]);i++){
        if(opNames[i] == tree.id[node]){
            std::unique_ptr<ASTNode> lc = nullptr,rc;
            for(int nNode: tree.adj[node]){
                if(tree.id[nNode] == "<UnAmb>"){
                    if(lc == nullptr){
                        lc = std::move(convertUnAmb(nNode, tree));
                    }else{
                        rc = std::move(convertUnAmb(nNode, tree));
                    }
                }else if(tree.id[nNode][0] == '<'){
                    if(lc == nullptr){
                        lc = std::move(convertBinaryOpHelper(nNode, tree));
                    }else{
                        rc = std::move(convertBinaryOpHelper(nNode, tree));
                    }
                }
            }
            return std::make_unique<BinaryOperator>(opCodes[i], std::move(lc), std::move(rc));
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertChain(int node, parser::parseTree &tree){
    LOG("about to throw error")
    throw std::logic_error("Chaining isn't implemented yet");
    /*LOG("inside chain")
    std::vector<std::unique_ptr<ASTNode>> terms;
    std::vector<OperatorType> ops;
    int curNode = node;
    while(true){
        int tNode = tree.adj[curNode][0];
        terms.push_back(convertUnAmb(tree.adj[tNode][0], tree));
        if(tree.id[tNode] == "<Lesser>"){
            ops.push_back(LT_OP);
        }else if(tree.id[tNode] == "<LesserEqual>"){
            ops.push_back(LEQ_OP);
        }else if(tree.id[tNode] == "<Greater>"){
            ops.push_back(GT_OP);
        }else if(tree.id[tNode] == "<GreaterEqual>"){
            ops.push_back(GEQ_OP);
        }
        if(tree.id[tree.adj[tNode][2]] == "<UnAmb>"){
            terms.push_back(convertUnAmb(tree.adj[tNode][2], tree));
            break;
        }else{
            curNode = tree.adj[tNode][2];
        }
    }
    auto lc = std::make_unique<BinaryOperator>(ops[0], terms[0], terms[1]);
    for(size_t i = 2;i < terms.size();i++){
        auto rc = std::make_unique<BinaryOperator>(ops[i-1], terms[i-1], terms[i]);
        lc = std::make_unique<BinaryOperator>(AND_OP, std::move(lc), std::move(rc));
    }
    return lc;*/
}

std::unique_ptr<ast::ASTNode> ast::convertBinaryOp(int node, parser::parseTree &tree){
    LOG("inside binary op")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<ArrAcc>"){
            return convertArrAcc(nNode, tree);
        }else if(tree.id[nNode] == "<Less>" || tree.id[nNode] == "<Great>"){
            return convertBinaryOpHelper(nNode, tree);
            //return convertChain(nNode, tree);
        }else{
            return convertBinaryOpHelper(nNode, tree);
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertExp(int node, parser::parseTree &tree){
    LOG("inside exp")
    for(int nNode: tree.adj[node]){
        std::string name = tree.id[nNode];
        if(name == "<UnAmb>"){
            return convertUnAmb(nNode, tree);
        }else if(name == "<UOp>"){
            return convertUnaryOp(nNode, tree);
        }else if(name == "<BinOp>"){
            return convertBinaryOp(nNode, tree);
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertArr(int node, parser::parseTree &tree){
    LOG("inside arr")
    int aNode;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Args>"){
            aNode = nNode;
        }
    }
    auto ptr = std::make_unique<ArrValue>();
    int curNode = aNode;
    while(tree.adj[curNode].size() > 1){
        ptr->addElem(convertVal(tree.adj[curNode][0], tree));
        curNode = tree.adj[curNode][2];
    }
    ptr->addElem(convertVal(tree.adj[curNode][0], tree));
    return ptr;
}

std::unique_ptr<ast::ASTNode> ast::convertArrAcc(int node, parser::parseTree &tree){
    LOG("inside arracc")
    std::unique_ptr<ASTNode> ptr;
    int vNode;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Arr>"){
            ptr = convertArr(nNode, tree);
        }else if(tree.id[nNode] == "IDEN"){
            ptr = std::make_unique<Identifier>(tree.val[nNode]);
        }else if(tree.id[nNode] == "<Exp>"){
            ptr = convertExp(nNode, tree);
        }else if(tree.id[nNode] == "<VMats>"){
            vNode = nNode;
        }
    }
    int curNode = vNode;
    while(tree.adj[curNode].size() > 1){
        ptr = std::make_unique<BinaryOperator>(ARRACC_OP, std::move(ptr), convertVal(tree.adj[tree.adj[curNode][0]][1], tree));
        curNode = tree.adj[curNode][1];
    }
    return std::make_unique<BinaryOperator>(ARRACC_OP, std::move(ptr), convertVal(tree.adj[tree.adj[curNode][0]][1], tree));
}

std::unique_ptr<ast::ASTNode> ast::convertPtrDeref(int node, parser::parseTree &tree){
    LOG("inside ptrderef")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<UnAmb>") {
            return std::make_unique<UnaryOperator>(PTRDEREF_OP, convertUnAmb(nNode, tree));
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertLoc(int node, parser::parseTree &tree){
    LOG("inside loc")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<ArrAcc>") {
            return convertArrAcc(nNode, tree);
        }else if(tree.id[nNode] == "IDEN"){
            return std::make_unique<Identifier>(tree.val[nNode]);
        }else if(tree.id[nNode] == "<PtrDeref>"){
            return convertPtrDeref(nNode, tree);
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertAssn(int node, parser::parseTree &tree){
    LOG("inside assn")
    std::unique_ptr<ASTNode> var,val;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Loc>") {
            var = convertLoc(nNode, tree);
        }else if(tree.id[nNode] == "<Val>"){
            val = convertVal(nNode, tree);
        }
    }
    return std::make_unique<Assign>(std::move(var), std::move(val));
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

std::unique_ptr<ast::ASTNode> ast::convertConstDecl(int node, parser::parseTree &tree){
    LOG("inside constdecl")
    std::unique_ptr<ASTNode> idenPtr, valPtr;
    std::unique_ptr<types::Type> type;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<DeclType>"){
            type = convertDeclType(nNode, tree);
        }else if(tree.id[nNode] == "IDEN"){
            idenPtr = std::make_unique<Identifier>(tree.val[nNode]);
        }else if(tree.id[nNode] == "<Val>"){
            valPtr = convertVal(nNode, tree);
        }
    }
    return std::make_unique<LetConst>(std::move(idenPtr), std::move(type), std::move(valPtr));
}

std::unique_ptr<ast::ASTNode> ast::convertVarDecl(int node, parser::parseTree &tree){
    LOG("inside vardecl")
    std::unique_ptr<ASTNode> idenPtr, valPtr=nullptr;
    std::unique_ptr<types::Type> type;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<DeclType>"){
            type = convertDeclType(nNode, tree);
        }else if(tree.id[nNode] == "IDEN"){
            idenPtr = std::make_unique<Identifier>(tree.val[nNode]);
        }else if(tree.id[nNode] == "<Val>"){
            valPtr = convertVal(nNode, tree);
        }
    }
    if(valPtr == nullptr){
        return std::make_unique<LetVar>(std::move(idenPtr), std::move(type));
    }else{
        return std::make_unique<LetVar>(std::move(idenPtr), std::move(type), std::move(valPtr));
    }
}

std::pair<std::unique_ptr<types::Type>, std::unique_ptr<ast::ASTNode>> ast::convertDeclParam(int node, parser::parseTree &tree){
    LOG("inside param")
    std::unique_ptr<types::Type> typ;
    std::string iden;
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Type>"){
            LOG("# decl param type detected")
            typ = convertType(nNode, tree);
            LOG(typ->name())
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
        if(tree.id[nNode] == "<Type>"){
            retType = convertType(nNode, tree);
        } 
        else if(tree.id[nNode] == "IDEN"){
            iden = tree.val[nNode];
        } 
        else if(tree.id[nNode] == "<DeclParams>"){
            paramNode = nNode;
        }
        else if(tree.id[nNode] == "<Block>"){
            body = convertBlock(nNode, tree);
        }
    }
    std::unique_ptr<LetFun> ptr = std::make_unique<LetFun>(std::move(retType), std::make_unique<Identifier>(iden), std::move(body));
    if(paramNode == -1){
        return ptr;
    }
    int curNode = paramNode;
    while(tree.adj[curNode].size() > 1){
        LOG("inside while at fun decl (trying to parse params)")
        auto d = convertDeclParam(tree.adj[curNode][0], tree);
        LOG("parsed a param")
        LOG(d.first->name())
        ptr->addParam(std::move(d.first), std::move(d.second));
        LOG("added param")
        curNode = tree.adj[curNode][2];
    }
    LOG("outside while loop")
    auto d = convertDeclParam(tree.adj[curNode][0], tree);
    LOG("another param parsed")
    LOG(d.first->name())
    ptr->addParam(std::move(d.first), std::move(d.second));
    LOG("another param added")
    return ptr;
}

std::unique_ptr<ast::ASTNode> ast::convertExpStmt(int node, parser::parseTree &tree){
    LOG("inside exp stmt")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Val>"){
            return convertVal(nNode, tree);
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

std::unique_ptr<ast::ASTNode> ast::convertLoop(int node, parser::parseTree &tree){
    LOG("inside convert loop")
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
            return convertExpStmt(nNode, tree);
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

std::unique_ptr<ast::ASTNode> ast::convertDecl(int node, parser::parseTree &tree){
    LOG("inside decl")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<ConstDecl>"){
            LOG("constdecl")
            return convertConstDecl(nNode, tree);
        }else if(tree.id[nNode] == "<VarDecl>"){
            LOG("var decl")
            return convertVarDecl(nNode, tree);
        }else if(tree.id[nNode] == "<Stmt>"){
            LOG("stmt decl")
            return convertStmt(nNode, tree);
        }else if(tree.id[nNode] == "<FunDecl>"){
            LOG("fun decl")
            return convertFunDecl(nNode, tree);
        }
    }
}

std::unique_ptr<ast::ASTNode> ast::convertBlock(int node, parser::parseTree &tree){
    LOG("inside block")
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Decls>"){
            std::unique_ptr<Block> root = std::make_unique<Block>();
            int curNode = nNode;
            while(tree.adj[curNode].size() > 1){
                LOG("branched at block")
                auto d = convertDecl(tree.adj[curNode][0], tree);
                root->addDecl(std::move(d));
                curNode = tree.adj[curNode][1];
            }
            LOG("out of loop at block")
            auto d = convertDecl(tree.adj[curNode][0], tree);
            LOG("decl done at block")
            root->addDecl(std::move(d));
            LOG("decl added at block")
            return root;
        }
    }
    return std::make_unique<NullValue>();
}

std::unique_ptr<ast::ASTNode> ast::convertProg(int node, parser::parseTree &tree){
    for(int nNode: tree.adj[node]){
        if(tree.id[nNode] == "<Decls>"){
            std::unique_ptr<Prog> root = std::make_unique<Prog>();
            int curNode = nNode;
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
    }
}

std::unique_ptr<ast::ASTNode> ast::parseTreeToAST(parser::parseTree &tree) {
    return convertProg(0, tree);
}