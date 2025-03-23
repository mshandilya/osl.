from dataclasses import dataclass
from collections.abc import Iterator
from more_itertools import peekable
from pprint import pprint
from typing import List, Optional
import sys
import time

sys.setrecursionlimit(100000000)

class Environment:
    envs: List
    
    def __init__(self):
        self.envs = [{}]
    
    def enter_scope(self):
        self.envs.append({})
    
    def exit_scope(self):
        assert self.envs
        self.envs.pop()
    
    def add(self, var, val):
        assert var not in self.envs[-1], f"Variable {var} already defined"
        self.envs[-1][var] = val
    
    def get(self, var):
        for env in reversed(self.envs):
            if var in env:
                return env[var]
        raise ValueError(f"Variable {var} not defined")
    
    def update(self, var, val):
        for env in reversed(self.envs):
            if var in env:
                env[var] = val
                return
        raise ValueError(f"Variable {var} not defined")
    
    def copy(self):
        new_env = Environment()
        new_env.envs = [dict(scope) for scope in self.envs]
        return new_env

cnt = 0
def fresh():
    global cnt
    cnt = cnt + 1
    return cnt

@dataclass
class AST:
    pass

@dataclass
class BinOp(AST):
    op: str
    left: AST
    right: AST

@dataclass
class Number(AST):
    val: int | float 

@dataclass
class UnOp(AST):
    op: str
    right: AST
    
@dataclass
class If(AST):
    condition: AST
    then_body: AST
    else_body: AST

@dataclass
class IfUnM(AST):
    condition: AST
    then_body: AST
@dataclass
class Let(AST):
    var: AST
    e1: Optional[AST]

@dataclass
class Assign(AST):
    var: AST
    e1: AST

@dataclass
class Variable(AST):
    varName: str
    id: int = None

@dataclass
class LetFun(AST):
    name: AST   # considering functions as first-class just like variables else it'll be str
    params: List[AST]
    body: AST

@dataclass
class CallFun(AST):
    fn: AST     # considering functions as first-class just like variables else it'll be str
    args: List[AST]
    
@dataclass
class FunObj:
    params: List[AST]
    body: AST
    env: Environment
    
@dataclass
class Statements(AST):
    stmts: List[AST]
    
@dataclass
class PrintStmt(AST):
    expr: AST
    
@dataclass
class ReturnStmt(AST):
    expr: Optional[AST]
    
@dataclass
class Program(AST):
    decls: List[AST]

class ParseErr(Exception):
    pass

@dataclass
class StringLiteral(AST):
    val: str
             
class Token:
    pass

@dataclass
class NumberToken(Token):
    val: str

@dataclass
class OperatorToken(Token):
    op: str
    
@dataclass
class KeyWordToken(Token):
    op: str
    
@dataclass
class VariableToken(Token):
    varName: str

@dataclass
class FunCallToken(Token):
    funName: str


def lex(s: str) -> Iterator[Token]:
    i = 0
    prev_token = None

    while True:
        while i < len(s) and s[i].isspace():
            i += 1

        if i >= len(s):
            return

        if s[i].isalpha() or s[i] == '_':
            start = i
            while i < len(s) and (s[i].isalpha() or s[i].isdigit() or s[i] == '_'):
                i += 1
                name = s[start:i]

            if name in {"if", "else", "var", "in", "letFunc", "print", "return"}:
                prev_token = KeyWordToken(name)
                yield prev_token

            # If preceded by `letFunc`, it's a function definition
            elif isinstance(prev_token, KeyWordToken) and prev_token.op == "letFunc":
                prev_token = VariableToken(name)
                yield prev_token

            # If followed by '(', it's a function call
            elif i < len(s) and s[i] == "(":
                prev_token = FunCallToken(name)
                yield prev_token

            else:
                prev_token = VariableToken(name)
                yield prev_token

        elif s[i].isdigit():
            start = i
            while i < len(s) and (s[i].isdigit() or s[i] == '.'):
                i += 1
            prev_token = NumberToken(s[start:i])
            yield prev_token

        else:
            if s[i:i+2] in {"<=", ">=", "!=", "||", "&&", ":="}:
                prev_token = OperatorToken(s[i:i+2])
                yield prev_token
                i += 2
            elif s[i] in {'+', '*', '/', '^', '-', '(', ')', '<', '>', '=', '%', '\u221a', ",", "{", "}", ";"}:
                prev_token = OperatorToken(s[i])
                yield prev_token
                i += 1
            else:
                raise ParseErr(f"Unexpected character: {s[i]} at position {i}")

      
