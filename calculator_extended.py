from dataclasses import dataclass
from collections.abc import Iterator
from more_itertools import peekable
from pprint import pprint
from graphviz import Digraph # type: ignore
from typing import List
import sys

sys.setrecursionlimit(10000)

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
class Let(AST):
    var: AST
    e1: AST
    e2: AST
    
@dataclass
class LetMut(AST):
    var: str
    e1: str
    e2: str

@dataclass
class Variable(AST):
    varName: str
    id: int = None
    
    # def make(name):
    #     return Variable(name, fresh())

@dataclass
class LetFun(AST):
    name: AST   # considering functions as first-class just like variables else it'll be str
    params: List[AST]
    body: AST
    expr: AST

@dataclass
class CallFun(AST):
    fn: AST     # considering functions as first-class just like variables else it'll be str
    args: List[AST]
    
@dataclass
class FunObj:
    params: List[AST]
    body: AST

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

def resolve(program: AST, env: Environment = None) -> AST:
    if env is None:
        env = Environment()
    
    def resolve_(program: AST) -> AST:
        return resolve(program, env)

    match program:
        case Variable(varName, _):
            return Variable(varName, env.get(varName)) # This ask too! why sir did env.get(varName)
            # return env.get(varName)
        
        case Number(_) as N:
            return N
        
        case Let(Variable(varName, _), e1, e2):
            re1 = resolve_(e1)
            env.enter_scope()
            env.add(varName, i := fresh())
            re2 = resolve_(e2)
            env.exit_scope()
            return Let(Variable(varName, i), re1, re2)
        
        case LetFun(Variable(varName, _), params, body, expr):
            env.enter_scope()
            env.add(varName, i := fresh())
            env.enter_scope()
            new_params = []
            for param in params:
                env.add(param.varName, j := fresh())
                new_params.append(Variable(param.varName, j))
            new_body = resolve_(body)
            env.exit_scope()
            new_expr = resolve_(expr)
            env.exit_scope()
            return LetFun(Variable(varName, i), new_params, new_body, new_expr)
        
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
        
        case If(op, left, right):
            op = resolve_(op)
            le = resolve_(left)
            ri = resolve_(right)
            return If(op, le, ri)
        


def e(tree: AST, env: Environment = None) -> int | float | bool:
    if env is None:
        env = Environment()
        
    def e_(tree: AST):
        return e(tree, env)
    
    match tree:
        case Number(val):
            # return float(val) if '.' in val else int(val)
            return val
        
        case Variable(varName, i): 
            return env.get(f"{varName}:{i}")
        
        case Let(Variable(varName, i), e1, e2):
            v1 = e_(e1)
            env.enter_scope()
            env.add(f"{varName}:{i}", v1)
            v2 = e_(e2)
            env.exit_scope()
            return v2
        
        case LetFun(Variable(varName, i), params, body, expr):
            env.enter_scope()
            # print(Variable(varName, i))
            env.add(f"{varName}:{i}", FunObj(params, body))
            # print(env.envs)
            rexpr = e_(expr)
            env.exit_scope()
            return rexpr
        
        case CallFun(Variable(varName, i), args):
            # print(Variable(varName, i))
            fun = env.get(f"{varName}:{i}")
            rargs = []
            for arg in args:
                rargs.append(e_(arg))
            
            env.enter_scope()
            for param, arg in zip(fun.params, rargs):
                env.add(f"{param.varName}:{param.id}", arg)
            
            rbody = e_(fun.body)
            env.exit_scope()
            return rbody
            
        case BinOp("+", left, right): return e_(left) + e_(right)
        case BinOp("*", left, right): return e_(left) * e_(right)
        case BinOp("/", left, right): return e_(left) / e_(right)
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
        case UnOp("\u221a", right): return e_(right) ** 0.5 # Square root Symbol
        
        case If(condition, then_body, else_body): 
            if e_(condition):
                return e_(then_body) 
            else:
                return e_(else_body)


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

        if s[i].isalpha():
            start = i
            while i < len(s) and s[i].isalpha():
                i += 1
            name = s[start:i]

            if name in {"if", "then", "else", "end", "let", "in", "letFunc"}:
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
            if s[i:i+2] in {"<=", ">=", "!=", "||", "&&"}:
                prev_token = OperatorToken(s[i:i+2])
                yield prev_token
                i += 2
            elif s[i:i+2] == ":=":
                prev_token = KeyWordToken(":=")
                yield prev_token
                i += 2
            else:
                match s[i]:
                    case '+' | '*' | '/' | '^' | '-' | '(' | ')' | '<' | '>' | '=' | '%' | '\u221a' | "," | "{" | "}":
                        prev_token = OperatorToken(s[i])
                        yield prev_token
                        i += 1
                    case _:
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
    
    def parse_func():
        match peek():
            case KeyWordToken("letFunc"):
                consume(KeyWordToken, "letFunc")
                func_name = consume(VariableToken)
                
                consume(OperatorToken, "(")
                args = []
                if peek() != OperatorToken(")"):
                    while True:
                        args.append(parse_let())
                        if peek() == OperatorToken(","):
                            consume(OperatorToken, ",")
                        else:
                            break
                consume(OperatorToken, ")")
                
                consume(OperatorToken, "{")
                body = parse_func()
                consume(OperatorToken, "}")
                
                consume(KeyWordToken, "in")
                call = parse_func()
                print(call)
                consume(KeyWordToken, "end")
                return LetFun(Variable(func_name.varName), args, body, call)
            
            case _:
                return parse_let()
              
    def parse_let():
        match peek():
            case KeyWordToken("let"):
                consume(KeyWordToken, "let")
                var = Variable(consume(VariableToken).varName)
                consume(KeyWordToken, ":=")
                e1 = parse_func()
                consume(KeyWordToken, "in")
                e2 = parse_func()
                consume(KeyWordToken, "end")
                return Let(var, e1, e2)
            case _:
                return parse_if()
        
    def parse_if():
        match peek():
            case KeyWordToken("if"):
                consume(KeyWordToken, "if")
                condition = parse_func()
                consume(KeyWordToken, "then")
                then_body = parse_func()
                consume(KeyWordToken, "else")
                else_body = parse_func()
                return If(condition, then_body, else_body)
            case _:
                return parse_bool()
    
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
                case OperatorToken("%"):
                    consume()
                    ast = BinOp("%", ast, parse_mul())
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
                        args.append(parse_let())
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
                ast = parse_add()
                consume(OperatorToken, ")")
                return ast
            case _:
                raise ParseErr(f"Unexpected token at index {i}")

    return parse_func()



