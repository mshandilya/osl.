#ifndef LEXER_H
#define LEXER_H

#include "utils.h"

namespace lexer{
    class Trie{
        private:
        std::vector<std::string> syms, names;
        std::vector<std::pair<bool, std::string>> term;
        std::vector<std::array<int,256>> nodes;
        std::map<std::string, bool> sp;
        int curNode = 0;    
        int initNode();
        void insert(std::string sym, std::string name);

        public:
        Trie();
        Trie(std::vector<std::string> syms, std::vector<std::string> names, std::map<std::string, bool> sp);
        Token feed(char c);
    };

    class genLexer{
        private:
        Trie trie;
        std::vector<Token> lex(std::string filename);

        public:
        std::vector<std::string> allTokens;
        genLexer(std::string filename);
        std::function<std::vector<Token>(const char*)> getLexer();
    };
}

#endif