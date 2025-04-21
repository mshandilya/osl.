#ifndef PARSER_H
#define PARSER_H

#include "utils.hpp"

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
        int getNodeCnt();
    };

    void vizTree(const parseTree &tree, int node, const std::string &prefix, bool isLast);

    class PDA{
        private:
        std::vector<std::vector<std::vector<std::string>>> rules;
        std::vector<std::string> allTokens;
        
        public:
        PDA() {};
        PDA(std::vector<std::vector<std::vector<std::string>>> rules, std::vector<std::string> allTokens, std::map<std::string, std::string> &desc);
        std::map<std::string, int> symId;
        std::unordered_map<int, std::string> idSym;
        std::unordered_map<int, bool> isToken;
        std::unordered_map<int, std::unordered_map<int, std::vector<std::vector<int>>>> trans;
        std::unordered_map<int, std::unordered_map<int, std::vector<int>>> pro;
        std::unordered_map<int, std::unordered_map<int,bool>> leftMost;
    };

    class genParser{
        private:
        PDA pda;
        parseTree parse(std::vector<Token> tokens);
        void populateTree(int curNode, int curSym, int &pind, std::vector<int> path, int &tind, std::vector<Token> tokens, parseTree &tree);
        std::map<std::string, std::string> desc;
        std::unordered_map<int, std::string> lines;

        public:
        genParser(std::string filename, std::vector<std::string> allTokens, std::map<std::string, std::string> des, std::unordered_map<int, std::string> lines);
        std::function<parseTree(std::vector<Token>)> getParser();
    };

    typedef std::tuple<int, std::stack<int>, std::vector<int>, std::unordered_map<int,int>> qInfo;
    class compQInfo {
    public:
        bool operator()(qInfo a, qInfo b){
            if(std::get<0>(a) < std::get<0>(b)){
                return true;
            }else if(std::get<2>(a).size() < std::get<2>(b).size()){
                return true;
            }
            return false;
        }
    };
}

#endif