def parse(s: str) -> AST:
    t = peekable(lex(s))
    i = 0
    
    def consume(expected_type=None, expected_value=None):
        nonlocal i
        token = next(t, None)
        if token is None:
            raise ParseErr(f"Unexpected end of input at index {i}")
        if expected_type and not isinstance(token, expected_type):
            raise ParseErr(f"Expected {expected_type.__name__} at index {i}, got {type(token).__name__}")
        if expected_value:
            actual_value = getattr(token, "op", None) or getattr(token, "varName", None) or getattr(token, "val", None)
            if actual_value != expected_value:
                raise ParseErr(f"Expected '{expected_value}' at index {i}, got '{actual_value}'")
        i += 1
        return token
    
    def peek():
        return t.peek(None)
    
    def parse_program():
        decls = []
        while peek():
            decls.append(parse_declaration())
        return Program(decls)
    
    def parse_declaration():
        match peek():
            case KeyWordToken("letFunc"):
                return parse_func()
            case KeyWordToken("var"):
                return parse_let()
            case _:
                return parse_statement()
            
    def parse_func():
        consume(KeyWordToken, "letFunc")
        func_name = consume(VariableToken)
        
        consume(OperatorToken, "(")
        args = []
        if peek() != OperatorToken(")"):
            while True:
                args.append(Variable(consume(VariableToken).varName)) # what else otherwise in the parameter list
                if peek() == OperatorToken(","):
                    consume(OperatorToken, ",")
                else:
                    break
        consume(OperatorToken, ")")
        
        body = parse_block()

        return LetFun(Variable(func_name.varName), args, body)
    
    def parse_let():
        consume(KeyWordToken, "var")
        var = Variable(consume(VariableToken).varName)
        e1 = None
        if peek() == OperatorToken(":="):
            consume(OperatorToken, ":=")
            e1 = parse_expression()
        consume(OperatorToken, ";")
        return Let(var, e1)
    
    def parse_statement():
        match peek():
            case KeyWordToken("if"):
                return parse_if()
            case KeyWordToken("print"):
                consume(KeyWordToken, "print")
                consume(OperatorToken, "(")
                expr = parse_expression()
                consume(OperatorToken, ")")
                consume(OperatorToken, ";")
                return PrintStmt(expr)
            case KeyWordToken("return"):
                consume(KeyWordToken, "return")
                if peek() != OperatorToken(";"):
                    expr = parse_expression()
                else:
                    expr = None
                consume(OperatorToken, ";")
                return ReturnStmt(expr)
            case OperatorToken("{"):
                return parse_block()
            case _:
                expr = parse_expression()
                consume(OperatorToken, ";")
                return expr
        
    def parse_if():
        consume(KeyWordToken, "if")
        condition = parse_expression()
        then_body = parse_statement()
        if peek() == KeyWordToken("else"):
            consume(KeyWordToken, "else")
            else_body = parse_statement()
            return If(condition, then_body, else_body)
        return IfUnM(condition, then_body)
    
    def parse_block():
        consume(OperatorToken, "{")
        decls = []
        while peek() != OperatorToken("}"):
            decl = parse_declaration()
            decls.append(decl)
        consume(OperatorToken, "}")
        return Statements(decls) if decls else Statements([])
    
    def parse_expression():
        # expression -> expB | assignment
        # first parse the lhs, if it's a variable and next token is ':=' then it's an assignment
        # otherwise it's an expB so return it as is.
        ast = parse_bool()
        if not isinstance(ast, Variable) and peek() == OperatorToken(":="):
            raise ParseErr(f"Expected variable on the left side of assignment := operator at index {i}")
        if isinstance(ast, Variable) and peek() == OperatorToken(":="):
            consume(OperatorToken, ":=")
            e1 = parse_bool()
            return Assign(ast, e1)
        return ast
    
    def parse_bool():
        ast = parse_comparison()
        while True:
            match peek():
                case OperatorToken("||"):
                    consume()
                    ast = BinOp("||", ast, parse_comparison())
                case OperatorToken("&&"):
                    consume()
                    ast = BinOp("&&", ast, parse_comparison())
                case _:
                    return ast

    def parse_comparison():
        ast = parse_add()
        match peek():
            case OperatorToken(op) if op in {"<", ">", "<=", ">=", "=", "!="}:
                consume()
                return BinOp(op, ast, parse_add())
            case _:
                return ast
    
    def parse_add():
        ast = parse_mul()
        while True:
            match peek():
                case OperatorToken('+'):
                    consume()
                    ast = BinOp('+', ast, parse_mul())
                case OperatorToken('-'):
                    consume()
                    ast = BinOp('-', ast, parse_mul())
                case _:
                    return ast
 
    def parse_mul():
        ast = parse_exponentiation()
        while True:
            match peek():
                case OperatorToken('*'):
                    consume()
                    ast = BinOp("*", ast, parse_exponentiation())
                case OperatorToken('/'):
                    consume()
                    ast = BinOp("/", ast, parse_exponentiation())
                case OperatorToken("%"):
                    consume()
                    ast = BinOp("%", ast, parse_exponentiation())
                case _:
                    return ast
    
    def parse_exponentiation():
        ast = parse_atom()
        while True:
            match peek():
                case OperatorToken('^'):
                    consume()
                    ast = BinOp("^", ast, parse_exponentiation())
                case _:
                    return ast

    def parse_atom():
        match peek():
            case NumberToken(v):
                consume()
                val = float(v) if '.' in v else int(v)
                return Number(val)
            
            case VariableToken(varName):
                consume()
                return Variable(varName)
            
            case FunCallToken(_):
                fn_name = consume(FunCallToken).funName
                consume(OperatorToken, "(")
                args = []
                if peek() != OperatorToken(")"):
                    while True:
                        args.append(parse_expression())
                        if peek() == OperatorToken(","):
                            consume(OperatorToken, ",")
                        else:
                            break
                consume(OperatorToken, ")")
                return CallFun(Variable(fn_name), args)
            
            case OperatorToken('-'):
                consume()
                return UnOp("-", parse_atom())
            
            case OperatorToken('\u221a'):
                consume()
                return UnOp("\u221a", parse_atom())
            
            case OperatorToken("("):
                consume()
                ast = parse_expression()
                consume(OperatorToken, ")")
                return ast
            case _:
                raise ParseErr(f"Unexpected token at index {i}")

    return parse_program()


