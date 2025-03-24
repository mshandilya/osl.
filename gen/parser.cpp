#include "parser.h"

int parser::parseTree::newNode(){
    adj.push_back({});
    id.push_back("");
    val.push_back("");
    return nodeCnt++;
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

parser::genParser::genParser(std::string filename, std::vector<std::string> allTokens){
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
    while(!q.empty()){
        std::tuple<std::stack<int>,std::vector<int>,int> cur = q.front();
        q.pop();
        std::stack<int> st = std::get<0>(cur);
        std::vector<int> path = std::get<1>(cur);
        int curPos = std::get<2>(cur);
        if(st.empty()){
            parseTree tree;
            int tind = 0,pind = 0;
            populateTree(tree.newNode(),1,pind,path,tind,tokens,tree);
            return tree;
        }
        if(curPos >= tokens.size()){
            continue;
        }
        int curSym = st.top();
        st.pop();
        int inpSym = pda.symId[tokens[curPos].getId()];
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
    std::cout << "Parse failed\n";
    return parseTree();
}

std::function<parser::parseTree(std::vector<Token>)> parser::genParser::getParser(){
    return [this](std::vector<Token> tokens) { return this->parse(tokens); };
}