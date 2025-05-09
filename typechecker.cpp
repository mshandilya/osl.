#include "typechecker.hpp"

namespace typecheck {

    std::unique_ptr<types::Type> defaultRetType = nullptr;

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckNext(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        // std::cout<<"Deciding next typecheck"<<std::endl;
        switch(root->type()) {
            case ast::PROG_AST:
                return typecheckProg(std::move(root), expectedReturnType);
            case ast::BLOCK_AST:
                return typecheckBlock(std::move(root), expectedReturnType);
            case ast::LOOP_AST:
                return typecheckLoop(std::move(root), expectedReturnType);
            case ast::COND_AST:
                return typecheckCond(std::move(root), expectedReturnType);
            case ast::RET_AST:
                return typecheckReturn(std::move(root), expectedReturnType);
            case ast::LOG_AST:
                // log's value cannot have a return statement
                return typecheckLog(std::move(root));
            case ast::LETFUN_AST:
                // return statement after this would link to another function
                return typecheckLetFun(std::move(root));
            case ast::LETARR_AST:
                // let array won't have a return statement
                throw std::logic_error("Let Array should never have reached in typecheckNext.");
            case ast::LETVAR_AST:
                // let var won't have a return statement
                return typecheckLetVar(std::move(root));
            case ast::LETCONST_AST:
                // let const won't have a return statement
                return typecheckLetConst(std::move(root));
            case ast::ASSIGN_AST:
                // let assign won't have a return statement
                return typecheckAssign(std::move(root));
            case ast::BOP_AST:
                return typecheckBinOp(std::move(root));
            case ast::UOP_AST:
                return typecheckUnOp(std::move(root));
            case ast::FUNCALL_AST:
                return typecheckFunCall(std::move(root));
            case ast::ARR_AST:
                return typecheckArray(std::move(root));
            case ast::NUM_AST:
                return typecheckNum(std::move(root));
            case ast::CHAR_AST:
                return typecheckChar(std::move(root));
            case ast::BOOL_AST:
                return typecheckBool(std::move(root));
            case ast::NULL_AST:
                return typecheckNull(std::move(root));
            case ast::IDEN_AST:
                return typecheckIden(std::move(root));
        }
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckProg(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::Prog*>(root.get());
        bool satisfied = false;
        std::vector<std::unique_ptr<codetree::CodeTreeNode>> children;
        for(auto&& decl : node->decls) {
            if(decl->type() == ast::LETARR_AST) {
                auto outDecls = typecheckLetArr(std::move(decl), expectedReturnType);
                if(outDecls.size() > 1) {
                    auto& decl1 = children.back();
                    children.pop_back();
                    auto& decl2 = children.back();
                    children.pop_back();
                    children.push_back(std::move(decl1));
                    children.push_back(std::move(decl2));
                }
                else {
                    auto& decl1 = children.back();
                    children.pop_back();
                    children.push_back(std::move(decl1));
                }
            }
            else {
                children.push_back(typecheckNext(std::move(decl), expectedReturnType));
            }
            if(expectedReturnType != nullptr and children.back()->resultingType != nullptr) {
                if(!satisfied and (*expectedReturnType) == (*children.back()->resultingType)) {
                    satisfied = true;
                }
            }
        }
        auto ret = std::make_unique<codetree::NnaryCTN>(codetree::PROG_CTN, std::move(children));
        if(expectedReturnType != nullptr and satisfied) {
            ret->setResultingType(types::gtc(*expectedReturnType));
        }
        return ret;
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckBlock(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::Block*>(root.get());
        bool satisfied = false;
        std::vector<std::unique_ptr<codetree::CodeTreeNode>> children;
        if(expectedReturnType == nullptr)
            satisfied = true;
        for(auto&& decl : node->decls) {
            if(decl->type() == ast::LETARR_AST) {
                auto outDecls = typecheckLetArr(std::move(decl), expectedReturnType);
                if(outDecls.size() > 1) {
                    auto& decl1 = children.back();
                    children.pop_back();
                    auto& decl2 = children.back();
                    children.pop_back();
                    children.push_back(std::move(decl1));
                    children.push_back(std::move(decl2));
                }
                else {
                    auto& decl1 = children.back();
                    children.pop_back();
                    children.push_back(std::move(decl1));
                }
            }
            else {
                children.push_back(typecheckNext(std::move(decl), expectedReturnType));
            }
            if(expectedReturnType != nullptr and children.back()->resultingType != nullptr) {
                if(!satisfied and (*expectedReturnType) == (*children.back()->resultingType)) {
                    satisfied = true;
                }
            }
        }
        auto ret = std::make_unique<codetree::NnaryCTN>(codetree::BLOCK_CTN, std::move(children));
        if(expectedReturnType != nullptr and satisfied) {
            ret->setResultingType(types::gtc(*expectedReturnType));
        }
        return ret;
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckLoop(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::Loop*>(root.get());
        // condition can't have a return statement
        auto lc = typecheckNext(std::move(node->cond));
        if(lc->resultingType->name() == types::ATOM) {
            auto lcat = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
            if(lcat->atomicName() != types::BOOL or lcat->atomicName() != types::ANY) {
                std::cout << "code.osl: \033[31mTypeError\033[0m: Condition to a while loop must resolve to a `bool` type." << std::endl;
                exit(0);
            }
        }
        else {
            std::cout << "code.osl: \033[31mTypeError\033[0m: Condition to a while loop must resolve to a `bool` type." << std::endl;
            exit(0);
        }
        auto rc = typecheckNext(std::move(node->body), expectedReturnType);
        auto ret = std::make_unique<codetree::BinaryCTN>(codetree::LOOP_CTN, std::move(lc), std::move(rc));
        if(expectedReturnType != nullptr and rc->resultingType != nullptr) {
            ret->setResultingType(types::gtc(*expectedReturnType));
        }
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckCond(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::Conditional*>(root.get());
        // condition can't have a return statement
        auto lc = typecheckNext(std::move(node->cond));
        if(lc->resultingType->name() == types::ATOM) {
            auto lcat = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
            if(lcat->atomicName() != types::BOOL or lcat->atomicName() != types::ANY) {
                std::cout << "code.osl: \033[31mTypeError\033[0m: Condition to an if statement must resolve to a `bool` type." << std::endl;
                exit(0);
            }
        }
        else {
            std::cout << "code.osl: \033[31mTypeError\033[0m: Condition to an if statement must resolve to a `bool` type." << std::endl;
            exit(0);
        }
        auto mc = typecheckNext(std::move(node->thenBody), expectedReturnType);
        if(node->elseBody != nullptr) {
            auto rc = typecheckNext(std::move(node->elseBody), expectedReturnType);
            auto ret = std::make_unique<codetree::TernaryCTN>(codetree::COND_CTN, std::move(lc), std::move(mc), std::move(rc));
            if(expectedReturnType != nullptr and (mc->resultingType != nullptr or rc->resultingType != nullptr)) {
                ret->setResultingType(types::gtc(*expectedReturnType));
            }
            return ret;
        }
        else {
            auto ret = std::make_unique<codetree::BinaryCTN>(codetree::PCOND_CTN, std::move(lc), std::move(mc));
            if(expectedReturnType != nullptr and mc->resultingType != nullptr) {
                ret->setResultingType(types::gtc(*expectedReturnType));
            }
            return ret;
        }
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckReturn(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::Return*>(root.get());
        auto child = typecheckNext(std::move(node->val));
        
        // child's type must be that of the function's return type
        if(!(*expectedReturnType == *(child->resultingType))) {
            std::cout << "code.osl: \033[31mTypeError\033[0m: Return type of the function was expected to be something else." << std::endl;
            exit(0);
        }

        auto ret = std::make_unique<codetree::UnaryCTN>(codetree::RET_CTN, std::move(child));

        ret->setResultingType(types::gtc(*expectedReturnType));

        return ret;
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckLog(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::Log*>(root.get());
        auto child = typecheckNext(std::move(node->val));
        
        if(child->resultingType->name() != types::ATOM) {
            std::cout << "code.osl: \033[31mTypeError\033[0m: Only atomic types are allowed to be logged." << std::endl;
            exit(0);
        }

        return std::make_unique<codetree::UnaryCTN>(codetree::LOG_CTN, std::move(child));
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckLetFun(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        // make the function object first.
        // then bind it to the id of the identifier.
        // function object is essentially just the params (their types, what they bind to, and finally the body)
        auto node = dynamic_cast<ast::LetFun*>(root.get());
        std::vector<std::unique_ptr<codetree::CodeTreeNode>> children;
        for(auto& param : node->params) {
            // param.second already has the type associated with it
            if(param.second->type() != ast::IDEN_AST) {
                std::cout << "code.osl: \033[31mTypeError\033[0m: Parameters to a function declaration must be recognized by valid identifiers." << std::endl;
                exit(0);
            }
            auto parIden = dynamic_cast<ast::Identifier*>(param.second.get());
            auto parCTN = std::make_unique<codetree::IdenCTN>(parIden->id, parIden->scopeId, ast::VAR);
            parCTN->setResultingType(std::move(parIden->boundDataType));
            children.push_back(std::move(parCTN));
        }
        children.push_back(typecheckNext(std::move(node->body), node->retType));

        if(expectedReturnType != nullptr and !(*expectedReturnType == types::Null()) and (children.back()->resultingType == nullptr or !(*(children.back()->resultingType) == *expectedReturnType))) {
            std::cout << "code.osl: \033[31mTypeError\033[0m: The body of a function returning a non-nav value must contain a return statement." << std::endl;
            exit(0);
        }
        auto fn = std::make_unique<codetree::NnaryCTN>(codetree::MKFUN_CTN, std::move(children));
        if(node->name->type() == ast::IDEN_AST) {
            auto iden = typecheckIden(std::move(node->name), expectedReturnType);
            return std::make_unique<codetree::BinaryCTN>(codetree::BIND_CTN, std::move(fn), std::move(iden));
        }
        else {
            std::cout << "code.osl: \033[31mTypeError\033[0m: A function declaration must must bind to a valid identifier." << std::endl;
            exit(0);
        }
    }

    std::vector<std::unique_ptr<codetree::CodeTreeNode>> TypeChecker::typecheckLetArr(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        // first, resolve each size (from the array declare type)
        // second, make array object (with given dimensions)
        // third, bind array object to an identifier
        // fourth, set the identifier to the value (if present)
        auto node = dynamic_cast<ast::LetArray*>(root.get());
        std::vector<std::unique_ptr<codetree::CodeTreeNode>> ret;
        auto declArrObj = typecheckDeclType(std::move(node->myType)); // DMK_ARR
        if(node->name->type() == ast::IDEN_AST) {
            auto iden2 = typecheckIden(std::make_unique<ast::Identifier>(*(dynamic_cast<ast::Identifier*>(node->name.get()))), expectedReturnType);
            auto iden = typecheckIden(std::move(node->name), expectedReturnType);
            ret.push_back(std::make_unique<codetree::BinaryCTN>(codetree::BIND_CTN, std::move(declArrObj), std::move(iden)));
            if(node->val->type() != ast::NULL_AST) {
                auto val = typecheckNext(std::move(node->val));
                if(*(val->resultingType) == types::Null() and *(val->resultingType) == *(iden2->resultingType))
                    ret.push_back(std::make_unique<codetree::BinaryCTN>(codetree::SET_CTN, std::move(iden2), std::move(val)));
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: The value assigned to an identifier must be of the same/compatible type as the identifier." << std::endl;
                    exit(0);
                }
            }
        }
        else {
            std::cout << "code.osl: \033[31mTypeError\033[0m: An array must must bind to a valid identifier." << std::endl;
            exit(0);
        }
        return ret;
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckLetVar(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::LetVar*>(root.get());
        auto val = typecheckNext(std::move(node->val), expectedReturnType);
        auto var = typecheckNext(std::move(node->var), expectedReturnType);
        if(*(val->resultingType) == types::Null() or *(val->resultingType) == *(var->resultingType)) {
            return std::make_unique<codetree::BinaryCTN>(codetree::BIND_CTN, std::move(val), std::move(var));
        }
        else {
            std::cout << "code.osl: \033[31mTypeError\033[0m: The value assigned to an identifier must be of the same/compatible type as the identifier." << std::endl;
            exit(0);
        }
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckLetConst(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::LetConst*>(root.get());
        auto val = typecheckNext(std::move(node->val), expectedReturnType);
        auto var = typecheckNext(std::move(node->var), expectedReturnType);
        if(*(val->resultingType) == types::Null() or *(val->resultingType) == *(var->resultingType)) {
            return std::make_unique<codetree::BinaryCTN>(codetree::BIND_CTN, std::move(val), std::move(var));
        }
        else {
            std::cout << "code.osl: \033[31mTypeError\033[0m: The value assigned to an identifier must be of the same/compatible type as the identifier." << std::endl;
            exit(0);
        }
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckAssign(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::Assign*>(root.get());
        auto val = typecheckNext(std::move(node->val), expectedReturnType);
        auto var = typecheckNext(std::move(node->var), expectedReturnType);
        if(var->isLocation) {
            if(*(val->resultingType) == types::Null() or *(var->resultingType) == *(val->resultingType)) {
                auto t = types::gtc(*(var->resultingType));
                auto nnode = std::make_unique<codetree::BinaryCTN>(codetree::SETASS_CTN, std::move(val), std::move(var));
                nnode->setResultingType(std::move(t));
            }
        }
        else {
            std::cout << "code.osl: \033[31mTypeError\033[0m: The left hand operator to the := operator must be an assignable value." << std::endl;
            exit(0);
        }
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckBinOp(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        // std::cout<<"Entering Binary Operation"<<std::endl;
        auto node = dynamic_cast<ast::BinaryOperator*>(root.get());
        switch(node->op) {
            case ast::GT_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values." << std::endl;
                            exit(0);
                    }
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::RGT_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::make_unique<types::Boolean>());
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::GEQ_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values." << std::endl;
                            exit(0);
                    }
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::RGEQ_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::make_unique<types::Boolean>());
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::LT_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values." << std::endl;
                            exit(0);
                    }
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::RLT_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::make_unique<types::Boolean>());
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::LEQ_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values." << std::endl;
                            exit(0);
                    }
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::RLEQ_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::make_unique<types::Boolean>());
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::NEQ_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(*(lc->resultingType) == *(rc->resultingType)) {
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::RNEQ_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::make_unique<types::Boolean>());
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::EQ_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(*(lc->resultingType) == *(rc->resultingType)) {
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::REQ_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::make_unique<types::Boolean>());
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary relational comparator may only be applied to numerical values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::MUL_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                            exit(0);
                    }
                    auto retType = types::gtc(*(lc->resultingType));
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::AMUL_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::move(retType));
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::ADD_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                // std::cout<<"Typechecked children for addition"<<std::endl;
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            // std::cout<<"\tLeft type deduced as u32"<<std::endl;
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    // std::cout<<"\tRight type deduced as u32 as well"<<std::endl;
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                            exit(0);
                    }
                    auto retType = types::gtc(*(lc->resultingType));
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::AADD_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::move(retType));
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::DIV_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                            exit(0);
                    }
                    auto retType = types::gtc(*(lc->resultingType));
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::ADIV_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::move(retType));
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::SUB_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                            exit(0);
                    }
                    auto retType = types::gtc(*(lc->resultingType));
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::ASUB_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::move(retType));
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::POW_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                            exit(0);
                    }
                    auto retType = types::gtc(*(lc->resultingType));
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::APOW_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::move(retType));
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::MOD_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values of same/compatible types." << std::endl;
                            exit(0);
                    }
                    auto retType = types::gtc(*(lc->resultingType));
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::AMOD_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::move(retType));
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary arithmetic operator may only be applied to numerical values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::OR_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::BOOL: {
                            switch(atr->atomicName()) {
                                case types::BOOL: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeMismatch\033[0m: Logical operator may only be applied to boolean values." << std::endl;
                                    exit(0);
                                }
                            }
                            auto retType = types::gtc(*(lc->resultingType));
                            auto ret = std::make_unique<codetree::BinaryCTN>(codetree::LOR_CTN, std::move(lc), std::move(rc));
                            ret->setResultingType(std::move(retType));
                            return ret;
                            break;
                        }
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Overloaded OR operator may only be applied to numerical values of same/compatible types or to boolean values." << std::endl;
                            exit(0);
                    }
                    auto retType = types::gtc(*(lc->resultingType));
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::BOR_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::move(retType));
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Overloaded OR operator may only be applied to numerical values of same/compatible types or to boolean values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::AND_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::BOOL: {
                            switch(atr->atomicName()) {
                                case types::BOOL: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeMismatch\033[0m: Logical operator may only be applied to boolean values." << std::endl;
                                    exit(0);
                                }
                            }
                            auto retType = types::gtc(*(lc->resultingType));
                            auto ret = std::make_unique<codetree::BinaryCTN>(codetree::LAND_CTN, std::move(lc), std::move(rc));
                            ret->setResultingType(std::move(retType));
                            return ret;
                            break;
                        }
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Overloaded AND operator may only be applied to numerical values of same/compatible types or to boolean values." << std::endl;
                            exit(0);
                    }
                    auto retType = types::gtc(*(lc->resultingType));
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::BAND_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::move(retType));
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Overloaded AND operator may only be applied to numerical values of same/compatible types or to boolean values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::XOR_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::BOOL: {
                            switch(atr->atomicName()) {
                                case types::BOOL: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeMismatch\033[0m: Logical operator may only be applied to boolean values." << std::endl;
                                    exit(0);
                                }
                            }
                            auto retType = types::gtc(*(lc->resultingType));
                            auto ret = std::make_unique<codetree::BinaryCTN>(codetree::LXOR_CTN, std::move(lc), std::move(rc));
                            ret->setResultingType(std::move(retType));
                            return ret;
                            break;
                        }
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Overloaded XOR operator may only be applied to numerical values of same/compatible types or to boolean values." << std::endl;
                            exit(0);
                    }
                    auto retType = types::gtc(*(lc->resultingType));
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::BXOR_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::move(retType));
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Overloaded XOR operator may only be applied to numerical values of same/compatible types or to boolean values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::XAND_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::BOOL: {
                            switch(atr->atomicName()) {
                                case types::BOOL: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeMismatch\033[0m: Logical operator may only be applied to boolean values." << std::endl;
                                    exit(0);
                                }
                            }
                            auto retType = types::gtc(*(lc->resultingType));
                            auto ret = std::make_unique<codetree::BinaryCTN>(codetree::LXAND_CTN, std::move(lc), std::move(rc));
                            ret->setResultingType(std::move(retType));
                            return ret;
                            break;
                        }
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Bitwise operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Overloaded XAND operator may only be applied to numerical values of same/compatible types or to boolean values." << std::endl;
                            exit(0);
                    }
                    auto retType = types::gtc(*(lc->resultingType));
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::BXAND_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::move(retType));
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Overloaded XAND operator may only be applied to numerical values of same/compatible types or to boolean values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::LSHIFT_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                            exit(0);
                    }
                    auto retType = types::gtc(*(lc->resultingType));
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::BLSHFT_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::move(retType));
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::RSHIFT_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ATOM and rc->resultingType->name() == types::ATOM) {
                    auto atl = dynamic_cast<types::AtomicType*>(lc->resultingType.get());
                    auto atr = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                    switch(atl->atomicName()) {
                        case types::NUM: {
                            switch(atr->atomicName()) {
                                case types::NUM: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT: {
                            switch(atr->atomicName()) {
                                case types::INT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_8: {
                            switch(atr->atomicName()) {
                                case types::INT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_16: {
                            switch(atr->atomicName()) {
                                case types::INT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_32: {
                            switch(atr->atomicName()) {
                                case types::INT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_64: {
                            switch(atr->atomicName()) {
                                case types::INT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::INT_128: {
                            switch(atr->atomicName()) {
                                case types::INT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT: {
                            switch(atr->atomicName()) {
                                case types::UINT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_8: {
                            switch(atr->atomicName()) {
                                case types::UINT_8: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_16: {
                            switch(atr->atomicName()) {
                                case types::UINT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_32: {
                            switch(atr->atomicName()) {
                                case types::UINT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_64: {
                            switch(atr->atomicName()) {
                                case types::UINT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::UINT_128: {
                            switch(atr->atomicName()) {
                                case types::UINT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT: {
                            switch(atr->atomicName()) {
                                case types::FLOAT: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_16: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_16: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_32: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_32: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_64: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_64: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        case types::FLOAT_128: {
                            switch(atr->atomicName()) {
                                case types::FLOAT_128: {
                                    break;
                                }
                                default: {
                                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                                    exit(0);
                                }
                            }
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values of same/compatible types." << std::endl;
                            exit(0);
                    }
                    auto retType = types::gtc(*(lc->resultingType));
                    auto ret = std::make_unique<codetree::BinaryCTN>(codetree::BRSHFT_CTN, std::move(lc), std::move(rc));
                    ret->setResultingType(std::move(retType));
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Binary bitwise shift operator may only be applied to numerical values." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::ARRACC_OP: {
                auto lc = typecheckNext(std::move(node->leftChild), expectedReturnType);
                auto rc = typecheckNext(std::move(node->rightChild), expectedReturnType);
                if(lc->resultingType->name() == types::ARR) {
                    auto retType = types::gtc(*(dynamic_cast<types::ArrayType*>(lc->resultingType.get())->underlyingType));
                    if(rc->resultingType->name() == types::ATOM) {
                        auto at = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
                        switch(at->atomicName()) {
                            case types::NUM:
                            case types::INT:
                            case types::INT_8:
                            case types::INT_16:
                            case types::INT_32:
                            case types::INT_64:
                            case types::INT_128:
                            case types::UINT:
                            case types::UINT_8:
                            case types::UINT_16:
                            case types::UINT_32:
                            case types::UINT_64:
                            case types::UINT_128: {
                                auto ret = std::make_unique<codetree::BinaryCTN>(codetree::ARRGET_CTN, std::move(lc), std::move(rc));
                                ret->setResultingType(std::move(retType));
                                return ret;
                                break;
                            }
                            default:
                                std::cout << "code.osl: \033[31mTypeError\033[0m: Array access index must be an integer." << std::endl;
                                exit(0);
                        }
                    }
                    else {
                        std::cout << "code.osl: \033[31mTypeError\033[0m: Array access index must be an integer." << std::endl;
                        exit(0);
                    }
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Array Accesses must occur on valid arrays." << std::endl;
                    exit(0);
                }
                break;
            }
        }
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckUnOp(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        // apply a unary op here
        auto node = dynamic_cast<ast::UnaryOperator*>(root.get());
        switch(node->op) {
            case ast::UNEG_OP: {
                auto child = typecheckNext(std::move(node->child), expectedReturnType);
                if(child->resultingType->name() == types::ATOM) {
                    auto at = dynamic_cast<types::AtomicType*>(child->resultingType.get());
                    switch(at->atomicName()) {
                        case types::NUM:
                        case types::INT:
                        case types::INT_8:
                        case types::INT_16:
                        case types::INT_32:
                        case types::INT_64:
                        case types::INT_128:
                        // case types::UINT:
                        // case types::UINT_8:
                        // case types::UINT_16:
                        // case types::UINT_32:
                        // case types::UINT_64:
                        // case types::UINT_128:
                        case types::FLOAT:
                        case types::FLOAT_16:
                        case types::FLOAT_32:
                        case types::FLOAT_64:
                        case types::FLOAT_128: {
                            auto retType = types::gtc(*(child->resultingType));
                            auto ret = std::make_unique<codetree::UnaryCTN>(codetree::ANEG_CTN, std::move(child));
                            ret->setResultingType(std::move(retType));
                            return ret;
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Unary negation may only be applied to a numerical value." << std::endl;
                            exit(0);
                    }
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Unary negation may only be applied to a numerical value." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::UNOT_OP: {
                auto child = typecheckNext(std::move(node->child), expectedReturnType);
                if(child->resultingType->name() == types::ATOM) {
                    auto at = dynamic_cast<types::AtomicType*>(child->resultingType.get());
                    switch(at->atomicName()) {
                        case types::NUM:
                        case types::INT:
                        case types::INT_8:
                        case types::INT_16:
                        case types::INT_32:
                        case types::INT_64:
                        case types::INT_128:
                        case types::UINT:
                        case types::UINT_8:
                        case types::UINT_16:
                        case types::UINT_32:
                        case types::UINT_64:
                        case types::UINT_128:
                        case types::FLOAT:
                        case types::FLOAT_16:
                        case types::FLOAT_32:
                        case types::FLOAT_64:
                        case types::FLOAT_128: {
                            auto retType = types::gtc(*(child->resultingType));
                            auto ret = std::make_unique<codetree::UnaryCTN>(codetree::BNOT_CTN, std::move(child));
                            ret->setResultingType(std::move(retType));
                            return ret;
                            break;
                        }
                        case types::BOOL: {
                            auto retType = types::gtc(*(child->resultingType));
                            auto ret = std::make_unique<codetree::UnaryCTN>(codetree::LNOT_CTN, std::move(child));
                            ret->setResultingType(std::move(retType));
                            return ret;
                            break;
                        }
                        default:
                            std::cout << "code.osl: \033[31mTypeError\033[0m: Unary not may only be applied to a numerical value or a boolean value." << std::endl;
                            exit(0);
                    }
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Unary not may only be applied to a numerical value or a boolean value." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::PTRREF_OP: {
                auto child = typecheckNext(std::move(node->child), expectedReturnType);
                // only locs may be pointed to
                if(child->isLocation) {
                    auto type = std::make_unique<types::PointerType>(types::gtc(*(child->resultingType)));
                    auto ret = std::make_unique<codetree::UnaryCTN>(codetree::PTRREF_CTN, std::move(child));
                    ret->setResultingType(std::move(type));
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Pointer references may only be created to objects in the memory." << std::endl;
                    exit(0);
                }
                break;
            }
            case ast::PTRDEREF_OP: {
                auto child = typecheckNext(std::move(node->child), expectedReturnType);
                // only locs may be pointed to
                if(child->resultingType->name() == types::PTR) {
                    auto ut = types::gtc(*((dynamic_cast<types::PointerType*>(child->resultingType.get()))->underlyingType));
                    if(ut->name() == types::ATOM and (*dynamic_cast<types::AtomicType*>(ut.get())).atomicName() == types::NULLV)
                        ut = std::make_unique<types::Type>(types::AnyType());
                    auto ret = std::make_unique<codetree::UnaryCTN>(codetree::PTRAT_CTN, std::move(child));
                    ret->setResultingType(std::move(ut));
                    ret->isLocation = true;
                    return ret;
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Only Pointer Types can be dereferenced." << std::endl;
                    exit(0);
                }
                break;
            }
        }
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckFunCall(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::FunctionCall*>(root.get());
        std::vector<std::unique_ptr<codetree::CodeTreeNode>> children;
        auto calling = typecheckNext(std::move(node->name), expectedReturnType);
        if(calling->resultingType->name() == types::FN) {
            auto calltype = dynamic_cast<types::FunctionType*>(calling->resultingType.get());
            children.push_back(std::move(calling));
            for(uint32_t ind = 0; ind < (uint32_t)(node->params.size()); ind++) {
                auto arg = typecheckNext(std::move(node->params[ind]), expectedReturnType);
                if(*(arg->resultingType) == *(calltype->paramTypes[ind])) {
                    children.push_back(std::move(arg));
                }
                else {
                    // check casting here
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Type mismatch for the parameter at position "<<ind<<" passed to the function call." << std::endl;
                    exit(0);
                }
            }
            auto ret = std::make_unique<codetree::NnaryCTN>(codetree::CALL_CTN, std::move(children));
            ret->setResultingType(types::gtc(*(calltype->returnType)));
            return ret;
        }
        else {
            std::cout << "code.osl: \033[31mTypeError\033[0m: Function Calls may only be applied to a function." << std::endl;
            exit(0);
        }
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckArray(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::ArrValue*>(root.get());
        // to-do
        // check that types of each element is same
        // find resulting type based on elem
        // make array and send
        if(node->elems.empty()) {
            // array of null values
            auto ret = std::make_unique<codetree::NnaryCTN>(codetree::MKARR_CTN, std::vector<std::unique_ptr<codetree::CodeTreeNode>>());
            ret->setResultingType(std::make_unique<types::ArrayType>(std::make_unique<types::Null>()));
            return ret;
        } 
        else {
            // array of some defined type
            std::vector<std::unique_ptr<codetree::CodeTreeNode>> children;
            auto fe = typecheckNext(std::move(node->elems[0]), expectedReturnType);
            auto underlyingType = types::gtc(*(fe->resultingType));
            children.push_back(std::move(fe));
            for(uint32_t ind = 1; ind < children.size(); ind++) {
                auto el = typecheckNext(std::move(node->elems[ind]), expectedReturnType);
                if(*underlyingType == types::Null()) {
                    underlyingType = types::gtc(*(el->resultingType));
                }
                if(*(el->resultingType) == types::Null() or *(el->resultingType) == *underlyingType) {
                    children.push_back(std::move(el));
                }
                else {
                    std::cout << "code.osl: \033[31mTypeError\033[0m: Types of elements inside the array are not coherent." << std::endl;
                    exit(0);
                }
            }
            auto ret = std::make_unique<codetree::NnaryCTN>(codetree::MKARR_CTN, std::move(children));
            ret->setResultingType(std::make_unique<types::ArrayType>(std::move(underlyingType)));
        }
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckNum(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::NumValue*>(root.get());
        auto ret = std::make_unique<codetree::AtomCTN>(codetree::NUM_CTN, std::move(node->val));
        ret->setResultingType(types::gtc(*(ret->val)));
        return ret;
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckChar(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        // std::cout<<"Got character type"<<std::endl;
        auto node = dynamic_cast<ast::CharValue*>(root.get());
        auto ret = std::make_unique<codetree::AtomCTN>(codetree::CHAR_CTN, std::move(node->val));
        ret->setResultingType(types::gtc(*(ret->val)));
        // std::cout<<ret->resultingType->name()<<" "<<(dynamic_cast<types::AtomicType*>(ret->resultingType.get()))->atomicName()<<std::endl;
        return ret;
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckBool(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::BoolValue*>(root.get());
        auto ret = std::make_unique<codetree::AtomCTN>(codetree::BOOL_CTN, std::move(node->val));
        ret->setResultingType(types::gtc(*(ret->val)));
        return ret;
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckNull(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::NullValue*>(root.get());
        auto ret = std::make_unique<codetree::AtomCTN>(codetree::NULL_CTN, std::make_unique<types::Null>());
        ret->setResultingType(types::gtc(*(ret->val)));
        return ret;
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckIden(std::unique_ptr<ast::ASTNode>&& root, std::unique_ptr<types::Type>& expectedReturnType) {
        auto node = dynamic_cast<ast::Identifier*>(root.get());
        auto ret = std::make_unique<codetree::IdenCTN>(node->id, node->scopeId, node->access);
        ret->setResultingType(std::move(node->boundDataType));
        ret->isLocation = true;
        return ret;
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckDeclType(std::unique_ptr<types::ArrayDeclType>&& node) {
        // I wish to deduce the size of which I need to make an array
        std::unique_ptr<codetree::CodeTreeNode> lc, rc;
        auto resType = types::gtc(*node);
        if(node->underlyingType->name() == types::ARRD) {
            lc = typecheckDeclType(std::move(node->underlyingType));
        }
        else {
            lc = std::make_unique<codetree::AtomCTN>(codetree::NULL_CTN, std::make_unique<types::Null>());
        }
        rc = typecheckNext(std::move(node->size));
        if(rc->resultingType->name() == types::ATOM) {
            auto temptype = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
            switch(temptype->atomicName()) {
                case types::INT:
                case types::INT_8:
                case types::INT_16:
                case types::INT_32:
                case types::INT_64:
                case types::INT_128:
                case types::UINT:
                case types::UINT_8:
                case types::UINT_16:
                case types::UINT_32:
                case types::UINT_64:
                case types::UINT_128:
                    break;
                default:
                    std::cout << "code.osl: \033[31mTypeError\033[0m: The size of an array must evaluate to a signed or unsigned integer." << std::endl;
                    exit(0);
                    break;
            }
        }
        else {
            std::cout << "code.osl: \033[31mTypeError\033[0m: The size of an array must evaluate to a signed or unsigned integer." << std::endl;
            exit(0);
        }
        return std::make_unique<codetree::BinaryCTN>(codetree::DMKARR_CTN, std::move(lc), std::move(rc));
    }

    std::unique_ptr<codetree::CodeTreeNode> TypeChecker::typecheckDeclType(std::unique_ptr<types::Type>&& node) {
        std::unique_ptr<codetree::CodeTreeNode> lc, rc;
        auto resType = types::gtc(*node);
        auto nnode = dynamic_cast<types::ArrayDeclType*>(node.get());
        if(nnode->underlyingType->name() == types::ARRD) {
            lc = typecheckDeclType(std::move(nnode->underlyingType));
        }
        else {
            lc = std::make_unique<codetree::AtomCTN>(codetree::NULL_CTN, std::make_unique<types::Null>());
        }
        rc = typecheckNext(std::move(nnode->size));
        if(rc->resultingType->name() == types::ATOM) {
            auto temptype = dynamic_cast<types::AtomicType*>(rc->resultingType.get());
            switch(temptype->atomicName()) {
                case types::INT:
                case types::INT_8:
                case types::INT_16:
                case types::INT_32:
                case types::INT_64:
                case types::INT_128:
                case types::UINT:
                case types::UINT_8:
                case types::UINT_16:
                case types::UINT_32:
                case types::UINT_64:
                case types::UINT_128:
                    break;
                default:
                    std::cout << "code.osl: \033[31mTypeError\033[0m: The size of an array must evaluate to a signed or unsigned integer." << std::endl;
                    exit(0);
                    break;
            }
        }
        else {
            std::cout << "code.osl: \033[31mTypeError\033[0m: The size of an array must evaluate to a signed or unsigned integer." << std::endl;
            exit(0);
        }
        return std::make_unique<codetree::BinaryCTN>(codetree::DMKARR_CTN, std::move(lc), std::move(rc));
    }

}