exp = Let(Variable("a"), Number("3"),
		Let(Variable("b"), BinOp("+", Variable("a"), Number("2")),
		    BinOp("+", Variable("a"), Variable("b"))))

# exp = Let(Variable("a"), Number("3"), BinOp("+", Variable("a"), Variable("a")))

expL = "let a be 3 in let b be a + 2 in a + b end end"
# expL = "let a be 3 in a + a end"

exp = LetFun(Variable("f"), 
             [Variable("x")], 
             BinOp("+", Variable("x"), Number("1")), 
             CallFun(Variable("f"), [Number("2")]))

# letFunc f(x) be
#     x + 1
# in
# f(2)

exp = LetFun(Variable("fact"), 
             [Variable("n")], 
             If(BinOp("=", Variable("n"), Number("0")),
                Number("1"),
                Let(Variable("x"), 
                    CallFun(Variable("fact"), [BinOp("-", Variable("n"), Number("1"))]), 
                    BinOp("*", Variable("n"), Variable("x")))
                ),
             CallFun(Variable("fact"), [Number("5")]))


## PROJECT EULER 1
exp = LetFun(Variable("func"),
             [Variable("x"), Variable("s")],
             If(BinOp("=", Variable("x"), Number("1000")),
                Variable("s"),
                If(BinOp("||", BinOp("=", BinOp("%", Variable("x"), Number("3")), Number("0")), BinOp("=", BinOp("%", Variable("x"), Number("5")), Number("0"))),
                   CallFun(Variable("func"), [BinOp("+", Variable("x"), Number("1")), BinOp("+", Variable("s"), Variable("x"))]),
                   CallFun(Variable("func"), [BinOp("+", Variable("x"), Number("1")), Variable("s")])
                   )
                ),
             CallFun(Variable("func"), [Number("0"), Number("0")]))


## PROJECT EULER 2
exp = LetFun(Variable("fib_sum"),
             [Variable("a"), Variable("b"), Variable("s")],
             If(BinOp(">=", Variable("a"), Number("4000000")),
                Variable("s"),
                If(BinOp("=", BinOp("%", Variable("a"), Number("2")), Number("0")),
                   CallFun(Variable("fib_sum"), [Variable("b"), BinOp("+", Variable("a"), Variable("b")), BinOp("+", Variable("s"), Variable("a"))]),
                   CallFun(Variable("fib_sum"), [Variable("b"), BinOp("+", Variable("a"), Variable("b")), Variable("s")])
                   )
                ),
             CallFun(Variable("fib_sum"), [Number("0"), Number("1"), Number("0")]))

# pprint(exp)
# print()
# res_exp = resolve(exp)
# pprint(res_exp)
# print()
# pprint(e(res_exp))

# Euler Problem 1
exp = """
letFunc func(x, s)
{
     if x = 1000 then
         s
     else if x % 3 = 0 || x % 5 = 0 then
         func(x + 1, s + x)
     else
         func(x + 1, s)
}
in
func(0, 0)
end
"""

# Euler Problem 2
exp = """
letFunc fib(a, b, s)
{
    if a >= 4000000 then
        s
    else if a % 2 = 0 then
        fib(b, a + b, s + a)
    else
        fib(b, a + b, s)
}
in
fib(0, 1, 0)
end
"""

# Factorial
exp = """
letFunc fact(n)
{
    if n = 0 then
        1
    else
        let x := fact(n - 1) in
        n * x
        end
}
in
fact(5)
end
"""

# Fixed this -> added CallFun at the highest precedence in parse_atom()
exp = """
letFunc f(a)
{
    a + a
}
in
f(2) + f(3)
end
"""

exp = """
let x := 5 in
letFunc f(y) {
    x
} 
in
letFunc g(z) { 
    let x := 6 
    in f(z)
    end
}
in
g(0)
end
end
end
"""

exp = """
letFunc f(x)
{
    x ^ 2
}
in
letFunc g(f, y)
{
    f(y) + y
}
in
g(f, 3)
end
end
"""

exp = """
letFunc f(x)
{
    x * 7
}
in
let g := f in
g(3)
end
end
"""

# exp = """
# letFunc g()
# {
#     let x := 5 in
#     letFunc f()
#     {
#         let x := 6 in
#         x
#         end
#     }
#     in
#     f()
#     end
#     x
#     end
# }
# in
# g()
# end
# """

# Wanted something like this -> but need print statement
# def g():
#     x = 5
#     def f():
#         x = 6
#         print(x)
    
#     f()
#     print(x)

# g()

for t in lex(exp):
    print(t)

pprint(parse(exp))
print()
rexp = resolve(parse(exp))
pprint(rexp)
print()
print(e(rexp))
# exit()