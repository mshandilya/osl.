#include "lexer.hpp"
#include "parser.hpp"

void vizTree(const parser::parseTree &tree, int node, const std::string &prefix = "", bool isLast = true) {
    std::cout << prefix;
    if(!prefix.empty()){
        std::cout << "+-";
    }
    std::cout << tree.id[node];
    if(isValToken(tree.id[node])){
        std::cout << ": " << tree.val[node];
    }
    std::cout << "\n";
    std::string newPrefix;
    if(prefix.empty()){
        newPrefix = "   ";
    }else{
        if(isLast)
            newPrefix = prefix + "   ";
        else
            newPrefix = prefix + "| ";
    }
    for(int i = 0;i < tree.adj[node].size();i++){
        vizTree(tree, tree.adj[node][i], newPrefix, i == tree.adj[node].size()-1);
    }
}

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
        vizTree(tree, 0);
    }
    return 0;
}