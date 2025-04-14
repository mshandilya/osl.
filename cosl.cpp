#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"

int main(int argc, char* argv[]){
    lexer::genLexer lexer("TOKENS");
    auto lex = lexer.getLexer();
    if(argc < 2){
        std::cout << "Usage: cosl <osl-program>" << std::endl;
        return 1;
    }
    std::vector<Token> tokens = lex(argv[1]);
    /*std::cout << "Lexer Result:\n";
    //std::string isValToken[] = {"IDEN","NUM","BOOL"};
    for(Token token: tokens){
        std::cout << token << std::endl;
        if(isValToken(token.getId())){
            std::cout << "Value: " << token.getVal() << std::endl;
            break;
        }
    }*/
    parser::genParser parser("GRAMMAR", lexer.allTokens, lexer.dfa.desc, lexer.dfa.lines);
    auto parse = parser.getParser();
    parser::parseTree tree = parse(tokens);
    //std::cout << "\nParser Result:\n";
    if(tree.getNodeCnt() != 0){
        parser::vizTree(tree, 0, "", true);
    }
    ast::Prog root = ast::parseTreeToAST(tree);
    ast::vizTree(&root, "", true);
    return 0;
}