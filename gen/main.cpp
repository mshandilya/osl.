#include "lexer.h"
#include "parser.h"

void vizTree(const parser::parseTree &tree, int node, const std::string &prefix = "", bool isLast = true) {
    std::cout << prefix;
    if(!prefix.empty()){
        std::cout << "+-";
    }
    std::cout << tree.id[node];
    if(tree.val[node] != "")
        std::cout << ": " << tree.val[node];
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
    for(int i = 0;i < tree.adj[node].size();i++) {
        vizTree(tree, tree.adj[node][i], newPrefix, i == tree.adj[node].size()-1);
    }
}

int main(){
    /**
        TODO: Change lexer::genLexer's argument from filename to stream of characters 
    */
    lexer::genLexer lexer("lex_rules.txt");
    auto lex = lexer.getLexer();
    std::vector<Token> tokens = lex("input.txt");
    std::cout << "Lexer Result:\n";
    for(Token token: tokens){
        std::cout << token << std::endl;
    }

    parser::genParser parser("parse_rules_test.txt", lexer.allTokens);
    auto parse = parser.getParser();
    parser::parseTree tree = parse(tokens);

    /*for(int i = 0;i < tree.id.size();i++){
        std::cout << i << " -> " << tree.id[i] << " " << tree.val[i] << " : ";
        for(int j = 0;j < tree.adj[i].size();j++){
            std::cout << tree.adj[i][j] << " ";
        }
        std::cout << "\n";
    }*/
    std::cout << "\nParser Result:\n";
    vizTree(tree, 0);

    return 0;
}