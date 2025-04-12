#include "parser.hpp"

int parser::parseTree::newNode(){
    adj.push_back({});
    id.push_back("");
    val.push_back("");
    return nodeCnt++;
}

int parser::parseTree::getNodeCnt(){
    return nodeCnt;
}

parser::PDA::PDA(std::vector<std::vector<std::vector<std::string>>> rules, std::vector<std::string> allTokens): rules(rules), allTokens(allTokens) {
    //Assign ids to symbols
    int idCnt = 1;
    for(std::vector<std::vector<std::string>> rule: rules){
        for(std::vector<std::string> prod: rule){
            for(std::string sym: prod){
                if(symId.find(sym) == symId.end()){
                    idSym[idCnt] = sym;
                    symId[sym] = idCnt++;
                }
            }
        }
    }
    for(std::string tokenName: allTokens){
        isToken[symId[tokenName]] = true;
    }
    for(int i = rules.size()-1;i >= 0;i--){
        int stSym = symId[rules[i][0][0]];
        if(!leftMost[stSym].empty()){
            continue;
        }
        for(int j = 1;j < rules[i].size();j++){
            int possSym = symId[rules[i][j][0]];
            if(isToken[possSym]){
                leftMost[stSym][possSym] = true;
            }else{
                for(auto possToken: leftMost[possSym]){
                    leftMost[stSym][possToken.first] = true;
                }
            }
        }
    }
    //Construct transitions
    for(std::vector<std::vector<std::string>> rule: rules){
        int stSym = symId[rule[0][0]];
        for(int i = 1;i < rule.size();i++){
            for(std::string sym: rule[i]){
                pro[stSym][i].push_back(symId[sym]);
            }
            std::vector<std::string> prod = rule[i];
            int inpSym = symId[prod[0]];
            if(!isToken[inpSym]){
                inpSym = 0;
            }
            std::vector<int> curTrans;
            curTrans.push_back(i);
            for(int j = prod.size()-1;j >= 1-(int)(inpSym==0);j--){
                int sym = symId[prod[j]];
                curTrans.push_back(sym);   
            }
            trans[inpSym][stSym].push_back(curTrans);
        }
    }
}

parser::genParser::genParser(std::string filename, std::vector<std::string> allTokens, std::map<std::string, std::string> desc, std::unordered_map<int, std::string> lines): desc(desc), lines(lines){
    std::ifstream fin;
    try{
        fin.open(filename);
    }catch(const std::exception& e){
        std::cout << e.what() << std::endl;
    }
    std::vector<std::vector<std::vector<std::string>>> rules;
    std::string line;
    while(getline(fin, line)){
        std::vector<std::string> v = split(line," ::= ");
        std::vector<std::string> prods = split(v[1], " | ");
        std::vector<std::vector<std::string>> rule;
        rule.push_back({v[0]});
        for(std::string prod: prods){
            std::vector<std::string> p = split(prod, " ");
            rule.push_back(p);
        }
        rules.push_back(rule);
    }
    pda = PDA(rules, allTokens);
}

void parser::genParser::populateTree(int curNode, int curSym, int &pind, std::vector<int> path, int &tind, std::vector<Token> tokens, parseTree &tree){
    tree.id[curNode] = pda.idSym[curSym];
    if(pda.isToken[curSym]){
        tree.val[curNode] = tokens[tind++].getVal();
        return;
    }
    int ind = path[pind++];
    for(int i = 0;i < pda.pro[curSym][ind].size();i++){
        int nNode = tree.newNode();
        tree.adj[curNode].push_back(nNode);
        populateTree(nNode, pda.pro[curSym][ind][i], pind, path, tind, tokens, tree);
    }
}

