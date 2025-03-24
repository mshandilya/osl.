#ifndef PARSER_H
#define PARSER_H

#include "utils.h"

namespace parser{
    class parseTree{
        private:
        int nodeCnt = 0;

        public:
        std::vector<std::vector<int>> adj;
        std::vector<std::string> id;
        std::vector<std::string> val;
        parseTree() {};
        int newNode();
    };

    class PDA{
        private:
        std::vector<std::vector<std::vector<std::string>>> rules;
        std::vector<std::string> allTokens;
        
        public:
        PDA() {};
        PDA(std::vector<std::vector<std::vector<std::string>>> rules, std::vector<std::string> allTokens);
        std::map<std::string, int> symId;
        std::unordered_map<int, std::string> idSym;
        std::unordered_map<int, bool> isToken;
        std::unordered_map<int, std::unordered_map<int, std::vector<std::vector<int>>>> trans;
        std::unordered_map<int, std::unordered_map<int, std::vector<int>>> pro;
    };

    class genParser{
        private:
        PDA pda;
        parseTree parse(std::vector<Token> tokens);
        void populateTree(int curNode, int curSym, int &pind, std::vector<int> path, int &tind, std::vector<Token> tokens, parseTree &tree);

        public:
        genParser(std::string filename, std::vector<std::string> allTokens);
        std::function<parseTree(std::vector<Token>)> getParser();
    };
}

#endif