def resolve(program: AST, env: Environment = None) -> AST:
    if env is None:
        env = Environment()
    
    def resolve_(program: AST) -> AST:
        return resolve(program, env)

    match program:
        case Program(decls):
            new_decls = [resolve_(decl) for decl in decls]
            return Program(new_decls)
        
        case Variable(varName, _):
            return Variable(varName, env.get(varName))
        
        case Number(_) as N:
            return N
        
        case Let(Variable(varName, _), e1):
            re1 = resolve_(e1) if e1 else None
            env.add(varName, i := fresh())
            return Let(Variable(varName, i), re1)
        
        case Assign(Variable(varName, _), e1):
            re1 = resolve_(e1)
            return Assign(Variable(varName, env.get(varName)), re1)
        
        case LetFun(Variable(varName, _), params, body):
            env.add(varName, i := fresh())
            env.enter_scope()
            new_params = []
            for param in params:
                env.add(param.varName, j := fresh())
                new_params.append(Variable(param.varName, j))
            new_body = resolve_(body)
            env.exit_scope()
            return LetFun(Variable(varName, i), new_params, new_body)
        
        case Statements(stmts):
            env.enter_scope()
            stmts = [resolve_(stmt) for stmt in stmts]
            env.exit_scope()
            return Statements(stmts)
        
        case CallFun(fn, args):
            rfn = resolve_(fn)
            rargs = [resolve_(arg) for arg in args]
            return CallFun(rfn, rargs)
        
        case BinOp(op, left, right):
            le = resolve_(left)
            ri = resolve_(right)
            return BinOp(op, le, ri)
        case UnOp(op, right):
            ri = resolve_(right)
            return UnOp(op, ri)
        
        case If(condition, then_body, else_body):
            condition = resolve_(condition)
            then_body = resolve_(then_body)
            else_body = resolve_(else_body)
            return If(condition, then_body, else_body)
        
        case IfUnM(condition, then_body):
            condition = resolve_(condition)
            then_body = resolve_(then_body)
            return IfUnM(condition, then_body)
        
        case PrintStmt(expr):
            return PrintStmt(resolve_(expr))
        
        case ReturnStmt(expr):
            return ReturnStmt(resolve_(expr))

