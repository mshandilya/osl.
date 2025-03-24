#include "utils.h"

Token::Token(): id("NONE"), val("") {};
Token::Token(std::string x): id(x), val("") {};
Token::Token(std::string x, std::string y): id(x), val(y) {};
    
std::ostream& operator<<(std::ostream& os, Token const& t){
    return os << t.id;
}

std::string Token::getId(){
    return id;
}

std::string Token::getVal(){
    return val;
}

std::vector<std::string> split(std::string s, std::string t){
    std::vector<std::string> v;
    int ls = 0;
    for(int i = 0;i < s.length();i++){
        bool match = true;
        for(int j = 0;j < t.length();j++){
            match &= s[i+j] == t[j];
        }
        if(match){
            v.push_back(s.substr(ls,i-ls));
            i += t.length() - 1;
            ls = i+1;
        }
    }
    if(ls < s.length()){
        v.push_back(s.substr(ls));
    }
    return v;
}