#ifndef UTILS_H
#define UTILS_H

#include<string>
#include<vector>
#include<iostream>
#include<fstream>
#include<array>
#include<functional>
#include<stdexcept>
#include<map>
#include<queue>
#include<stack>
#include<memory>

class Token{
    private:
    std::string id;
    std::string val;
    std::pair<int,int> pos;

    public:
    Token();
    Token(std::string x, int lineNo, int charNo);
    Token(std::string x, std::string y, int lineNo, int charNo);
    friend std::ostream& operator<<(std::ostream& os, Token const& t);
    std::string getId();
    std::string getVal();
    std::pair<int,int> getPos();
};

bool isValToken(std::string s);
std::vector<std::string> split(std::string s, std::string t);
std::string charFormat(char c);
std::string regFormat(std::string s, std::string name);
int numChar(int val);

#endif