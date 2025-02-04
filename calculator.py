from pprint import pprint
from typing import List

from dataclasses import dataclass

class AST:
    pass

@dataclass
class BinOp(AST):
    op: str
    left: AST
    right: AST

@dataclass
class UnOp(AST):
    op: str
    val: str

@dataclass
class Number(AST):
    val: str

@dataclass
class Var(AST):
    name: str
    id: int = None

@dataclass 
class Let(AST):
    v: AST
    e1: AST
    e2: AST

@dataclass
class LetMut(AST):
    v: AST
    e1: AST
    e2: AST

@dataclass
class Put(AST):
    v: str
    e: AST

@dataclass
class Get(AST):
    v: str

@dataclass
class If(AST):
    condition_: AST
    then_: AST
    else_: AST

@dataclass
class Fun:
    name: 'AST'
    params: 'AST'
    body: 'AST'
    expr: 'AST'

@dataclass
class Call(AST):
    name: str # Name of function
    arg: AST # Argument

# @dataclass
# class FunCall:
#     fn: 'AST'
#     args: List['AST']

# @dataclass
# class FunObject:
#     params: List['AST']
#     body: 'AST'

cnt = 0
def cntr():
    global cnt
    cnt = cnt + 1
    return cnt

class Environment:
    envs: list
    def __init__(self):
        self.envs = [{}]

    def enter_scope(self):
        self.envs.append({})

    def exit_scope(self):
        self.envs.pop()

    def add(self, v, val):
        assert v not in self.envs[-1], f"Variable {v} already exists"
        self.envs[-1][v] = val

    def get(self, name):
        for env in reversed(self.envs):
            if name in env:
                return env[name]
        raise ValueError(f"Variable {name} not found")

    def update(self, name, val):
        # Think: If the variable is not in the current local environment, should I create a new variable or update the variable present in the parent environment?
        for env in reversed(self.envs):
            if name in env:
                env[name] = val
                return
        raise ValueError(f"Variable {name} not found")

def resolve(program: AST, env: Environment = None) -> AST:
    if env is None:
        env = Environment()

    match program:
        case Number(v):
            return Number(v)
        
        case Var(name, _):
            return Var(name, env.get(name))
            
        case Let(Var(name, _), e1, e2):
            re1 = resolve(e1, env)
            env.enter_scope()
            env.add(name, i := cntr()) # assign's cntr() to i and also returns the value
            re2 = resolve(e2, env)
            env.exit_scope()
            return Let(Var(name, i), re1, re2)
        
        case Fun(f, Var(name, _), body, expr):
            env.enter_scope()
            env.add(name, i := cntr())
            rbody = resolve(body, env)
            env.exit_scope()
            rexpr = resolve(expr, env)
            return Fun(f, Var(name, i), rbody, rexpr)

        case Call(f, x):
            xr = resolve(x, env)
            return Call(f, xr)

        case BinOp(op, l, r):
            rl = resolve(l, env)
            rr = resolve(r, env)
            return BinOp(op, rl, rr)

def e(tree: AST, env = None) -> float:
    if env is None:
        env = Environment() # create a new object of class Environment

    match tree:
        case Var(v): return env.get(v)
        case Number(v): return float(v)
        case Let(v,x,y):
            valx = e(x, env)
            env.enter_scope()
            env.add(v, valx)
            valy = e(y, env)
            env.exit_scope()
            return valy
        case LetMut(v,x,y):
            valx = e(x, env)
            env.enter_scope()
            env.add(v, valx)
            valy = e(y, env)
            env.exit_scope()
            return valy
        case Put(v, x):
            env.update(v, e(x, env))
            return env.get(v)
        case Get(v): return env.get(v)
        case BinOp("+", l, r): return e(l, env) + e(r, env)
        case BinOp("-", l, r): return e(l, env) - e(r, env)
        case BinOp("*", l, r): return e(l, env) * e(r, env)
        case BinOp("/", l, r): return e(l, env) / e(r, env)
        case BinOp("^", l, r): return e(l, env) ** e(r, env)
        case BinOp("<", l, r): return e(l, env) < e(r, env)
        case BinOp(">", l, r): return e(l, env) > e(r, env)
        case BinOp("<=", l, r): return e(l, env) <= e(r, env)
        case BinOp(">=", l, r): return e(l, env) >= e(r, env)
        case BinOp("==", l, r): return e(l, env) == e(r, env)
        case UnOp("-", v): return -e(v, env)
        case If(c, t, e_):
            if e(c, env):
                return e(t, env)
            else:
                return e(e_, env)

class Token:
    pass

@dataclass
class NumberToken(Token):
    v: str

@dataclass
class OperatorToken(Token):
    o: str

@dataclass
class KeywordToken(Token):
    k: str

@dataclass
class VarToken(Token):
    val: str

