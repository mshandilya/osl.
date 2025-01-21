from pprint import pprint

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
class If(AST):
    condition_: AST
    then_: AST
    else_: AST

def e(tree: AST) -> float:
    match tree:
        case Number(v): return float(v)
        case BinOp("+", l, r): return e(l) + e(r)
        case BinOp("-", l, r): return e(l) - e(r)
        case BinOp("*", l, r): return e(l) * e(r)
        case BinOp("/", l, r): return e(l) / e(r)
        case BinOp("^", l, r): return e(l) ** e(r)
        case BinOp("<", l, r): return e(l) < e(r)
        case BinOp(">", l, r): return e(l) > e(r)
        case BinOp("<=", l, r): return e(l) <= e(r)
        case BinOp(">=", l, r): return e(l) >= e(r)
        case BinOp("==", l, r): return e(l) == e(r)
        case UnOp("-", v): return -e(v)
        case If(c, t, e_):
            if e(c):
                return e(t)
            else:
                return e(e_)

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
            # To do: Check for valid keywords
            yield KeywordToken(t)

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
                if(type(t.peek(None)) == KeywordToken):
                    raise SyntaxError(f"{t.peek(None)} is not a valid keyword")
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

    return parse_if()

def evall(s: str, val) -> AST:
    print("Calculate:", s)
    pprint(parse(s))
    result = e(parse(s))
    print(result)
    if(result != val):
        raise ValueError(f"\n-----------------------\nTest failed for {s}:\nExpected {val} got {result}")
    print("-----------------------")

# Tests

# print(parse("2"))

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

print("All tests passed!")

# print(e(parse("9*8/3*5")))

# print(e(parse("(2^3)^5")))