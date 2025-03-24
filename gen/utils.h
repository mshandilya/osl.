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

class Token{
    private:
    std::string id;
    std::string val;

    public:
    Token();
    Token(std::string x);
    Token(std::string x, std::string y);
    friend std::ostream& operator<<(std::ostream& os, Token const& t);
    std::string getId();
    std::string getVal();
};

std::vector<std::string> split(std::string s, std::string t);

#endif