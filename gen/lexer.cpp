#include "lexer.h"

lexer::Trie::Trie(){
    if(nodes.empty()){
        initNode();
    }
}

lexer::Trie::Trie(std::vector<std::string> syms, std::vector<std::string> names, std::map<std::string, bool> sp): syms(syms), names(names), sp(sp){
    if(nodes.empty()){
        initNode();
    }
    for(int i = 0;i < syms.size();i++){
        insert(syms[i], names[i]);
    }
};

int lexer::Trie::initNode(){
    std::array<int,256> cur;
    cur.fill(-1);
    nodes.push_back(cur);
    term.push_back({false, ""});
    return nodes.size()-1;
}

void lexer::Trie::insert(std::string sym, std::string name){
    int node = 0;
    for(char c: sym){
        if(nodes[node][c] == -1){
            nodes[node][c] = initNode();
        }
        node = nodes[node][c];
    }
    term[node] = {true, name};
}

Token lexer::Trie::feed(char c){
    if(c == ' ' || c == '\n'){
        return Token();
    }
    if(nodes[curNode][c] == -1){
        std::string err = "Unexpected ";err += c;err += "!";
        throw std::logic_error(err);
    }
    curNode = nodes[curNode][c];
    if(term[curNode].first){
        std::string name = term[curNode].second;
        curNode = 0;
        if(sp[name]){
            std::string tmp = "";tmp += c;
            return Token(name, tmp);
        }else{
            return Token(name);
        }
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
    std::map<std::string, bool> sp;
    while(fin >> exp >> name){
        std::vector<std::string> v = split(exp,"-->");
        std::string cur = v[0];
        while(true){
            syms.push_back(cur);
            names.push_back(name);
            if(cur == v.back()){
                break;
            }
            cur = (char)(((int)cur[0])+1);
        }
        if(v.size() > 1){
            sp[name] = true;
        }
    }
    fin.close();
    allTokens = names;
    trie = Trie(syms, names, sp);
}

std::vector<Token> lexer::genLexer::lex(std::string filename){
    std::ifstream fin;
    try{
        fin.open(filename);
    }catch(const std::exception& e){
        std::cout << e.what() << std::endl;
    }
    std::vector<Token> tokens;
    char c;
    while(fin >> c){
        Token cur = trie.feed(c);
        if(cur.getId() != "NONE"){
            tokens.push_back(cur);
        }
    }
    fin.close();
    return tokens;
}

std::function<std::vector<Token>(const char*)> lexer::genLexer::getLexer(){
    return [this](const char* filename) { return this->lex(filename); };
}