parser::parseTree parser::genParser::parse(std::vector<Token> tokens){
    std::queue<std::tuple<std::stack<int>,std::vector<int>,int>> q;
    std::stack<int> st;st.push(1);
    q.push({st,{},0});
    int longestPath = 0;
    int expVal;
    std::string befVal, aftVal;
    std::pair<int,int> pos;
    int missVal = -1;
    bool single = false;
    while(!q.empty()){
        std::tuple<std::stack<int>,std::vector<int>,int> cur = q.front();
        q.pop();
        std::stack<int> st = std::get<0>(cur);
        std::vector<int> path = std::get<1>(cur);
        int curPos = std::get<2>(cur);
        if(st.empty()){
            if(curPos == tokens.size()){
                parseTree tree;
                int tind = 0,pind = 0;
                populateTree(tree.newNode(),1,pind,path,tind,tokens,tree);
                return tree;   
            }
            continue;
        }
        if(curPos >= tokens.size()){
            if(pda.isToken[st.top()] && (missVal == -1 || !pda.isToken[missVal])){
                missVal = st.top();
            }else if(!pda.isToken[st.top()] && missVal == -1){
                missVal = st.top();
            }
            continue;
        }
        int curSym = st.top();
        st.pop();
        int inpSym = pda.symId[tokens[curPos].getId()];
        if(curPos > longestPath){
            single = true;
            longestPath = curPos;
            expVal = curSym;
            befVal = tokens[curPos-1].getVal();
            aftVal = tokens[curPos].getVal();
            pos = tokens[curPos-1].getPos();
        }else if(curPos == longestPath && expVal != curSym){
            single = false;
        }
        if(!pda.isToken[curSym] && pda.leftMost[curSym].find(inpSym) == pda.leftMost[curSym].end()){
            continue;
        }
        if(pda.isToken[curSym]){
            if(curSym == inpSym){
                q.push({st,path,curPos+1});
            }
        }else{
            for(std::vector<int> curTrans: pda.trans[inpSym][curSym]){
                std::stack<int> nst = st;
                std::vector<int> npath = path;
                npath.push_back(curTrans[0]);
                for(int i = 1;i < curTrans.size();i++){
                    nst.push(curTrans[i]);
                }
                q.push({nst,npath,curPos+1});
            }
            //Add e-transitions
            for(std::vector<int> curTrans: pda.trans[0][curSym]){
                std::stack<int> nst = st;
                std::vector<int> npath = path;
                npath.push_back(curTrans[0]);
                for(int i = 1;i < curTrans.size();i++){
                    nst.push(curTrans[i]);
                }
                q.push({nst,npath,curPos});
            }
        }
    }
    char charExp = missVal;
    std::cout << "code.osl:";
    if(missVal != -1){
        std::cout << "EOF: \033[31mSyntaxError\033[0m: ";
        befVal = tokens[tokens.size()-1].getVal();
    }else{
        charExp = expVal;
        std::cout << pos.first << ":" << pos.second << ": \033[31mSyntaxError:\033[0m ";
        std::cout << "Unexpected `" << aftVal << "`. ";
    }
    if(single){
        std::cout << "Expected ";
    }else{
        std::cout << "Did you mean ";
    }
    std::string corr = "";
    if(!pda.isToken[charExp]){
        std::cout << pda.idSym[charExp].substr(1,pda.idSym[charExp].length()-2) << " ";
        if(pda.leftMost[charExp].size() == 1){
            corr = desc[pda.idSym[pda.leftMost[charExp].begin()->first]];
            std::cout << "beginning with `" << corr << "` ";
        }else{
            corr = pda.idSym[charExp].substr(1,pda.idSym[charExp].length()-2);
        }
    }else{
        corr = desc[pda.idSym[charExp]];
        std::cout << "`" << corr << "` ";
    }
    std::cout << "after `" << befVal << "`";
    if(single){
        std::cout << "." << std::endl;
    }else{
        std::cout << "?" << std::endl;
    }
    std::cout << " " << pos.first << " | " << lines[pos.first] << std::endl;
    for(int i = 0;i < numChar(pos.first)+2;i++){
        std::cout << " ";
    }
    std::cout << "|";
    for(int i = 0;i < pos.second+1;i++){
        std::cout << " ";
    }
    std::cout << "\033[31m^\033[0m" << std::endl;
    for(int i = 0;i < numChar(pos.first)+2;i++){
        std::cout << " ";
    }
    std::cout << "|";
    for(int i = 0;i < pos.second+1;i++){
        std::cout << " ";
    }
    if(single){
        std::cout << "\033[32m";
    }else{
        std::cout << "\033[1;33m";
    }
    std::cout << corr << "\033[0m" << std::endl;
    return parseTree();
}

std::function<parser::parseTree(std::vector<Token>)> parser::genParser::getParser(){
    return [this](std::vector<Token> tokens) { return this->parse(tokens); };
}