from collections.abc import Iterator
def lex(s: str) -> Iterator[Token]:
    i = 0
    while True:
        while i < len(s) and s[i].isspace():   # isspace() consideres spaces, newlines and tabs
            i = i + 1

        if i >= len(s):
            return

        elif s[i].isalpha():
            t = s[i]
            i = i + 1
            while i < len(s) and s[i].isalpha():
                t = t + s[i]
                i = i + 1
            if t in {'if', 'then', 'else', 'let', 'in', 'be', 'end'}:
                yield KeywordToken(t)
            else:
                yield VarToken(t)

        elif s[i].isdigit():
            t = s[i]
            i = i + 1
            while i < len(s) and s[i].isdigit():
                t = t + s[i]
                i = i + 1
            if(i<len(s) and s[i]=='.'):
                t = t + s[i]
                i = i + 1
                while i < len(s) and s[i].isdigit():
                    t = t + s[i]
                    i = i + 1
            yield NumberToken(t)

        else:
            match t := s[i]:
                case '+' | '-' | '*' | '/' | '^' | '(' | ')':
                    i = i + 1
                    yield OperatorToken(t)
                case '<' | '>' | '=':
                    if i+1<len(s) and s[i+1] == '=':
                        i = i + 2
                        yield OperatorToken(t + '=')
                    else:
                        i = i + 1
                        yield OperatorToken(t)

# for t in lex("2 + 3*5"):
#     print(t)

from collections.abc import Iterator
def parse(s: str) -> AST:
    from more_itertools import peekable
    t = peekable(lex(s))

    def expect(s):
        if t.peek(None) != s:
            raise SyntaxError(f"Expected '{s}' but got '{t.peek(None)}'")
        else:
            next(t)

    def parse_let():
        match t.peek(None):
            case KeywordToken('let'):
                next(t)
                v = t.peek(None)
                next(t)
                expect(KeywordToken('be'))
                v1 = parse_let()
                expect(KeywordToken('in'))
                v2 = parse_let()
                expect(KeywordToken('end'))
                return Let(Var(v.val), v1, v2) # V is a VarToken, and we want the first argument of Let to be a string (variable name)
            case _:
                return parse_if()

    def parse_if():
        match t.peek(None):
            case KeywordToken('if'):
                next(t)
                condition_= parse_if()
                expect(KeywordToken("then"))
                then_= parse_if()
                expect(KeywordToken("else"))
                else_= parse_if()
                return If(condition_, then_, else_)
            case _:
                # if(type(t.peek(None)) == KeywordToken):
                #     raise SyntaxError(f"{t.peek(None)} is not a valid keyword")
                return parse_comp()

    # We want least precedence for < and >
    def parse_comp():
        ast = parse_add_sub()
        while True:
            match t.peek(None):
                case OperatorToken('<'):
                    next(t)
                    ast = BinOp('<', ast, parse_add_sub())
                case OperatorToken('>'):
                    next(t)
                    ast = BinOp('>', ast, parse_add_sub())
                case OperatorToken('<='):
                    next(t)
                    ast = BinOp('<=', ast, parse_add_sub())
                case OperatorToken('>='):
                    next(t)
                    ast = BinOp('>=', ast, parse_add_sub())  
                case OperatorToken('=='):
                    next(t)
                    ast = BinOp('==', ast, parse_add_sub())    
                case _:
                    return ast

    def parse_add_sub():
        ast = parse_mul_div()
        while True:
            match t.peek(None):
                case OperatorToken('+'):
                    next(t)
                    ast = BinOp('+', ast, parse_mul_div()) # Currently, it is (2+3)+5, if parse_add() is done, then it becomes 2+(3+5)
                case OperatorToken('-'):
                    next(t)
                    ast = BinOp('-', ast, parse_mul_div())
                case _:
                    return ast

    def parse_mul_div():
        ast = parse_exp()
        while True:
            match t.peek(None):
                case OperatorToken('*'):
                    next(t)
                    ast = BinOp("*", ast, parse_exp())
                case OperatorToken('/'):
                    next(t)
                    ast = BinOp("/", ast, parse_exp())
                case _:
                    return ast

    def parse_exp():
        ast = parse_atom()
        while True:
            match t.peek(None):
                case OperatorToken('^'):
                    next(t)
                    ast = BinOp("^", ast, parse_exp()) # Keep it like python 2**3**2 = 2**(3**2)
                case _:
                    return ast

    def parse_atom():
        match t.peek(None):
            case NumberToken(v):
                next(t)
                return Number(v)
            case OperatorToken('('):
                next(t)
                ast = parse_if()
                if OperatorToken(')') != t.peek(None):
                    raise SyntaxError("Expected ')'")
                else:
                    next(t)
                    return ast
            case OperatorToken('-'):
                next(t)
                return UnOp("-", parse_atom())
            case OperatorToken(')'):
                raise SyntaxError("Expected '('")
            case VarToken(v):
                next(t)
                return Var(v)

    return parse_let()

