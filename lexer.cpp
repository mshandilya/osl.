#include "lexer.hpp"

lexer::DFA::DFA(){
    initNode();
    nodes[0][' '].push_back(0);
    nodes[0]['\n'].push_back(0);
    nodes[0]['\t'].push_back(0);
    curNodes.push_back(0);
}

lexer::DFA::DFA(std::vector<std::string> syms, std::vector<std::string> names): syms(syms), names(names){
    initNode();
    nodes[0][' '].push_back(0);
    nodes[0]['\n'].push_back(0);
    nodes[0]['\t'].push_back(0);
    curNodes.push_back(0);
    for(int i = 0;i < syms.size();i++){
        insert(syms[i], names[i]);
    }
};

int lexer::DFA::initNode(){
    std::array<std::vector<int>,256> cur;
    cur.fill({});
    nodes.push_back(cur);
    term.push_back({false, ""});
    e.push_back(-1);
    totNodes++;
    return totNodes-1;
}

void lexer::DFA::parse_union(std::string &sym, int &i, int &node){
    int j = i+1;
    while(j < sym.size() && sym[j] != ']'){
        j++;
    }
    bool plus = false;
    if(j+1 < sym.size() && sym[j+1] == '+'){
        plus = true;
    }
    i++;
    int nxtNode = initNode();
    bool star = false;
    if(j+1 < sym.size() && sym[j+1] == '*'){
        star = true;
        e[node] = nxtNode;
    }
    while(i < sym.size() && sym[i] != ']'){
        if(sym[i] == '\\'){
            i++;
            if(i == sym.size()){
                throw std::logic_error("Invalid regex: "+sym);
            }
            nodes[node][sym[i]].push_back(nxtNode);
            if(plus || star){
                nodes[nxtNode][sym[i]].push_back(nxtNode);
            }
            i++;
        }else if(i+2 < sym.size() && sym[i+1] == '-'){
            for(char c = sym[i];c <= sym[i+2];c++){
                nodes[node][c].push_back(nxtNode);
                if(plus || star){
                    nodes[nxtNode][c].push_back(nxtNode);
                }
            }
            i += 3;
        }else if(sym[i] == '['){
            parse_union(sym, i, node);
        }else if(sym[i] == ']' || sym[i] == '+' || sym[i] == '-' || sym[i] == '*' || sym[i] == '|'){
            throw std::logic_error("Invalid regex: "+sym);
        }else{
            nodes[node][sym[i]].push_back(nxtNode);
            if(plus || star){
                nodes[nxtNode][sym[i]].push_back(nxtNode);
            }
            i++;
        }
    }
    i++;
    if(plus || star){
        i++;
    }
    node = nxtNode;
}

void lexer::DFA::parse(std::string &sym, int &i, int &node){
    while(i < sym.size()){
        int prevNode = node;
        if(sym[i] == '['){
            parse_union(sym, i, node);
        }else if(sym[i] == '\\'){
            i++;
            if(i == sym.size()){
                throw std::logic_error("Invalid regex: "+sym);
            }
            int nxtNode = initNode();
            nodes[node][sym[i]].push_back(nxtNode);
            node = nxtNode;
            i++;
        }else if(sym[i] == '+'){
            if(i == 0){
                throw std::logic_error("Invalid regex: "+sym);
            }
            nodes[node][sym[i-1]].push_back(node);
            i++;
        }else if(sym[i] == '*'){
            if(i == 0){
                throw std::logic_error("Invalid regex: "+sym);
            }
            nodes[node][sym[i-1]].push_back(node);
            e[prevNode] = node;
            i++;
        }else if(i+2 < sym.size() && sym[i+1] == '-'){
            int nxtNode = initNode();
            for(char c = sym[i];c <= sym[i+2];c++){
                nodes[node][c].push_back(nxtNode);
            }
            node = nxtNode;
            i += 3;
        }else if(sym[i] == ']' || sym[i] == '+' || sym[i] == '-' || sym[i] == '*'){
            throw std::logic_error("Invalid regex: "+sym);
        }else if(sym[i] == '|'){
            break;
        }else{
            int nxtNode = initNode();
            nodes[node][sym[i]].push_back(nxtNode);
            node = nxtNode;
            i++;
        }
    }
}

