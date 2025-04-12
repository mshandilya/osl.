#ifndef LEXER_H
#define LEXER_H

#include "utils.hpp"

namespace lexer{
    class DFA{
        private:
        std::vector<std::string> syms, names;
        std::vector<std::pair<bool, std::string>> term;
        std::vector<std::array<std::vector<int>,256>> nodes;
        std::vector<int> e;
        int totNodes = 0;
        int initNode();
        void insert(std::string sym, std::string name);
        void parse(std::string &sym, int &i, int &node);
        void parse_union(std::string &sym, int &i, int &node);
        
        public:
        std::unordered_map<int, std::string> lines;
        std::map<std::string, std::string> desc;
        std::vector<int> curNodes;
        DFA();
        DFA(std::vector<std::string> syms, std::vector<std::string> names);
        Token feed(char c, int lineNo, int charNo, char pc);
    };

    class genLexer{
        private:
        std::vector<Token> lex(std::string filename);

        public:
        DFA dfa;
        std::vector<std::string> allTokens;
        genLexer(std::string filename);
        std::function<std::vector<Token>(const char*)> getLexer();
    };
}

#endif