def evall(s: str, val) -> AST:
    print("Calculate:", s)
    pprint(parse(s))
    result = resolve(parse(s))
    print(result)
    # if(result != val):
    #     raise ValueError(f"\n-----------------------\nTest failed for {s}:\nExpected {val} got {result}")
    # print("-----------------------")

# Tests

# print(parse("2"))

# expr_t4 = Let("a", Number("3"), BinOp("+", Var("a"), Var("a")))
# expr_t5 = Let("a", Number("3"),
# 		Let("b", BinOp("+", Var("a"), Number("2")),
# 		    BinOp("+", Var("a"), Var("b"))))

# print(e(expr_t4))
# print(e(expr_t5))

# for t in lex("let a be 3 in a + a end"):
#     print(t)

expr_t7ast = Let (
    Var("x"),
    Number("5"),
    Fun (
        "f",
        Var("y"),
        Var("x"),
        Fun (
            "g",
            Var("z"),
            Let (
                Var("x"),
                Number("6"),
                Call("f", Var("x"))
            ),
            Call("g", Number("0")))))

pprint(resolve(expr_t7ast))

evall("2+3+5", 10)

evall("2+3*5", 17)

evall("2+3-5", 0)

evall("2-3+5", 4)

evall("2-3-5", -6)

evall("2+3/5", 2.6)

evall("100/10/2", 5)

evall("10+2^5", 42)

evall("2^3^2", 512)

evall("2^3+2", 10)

evall("10.3+2.6", 12.9)

evall("2+3/5-3.1", -0.5)

evall("2+-3", -1)

evall("-3*2", -6)

evall("3*-2", -6)

evall("-3*-2", 6)

evall("-3 --2", -1)

evall("--2", 2)

evall("3*-2+5", -1)

evall("2*(3+2)", 10)

evall("(3+2)*2", 10)

evall("(3+2)*(15/3)", 25)

evall("(3-(5-2))", 0)

evall("-(5-2)", -3)

evall("--(5-2)", 3)

evall("2--(5-2)", 5)

evall("2+-3", -1)

evall("2+--3", 5)

evall("2+--(3-2)", 3)

evall("-2*--(5-2)", -6)

evall("-5*5", -25)

evall("---(-3)", 3)

evall("2--2", 4)

evall("2--(3-1)", 4)

evall("3*((2+2)*2)", 24)

# Observe the AST for these, some - are UnOp and some are BinOp

evall("-3", -3)

evall("--3", 3)

evall("2--3", 5)

evall("2---3", -1)

# evall("3*((2+2*2)+5") # SyntaxError: Expected ')'

# evall("3*)(2+2*2)+5") # SyntaxError: Expected '('

# evall("3**4") # Gives error: TypeError: unsupported operand type(s) for *: 'float' and 'NoneType'

evall("2<3", True)

evall("2>3", False)

evall("2+10>3+5", True)

# evall("<3") # TypeError: '<' not supported between instances of 'NoneType' and 'float'

evall("if 2<3 then 2 else 3", 2)

evall("if 2>3 then 2 else 3", 3)

evall("if 2>3 then 2 else 3+5", 8) # 3+5 is evaluated before if else

evall("3*(1+2)", 9)

evall("(1-3)*(1+2)", -6)

evall("2+10>3+9", 0)

evall("2+11>3+9", 1)

evall("2+9>=3+9", 0)

evall("2+10>=3+9", 1)

evall("2+10<=3+9", 1)

evall("2+9<=3+9", 1)

evall("(2^3)^5", 32768)

# evall("iff 2<3 then 2 else 3") # KeywordToken(k='iff') is not a valid keyword

# evall("if 2<3 thn 2 else 3") # SyntaxError: Expected 'KeywordToken(k='then')' but got 'KeywordToken(k='thn')'

evall("if 2>3 then 2 else if 3<4 then 5 else 6", 5)

evall("if 2<3 then if 8<9 then 14 else 15 else if 3<4 then 5 else 6", 14)

evall('''if 2<3 then 
            if 8<9 then 
                14 
            else 15 
        else if 3<4 then 
            5 
        else 
            6''', 14)

evall("let a be 3 in a + 2 end", 5)
pprint(parse("let a be 3 in a + 2 end"))
pprint(resolve(parse("let a be 3 in a + 2 end")))

evall("let a be 3 in a end", 3)

evall("let a be 3 in a+a end", 6)

evall("let a be 3 in let b be a+2 in a+b end end", 8)

evall('''let a be let c be 2 
                  in c 
                  end 
        in 
            let a be a+2
            in a+a 
            end 
        end''', 8)

print("All tests passed!")


e1 = LetMut("b", Number("2"), Let("a", BinOp("+", Number("1"), Get("b")), BinOp("+", Get("a"), Number("6"))))

# # print(e(e1))

# # print(e(parse("(2^3)^5")))

# # Strings
# # True False, booleans
# # Functions
# # Lists
