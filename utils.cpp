#include "utils.hpp"

Token::Token(): id("NONE"), val("") {};
Token::Token(std::string x, int lineNo, int charNo): id(x), val(""), pos({lineNo,charNo}) {};
Token::Token(std::string x, std::string y, int lineNo, int charNo): id(x), val(y), pos({lineNo,charNo}) {};

std::ostream& operator<<(std::ostream& os, Token const& t){
    return os << t.id;
}

std::string Token::getId(){
    return id;
}

std::string Token::getVal(){
    return val;
}

std::pair<int,int> Token::getPos(){
    return pos;
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

std::string charFormat(char c){
    std::string res;
    if(c == ' '){
        res = "space character";
    }else if(c == '\n'){
        res = "new line character";
    }else if(c == '\t'){
        res = "tab character";
    }else{
        res = "`";res += c;res += "`";
    }
    return res;
}

bool isValToken(std::string s){
    return s == "DEC" || s == "OCT" || s == "HEX" || s == "BIN" || s == "BOOL" || s == "IDEN";
}

std::string regFormat(std::string s, std::string name){
    std::string res = "";
    for(int i = 0;i < s.length();i++){
        if(s[i] == '\\'){
            i++;
            if(i < s.length()){
                res += s[i];
            }
            continue;
        }
        if(s[i] == '*' || s[i] == '+' || s[i] == '-' || s[i] == '|' || s[i] == '[' || s[i] == ']'){
            return name;
        }
        res += s[i];
    }
    return res;
}

int numChar(int val){
    if(val == 0){
        return 1;
    }
    int c = 0;
    while(val != 0){
        val /= 10;
        c++;
    }
    return c;
}