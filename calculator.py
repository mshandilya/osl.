from dataclasses import dataclass

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
    val: str
    
@dataclass
class UnOp(AST):
    op: str
    right: AST


expr = BinOp("+", Number("2"), BinOp("*", Number("3"), Number("4")))
print(expr)

def e(tree: AST) -> float:
    match tree:
        case Number(val): return float(val)
        case BinOp("+", left, right): return e(left) + e(right)
        case BinOp("*", left, right): return e(left) * e(right)
        case BinOp("/", left, right): return e(left) / e(right)
        case BinOp("^", left, right): return e(left) ** e(right)
        case BinOp("-", left, right): return e(left) - e(right)
        case UnOp("-", right): return -e(right)
         

print(e(expr))


class Token:
    pass

@dataclass
class NumberToken(Token):
    val: str

@dataclass
class OperatorToken(Token):
    op: str
    
    
from collections.abc import Iterator
def lex(s: str) -> Iterator[Token]:
    i = 0
    while True:
        while i < len(s) and s[i].isspace():
            i = i + 1

        if i >= len(s):
            return

        if s[i].isdigit():
            t = s[i]
            i = i + 1
            while i < len(s) and (s[i].isdigit() or s[i] == '.'):
                t = t + s[i]
                i = i + 1
            yield NumberToken(t)
        else:
            match t := s[i]:
                case '+' | '*' | '/' | '^' | '-' | '(' | ')':
                    i = i + 1
                    yield OperatorToken(t)
                    
                    
                    
                    
from collections.abc import Iterator
def parse(s: str) -> AST:
    from more_itertools import peekable
    t = peekable(lex(s))

    def parse_add():
        ast = parse_mul()
        while True:
            match t.peek(None):
                case OperatorToken('+'):
                    next(t)
                    ast = BinOp('+', ast, parse_mul())
                case OperatorToken('-'):
                    next(t)
                    ast = BinOp('-', ast, parse_mul())
                case _:
                    return ast
        
    def parse_mul():
        ast = parse_exponentiation()
        while True:
            match t.peek(None):
                case OperatorToken('*'):
                    next(t)
                    ast = BinOp("*", ast, parse_exponentiation())
                case OperatorToken('/'):
                    next(t)
                    ast = BinOp("/", ast, parse_exponentiation())
                case _:
                    return ast
    
    def parse_exponentiation():
        ast = parse_atom()
        while True:
            match t.peek(None):
                case OperatorToken('^'):
                    next(t)
                    ast = BinOp("^", ast, parse_exponentiation())
                case _:
                    return ast
    
    def parse_atom():
        match t.peek(None):
            case NumberToken(v):
                next(t)
                return Number(v)
            case OperatorToken('-'):
                next(t)
                return UnOp("-", parse_atom())
            case OperatorToken('('):
                next(t)
                ast = parse_add()
                if t.peek(None) != OperatorToken(')'):
                    raise ValueError("Expected closing bracket")
                next(t)
                return ast
            case _:
                raise ValueError("Expected Token")
            
    

    return parse_add()

print(parse("2"))
print(parse("2+3"))
print(parse("2+3*5"))
print()
print(e(parse("2 + 3*5")))
print()

# expr = BinOp("+", Number("2"), BinOp("*", Number("3"), BinOp("^", Number("5"), Number("2"))))
# print(expr)
# print(e(expr))
print(parse("2 + 3*5^2"))
print(e(parse("2 + 3*5^2")))
print()
print(parse("2 + 3*5^2 - 3"))
print(e(parse("2 + 3*5^2 - 3")))
print()

print(e(parse("2 + 3*5^2 - 3 / 2")))
print()
# decimal numbers
print(parse("2.5 + 3.5"))
print(e(parse("2.5 + 3.5")))
print()
print(parse("2^3^2"))
print(e(parse("2^3^2")))
print()
# mix of decimal and exponentiation
print(parse("2.5^3^2"))
print(e(parse("2.5^3^2")))
print()
print(parse("3 + 2*3.5^25/5"))
print(e(parse("3 + 2*3.5^4/2")))
print()
print(parse("2-3-5-6"))
print(e(parse("2-3-5-6")))
print()
print(parse("2/3/5"))
print(e(parse("2/3/5")))
print()
# take brackets into account
print(parse("((2+3)*5)/5"))
print(e(parse("((2+3)*5)/5")))
print()
print(parse("3-(5-6)"))
print(e(parse("3-(5-6)")))
print()
print(parse("3*(5-6^3)"))
print(e(parse("3*(5-6^3)")))
print()
print(parse("2- -2"))
print(e(parse("2- -2")))
print()
print(parse("2- -(3 - 1)"))
print(e(parse("2- -(3 - 1)")))
print(parse("5*-5"))
print(e(parse("5*-5")))
print()
print(parse("-5*5"))
print(e(parse("-5*5")))
print()
print(parse("2 +-3"))
print(e(parse("2 +-3")))
print()
print(parse("-(5-2)"))
print(e(parse("-(5-2)")))