def e(tree: AST, env: Environment = None) -> int | float | bool:
    if env is None:
        env = Environment()
        
    def e_(tree: AST):
        return e(tree, env)
    
    match tree:
        case Program(decls):
            res = None
            for decl in decls:
                res = e_(decl)
            return res
        
        case Number(val):
            return val
        
        case Variable(varName, i):
            return env.get(f"{varName}:{i}")
        
        case Let(Variable(varName, i), e1):
            v1 = e_(e1) if e1 else None
            env.add(f"{varName}:{i}", v1)
            return None
        
        case Assign(Variable(varName, i), e1):
            v1 = e_(e1)
            env.update(f"{varName}:{i}", v1)
            return None
        
        case LetFun(Variable(varName, i), params, body):
            # Closure -> Copy of Environment taken along with the declaration!
            funObj = FunObj(params, body, None)
            env.add(f"{varName}:{i}", funObj)
            funObj.env = env.copy()
            return None
        
        case CallFun(Variable(varName, i), args):
            fun = env.get(f"{varName}:{i}")
            rargs = [e_(arg) for arg in args]
            
            # use the environment that was copied when the function was defined
            call_env = fun.env.copy()
            call_env.enter_scope()
            for param, arg in zip(fun.params, rargs):
                call_env.add(f"{param.varName}:{param.id}", arg)
            
            rbody = e(fun.body, call_env)
            return rbody
        
        case Statements(stmts):
            env.enter_scope()
            res = None
            for i, stmt in enumerate(stmts):
                res = e_(stmt)
                if res is not None:
                    env.exit_scope()
                    return res
            env.exit_scope()
            return res
        
        case PrintStmt(expr):
            print(e_(expr))
            return
        
        case ReturnStmt(expr):
            if expr:
                return e_(expr)
            return None
            
        case BinOp("+", left, right): return e_(left) + e_(right)
        case BinOp("*", left, right): return e_(left) * e_(right)
        case BinOp("/", left, right): 
            left_val = e_(left)
            right_val = e_(right)
            if isinstance(left_val, int) and isinstance(right_val, int):
                return left_val // right_val
            return left_val / right_val
        case BinOp("^", left, right): return e_(left) ** e_(right)
        case BinOp("-", left, right): return e_(left) - e_(right)
        case BinOp("<", left, right): return e_(left) < e_(right)
        case BinOp(">", left, right): return e_(left) > e_(right)
        case BinOp("<=", left, right): return e_(left) <= e_(right)
        case BinOp(">=", left, right): return e_(left) >= e_(right)
        case BinOp("=", left, right): return e_(left) == e_(right)
        case BinOp("!=", left, right): return e_(left) != e_(right)
        case BinOp("%", left, right): return e_(left) % e_(right)
        case BinOp("||", left, right): return e_(left) or e_(right)
        case BinOp("&&", left, right): return e_(left) and e_(right)
        
        case UnOp("-", right): return -e_(right)
        case UnOp("\u221a", right): return e_(right) ** 0.5
        
        case If(condition, then_body, else_body): 
            if e_(condition):
                return e_(then_body) 
            else:
                return e_(else_body)
        
        case IfUnM(condition, then_body):
            if e_(condition):
                return e_(then_body)
            return None