void lexer::DFA::insert(std::string sym, std::string name){
    desc[name] = regFormat(sym, name);
    int node = 0;
    int i = 0;
    int nxtNode = initNode();
    while(i < sym.size()){
        if(sym[i] == '|'){
            if(i == 0){
                throw std::logic_error("Invalid regex: "+sym);
            }
            i++;
        }else{
            parse(sym, i, node);
            if(node != 0){
                e[node] = nxtNode;
            }
            node = 0;
        }
    }
    node = nxtNode;
    char lastChar = '\0';
    for(int i = 0;i < sym.size();i++){
        if(sym[i] == '\\'){
            if(i+1 == sym.size()){
                throw std::logic_error("Invalid regex: "+sym);
            }
            lastChar = sym[i+1];
            i++;
        }else if(sym[i] == '*' || sym[i] == '+' || sym[i] == '[' || sym[i] == ']' || sym[i] == '-' || sym[i] == '|'){
            continue;
        }else{
            lastChar = sym[i];
        }
    }
    bool endAlphaNum = (lastChar == '.') || (lastChar == '_') || ('a' <= lastChar && lastChar <= 'z') || ('A' <= lastChar && lastChar <= 'Z') || ('0' <= lastChar && lastChar <= '9');
    std::string subName = name.substr(1,name.length()-1);
    bool endSpBrac = name == "COMMA" || subName == "PAREN" || subName == "BRACE" || subName == "BOX";
    for(int i = 1;i < 256;i++){
        char curChar = (char)i;
        bool curEndAlphaNum = (curChar == '.') || (curChar == '_') || ('a' <= curChar && curChar <= 'z') || ('A' <= curChar && curChar <= 'Z') || ('0' <= curChar && curChar <= '9');
        bool curEndSpace = (curChar == ' ' || curChar == '\n' || curChar == '\t');
        if((endAlphaNum && !curEndAlphaNum) || endSpBrac || (!endAlphaNum && !endSpBrac && (curEndAlphaNum || curEndSpace))){
            int nNode = initNode();
            nodes[node][i].push_back(nNode);
            term[nNode] = {true, name};
        }
    }
}

