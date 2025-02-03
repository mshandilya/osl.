#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <cmath>
#include <stdexcept>


class AST
{
public:
    virtual ~AST() = default;
    virtual double evaluate() const = 0;
};


class Number : public AST
{
    double val;
};

class StringLiteral : public AST
{

};