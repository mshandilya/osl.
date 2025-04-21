#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "resolver.hpp"

#define LOG(x) std::cout<<x<<std::endl;

int main(int argc, char* argv[]){
    LOG("starting")
    lexer::genLexer lexer("TOKENS");
    auto lex = lexer.getLexer();
    if(argc < 2){
        std::cout << "Usage: cosl <osl-program>" << std::endl;
        return 1;
    }
    std::vector<Token> tokens = lex(argv[1]);
    LOG("Tokenized")
    //std::cout << "Lexer Result:\n";
    //std::string isValToken[] = {"IDEN","NUM","BOOL"};
    for(Token token: tokens){
        std::cout << token << std::endl;
        if(isValToken(token.getId())){
            std::cout << "Value: " << token.getVal() << std::endl;
        }
    }
    std::cout << "Tokenization Done Successfully" << std::endl;
    LOG("")
    LOG("starting parse")
    parser::genParser parser("GRAMMAR", lexer.allTokens, lexer.dfa.desc, lexer.dfa.lines);
    LOG("obj created")
    auto parse = parser.getParser();
    LOG("parser generated")
    parser::parseTree tree = parse(tokens);
    LOG("parsed")
    std::cout << "Parsing Done Successfully" << std::endl;
    //std::cout << "\nParser Result:\n";
    if(tree.getNodeCnt() != 0){
        parser::vizTree(tree, 0, "", true);
    }
    std::unique_ptr<ast::ASTNode> root = ast::parseTreeToAST(tree);
    std::cout << "AST Tree Formed Successfully" << std::endl;
    LOG("ast formed")
    ast::vizTree(root, "", true);
    LOG("about to start resolution")
    Resolver resolver(root);
    std::cout << "Resolution Performed Successfully" << std::endl;
    LOG("resolution done successfully")
    return 0;
}