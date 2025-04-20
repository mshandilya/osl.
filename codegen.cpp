#include "codegen.hpp"

void CodeGenerator::codifyNext(std::unique_ptr<ast::ASTNode>& node) {
    switch(node->type()) {
        case ast::PROG_AST:
            return codifyProg(node);
        case ast::BLOCK_AST:
            return codifyBlock(node);
        case ast::LOOP_AST:
            return codifyLoop(node);
        case ast::COND_AST:
            return codifyCond(node);
        case ast::RET_AST:
            return codifyReturn(node);
        case ast::LOG_AST:
            return codifyLog(node);
        case ast::LETFUN_AST:
            return codifyLetFun(node);
        case ast::LETVAR_AST:
            return codifyLetVar(node);
        case ast::LETCONST_AST:
            return codifyLetConst(node);
        case ast::VAL_AST:
            return codifyValue(node);
        case ast::ASSIGN_AST:
            return codifyAssign(node);
        case ast::LOC_AST:
            return codifyLocation(node);
        case ast::BOP_AST:
            return codifyBinOp(node);
        case ast::UOP_AST:
            return codifyUnOp(node);
        case ast::FUNCALL_AST:
            return codifyFunCall(node);
        case ast::ARR_AST:
            return codifyArray(node);
        case ast::NUM_AST:
            return codifyNumber(node);
        case ast::CHAR_AST:
            return codifyChar(node);
        case ast::BOOL_AST:
            return codifyBool(node);
        case ast::NULL_AST:
            return codifyNull(node);
        case ast::IDEN_AST:
            return codifyIden(node);
    }
}

void CodeGenerator::codifyProg(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::Prog*>(node.get());
}

void CodeGenerator::codifyBlock(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::Block*>(node.get());
}

void CodeGenerator::codifyLoop(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::Loop*>(node.get());
}

void CodeGenerator::codifyCond(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::Conditional*>(node.get());
}

void CodeGenerator::codifyReturn(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::Return*>(node.get());
}

void CodeGenerator::codifyLog(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::Log*>(node.get());
}

void CodeGenerator::codifyLetFun(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::LetFun*>(node.get());
}

void CodeGenerator::codifyLetVar(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::LetVar*>(node.get());
}

void CodeGenerator::codifyLetConst(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::LetConst*>(node.get());
}

// void CodeGenerator::codifyValue(std::unique_ptr<ast::ASTNode>& node) {
//     auto nnode = dynamic_cast<ast::Value*>(node.get());
// }

void CodeGenerator::codifyAssign(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::Assign*>(node.get());
}

// void CodeGenerator::codifyLocation(std::unique_ptr<ast::ASTNode>& node) {
//     auto nnode = dynamic_cast<ast::Location*>(node.get());
// }

void CodeGenerator::codifyBinOp(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::BinaryOperator*>(node.get());
}

void CodeGenerator::codifyUnOp(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::UnaryOperator*>(node.get());
}

void CodeGenerator::codifyFunCall(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::FunctionCall*>(node.get());
}

void CodeGenerator::codifyArray(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::ArrValue*>(node.get());
}

void CodeGenerator::codifyNumber(std::unique_ptr<ast::ASTNode>& node) {
    auto npnode = dynamic_cast<ast::AtomicASTNode*>(node.get());
    // transform further to Number after types design
    
}

void CodeGenerator::codifyChar(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::CharValue*>(node.get());
}

void CodeGenerator::codifyBool(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::BoolValue*>(node.get());
}

void CodeGenerator::codifyNull(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::NullValue*>(node.get());
}

void CodeGenerator::codifyIden(std::unique_ptr<ast::ASTNode>& node) {
    auto nnode = dynamic_cast<ast::Identifier*>(node.get());
}