exp = """
var x := 5;
letFunc f(y) 
{
    return x;
} 
print(x);
print(f(2));
letFunc g(z) 
{ 
    var x := 6;
    return f(z);
}
print(g(0));
"""

exp = """
var x := 5;
letFunc f(y) 
{
    return y ^ 2;
}
{
    var x := 6;
    return f(x);
}
print(f(x));
print(x);
letFunc g(z)
{
    return f(z);
}
print(f(g(2)));
print(g(3));
"""

exp = """
letFunc f1()
{
    letFunc f2()
    {
        var x := 10;
        return x;
    }
    return f2;
}
var msg := f1();
msg();
"""

exp = """
letFunc f1()
{
    var x := 10;
    letFunc f2()
    {
        return x;
    }
    return f2;
}
var msg := f1();
msg();
"""

exp = """
var x := 6;

letFunc F(x)
{
    letFunc G()
    {
        return x;
    }
    return G;
}

var y := F(5);
y() * y();
"""

exp = """
var x := 15;
if (x > 10)
if (x < 20)
print(x + 1);
else print(x - 1);
x;
"""

exp = """
letFunc fact(n)
{
    if (n = 0)
        return 1;
    return n * fact(n - 1);
}
fact(5);
"""

exp = """
letFunc fun(F, x)
{
    return F(x);
}

letFunc square(x)
{
    return x ^ 2;
}

fun(square, 5);
"""

## Euler Project Problem 1
exp = """
letFunc F(x, s)
{
    if (x = 1000) return s;
    if (x % 3 = 0 || x % 5 = 0) 
        return F(x + 1, s + x);
    return F(x + 1, s);
}

F(0, 0);
"""

# Euler Project Problem 2
exp = """
letFunc fib(a, b, s)
{
    if (a >= 4000000) 
        return s;
    if (a % 2 = 0) 
        return fib(b, a + b, s + a);
    return fib(b, a + b, s);
}

fib(0, 1, 0);
"""

# Euler Project Problem 3
exp = """
letFunc prime(n, i)
{
    if (i * i > n) 
        return n;
    
    if (n % i = 0)
        return prime(n / i, i);
    
    return prime(n, i + 1);
}

var n := 600851475143;
prime(n, 2);
"""

exp = """
var x := 5;
x := x + 6;
x;
"""

exp1 = """
letFunc isPal(n, rev, org)
{
    if (n = 0) return rev = org;
    
    return isPal(n/10, rev*10 + n%10, org);
}

var n := 10;

letFunc f(n)
{   
    if (n = 0) return 0;
    print(n);
    print(isPal(9999, 0, 9999));
    return f(n - 1);
}
f(n);
"""

exp1 = """
var x := 0;
var n := 121;

x := (x * 10) + (n % 10);
n := n / 10;
print(x);
print(n);

x := (x * 10) + (n % 10);
n := n / 10;
print(x);
print(n);

x := (x * 10) + (n % 10);
n := n / 10;
print(x);
n;
"""

# Euler Project Problem 4
exp = """
letFunc isPal(n, rev, org)
{
    if (n = 0) return rev = org;   
    return isPal(n/10, rev*10 + n%10, org);
}

letFunc F(i, j, maxPal)
{
    if (i < 100) return maxPal;
    if (j < 100) return F(i - 1, i - 1, maxPal);
    
    var prod := i * j;
    if ((prod > maxPal) && (isPal(prod, 0, prod))) 
        maxPal := prod;
    
    return F(i, j - 1, maxPal);
}

F(999, 999, 0);
"""

# print(exp)
# print()

# for i, t in enumerate(lex(exp)):
#     print(f"{i}: {t}")
# print()
# pprint(parse(exp))
# print()
# rexp = resolve(parse(exp))
# pprint(rexp)
# print()
# # print(e(rexp))