Token lexer::DFA::feed(char c, int lineNo, int charNo, char pc){
    std::string name = "NONE";
    int maxNode = -1;
    std::vector<int> nextCurNodes;
    bool empty = true;
    for(int curNode: curNodes){
        while(nodes[curNode][c].empty() && e[curNode] != -1){
            curNode = e[curNode];
        }
        if(nodes[curNode][c].empty() && e[curNode] == -1){
            continue;
        }
        if(e[curNode] != -1){
            nextCurNodes.push_back(e[curNode]);
            if(term[e[curNode]].first){
                if(maxNode < e[curNode]){
                    maxNode = e[curNode];
                    name = term[e[curNode]].second;
                    nextCurNodes = {0};
                }
            }
        }
        if(!nodes[curNode][c].empty()){
            empty = false;
        }
        for(int nCurNode: nodes[curNode][c]){
            nextCurNodes.push_back(nCurNode);
            if(term[nCurNode].first){
                if(maxNode < nCurNode){
                    maxNode = nCurNode;
                    name = term[nCurNode].second;
                    nextCurNodes.clear();
                    for(auto iniNode: nodes[0][c]){
                        nextCurNodes.push_back(iniNode);
                    }
                    if(nodes[0][c].empty()){
                        std::cout << "code.osl:" << lineNo << ":" << charNo << ": \033[31mTokenError\033[0m: Unexpected " << charFormat(pc) << "." << std::endl;
                        exit(0);
                    }
                }
            }
        }
    }
    if(empty){
        char expVal = '\0';
        bool unique = false;
        for(int curNode: curNodes){
            for(int i = 1;i < 256;i++){
                if(('a' <= i && i <= 'z') || ('A' <= i && i <= 'Z') || ('0' <= i && i <= '9') || i == '_' || i == ' ' || i == '\t' || i == '\n'){
                    continue;
                }
                if(!nodes[curNode][i].empty()){
                    if(expVal == '\0'){
                        expVal = i;
                        unique = true;
                    }else if(expVal != i){
                        unique = false;
                        goto fin;
                    }
                }
            }
        }
        fin:
        if(!unique){
            std::cout << "code.osl:" << lineNo << ":" << charNo << ": \033[31mTokenError\033[0m: ";
        }else{
            std::cout << "code.osl:" << lineNo << ":" << charNo+1 << ": \033[31mTokenError\033[0m: ";
        }
        std::cout << "Unexpected " << charFormat(c) << ".";
        if(unique){
            std::cout << " Expected ";
            std::cout << charFormat(expVal);
            std::cout << " after ";
            std::cout << charFormat(pc);
            std::cout << ".";
        }else{
            if(expVal != '\0'){
                std::cout << " Did you mean " << charFormat(expVal) << " after " << charFormat(pc) << "?";
            }
        }
        std::cout << std::endl;
        exit(0);
    }
    if(c != '\n'){
        lines[lineNo] += c;
    }
    curNodes = nextCurNodes;
    if(maxNode != -1){
        return Token(name, lineNo, charNo);
    }
    return Token();
}

lexer::genLexer::genLexer(std::string filename){
    std::ifstream fin;
    try{
        fin.open(filename);
    }catch(const std::exception& e){
        std::cout << e.what() << std::endl;
    }
    std::string exp, name;
    std::vector<std::string> syms,names;
    while(fin >> exp >> name){
        syms.push_back(exp);
        names.push_back(name);
    }
    fin.close();
    allTokens = names;
    dfa = DFA(syms, names);
}

std::vector<Token> lexer::genLexer::lex(std::string filename){
    std::ifstream fin;
    try{
        fin.open(filename);
    }catch(const std::exception& e){
        std::cout << e.what() << std::endl;
    }
    std::vector<Token> tokens;
    std::string val = "";
    char buf = ' ';
    char c;
    char pc = '\0';
    int lineNo = 1;
    int charNo = 0;
    while(fin.get(c)){
        Token cur = dfa.feed(c, lineNo, charNo, pc);
        pc = c;
        if(c != ' ' && c != '\n' && c != '\t'){
            buf = c;
        }else{
            buf = '\0';
        }
        std::string cid = cur.getId();
        if(cid != "NONE"){
            /*bool fval = false;
            for(std::string token: isValToken){
                if(cid == token){
                    tokens.push_back(Token(cid, val, lineNo, charNo));
                    fval = true;
                    break;
                }
            }
            if(!fval){
                tokens.push_back(cur);
            }*/
            tokens.push_back(Token(cid, val, lineNo, charNo));
            val = "";
        }
        if(buf != '\0'){
            val += buf;
        }
        charNo++;
        if(c == '\n'){
            lineNo++;
            charNo = 0;
        }
    }
    Token cur = dfa.feed(' ', lineNo, charNo, pc);
    std::string cid = cur.getId();
    if(cid != "NONE"){
        /*bool fval = false;
        for(std::string token: isValToken){
            if(cid == token){
                tokens.push_back(Token(cid, val, lineNo, charNo));
                fval = true;
                break;
            }
        }
        if(!fval){
            tokens.push_back(cur);
        }*/
        tokens.push_back(Token(cid, val, lineNo, charNo));
    }
    fin.close();
    return tokens;
}

std::function<std::vector<Token>(const char*)> lexer::genLexer::getLexer(){
    return [this](const char* filename) { return this->lex(filename); };
}