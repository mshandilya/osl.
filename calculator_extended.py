from dataclasses import dataclass
from collections.abc import Iterator
from more_itertools import peekable
from pprint import pprint
from graphviz import Digraph # type: ignore

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
class StringLiteral(AST):
    val: str

@dataclass
class UnOp(AST):
    op: str
    right: AST

class ParseErr(Exception):
    pass

@dataclass
class If(AST):
    condition: AST
    then_body: AST
    else_body: AST

def e(tree: AST) -> int | float | bool:
    match tree:
        case Number(val): return val
        case BinOp("+", left, right): return e(left) + e(right)
        case BinOp("*", left, right): return e(left) * e(right)
        case BinOp("/", left, right): return e(left) / e(right)
        case BinOp("^", left, right): return e(left) ** e(right)
        case BinOp("-", left, right): return e(left) - e(right)
        case BinOp("<", left, right): return e(left) < e(right)
        case BinOp(">", left, right): return e(left) > e(right)
        case BinOp("<=", left, right): return e(left) <= e(right)
        case BinOp(">=", left, right): return e(left) >= e(right)
        case BinOp("==", left, right): return e(left) == e(right)
        case BinOp("!=", left, right): return e(left) != e(right)
        case UnOp("\u221a", right): return e(right) ** 0.5 # Square root Symbol
        case If(condition, then_body, else_body): return e(then_body) if e(condition) else e(else_body)
        case UnOp("-", right): return -e(right)
         
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


def lex(s: str) -> Iterator[Token]:
    i = 0
    while True:
        while i < len(s) and s[i].isspace(): # newlines, spaces, tabs handled
            i = i + 1

        if i >= len(s):
            return

        if s[i].isalpha():
            t = s[i]
            i = i + 1
            while i < len(s) and s[i].isalpha():
                t = t + s[i]
                i = i + 1
            yield KeyWordToken(t)
        
        elif s[i].isdigit():
            t = s[i]
            i = i + 1
            while i < len(s) and (s[i].isdigit() or s[i] == '.'):
                t = t + s[i]
                i = i + 1
            yield NumberToken(t)
        else:
            # Handle multi-character operators first
            if s[i:i+2] in {"<=", ">=", "==", "!="}:
                yield OperatorToken(s[i:i+2])
                i += 2
            else:
                match t := s[i]:
                    case '+' | '*' | '/' | '^' | '-' | '(' | ')' | '<' | '>' | '\u221a':
                        i = i + 1
                        yield OperatorToken(t)
                    case _:
                        raise ParseErr(f"Unexpected character: {t} at position {i}")
      
def parse(s: str) -> AST:
    t = peekable(lex(s))
    i = 0
    
    def consume(expected_type=None, expected_value=None):
        nonlocal i  # Track the current index
        token = next(t, None)
        if token is None:
            raise ParseErr(f"Unexpected end of input at index {i}")
        if expected_type and not isinstance(token, expected_type):
            raise ParseErr(f"Expected {expected_type.__name__} at index {i}, got {type(token).__name__}")
        if expected_value and getattr(token, "op", None) != expected_value:
            raise ParseErr(f"Expected '{expected_value}' at index {i}, got '{token.op}'")
        i += len(str(token.val if isinstance(token, NumberToken) else token.op))
        return token
    
    def peek():
        return t.peek(None)
    
    def parse_if():
        match t.peek(None):
            case KeyWordToken("if"):
                consume(KeyWordToken, "if")
                condition = parse_if()
                if peek() != KeyWordToken("then"):
                    raise ParseErr(f"Expected 'then' at index {i}")
                consume(KeyWordToken, "then")
                then_body = parse_if()
                if peek() != KeyWordToken("else"):
                    raise ParseErr(f"Expected 'else' at index {i}")
                consume(KeyWordToken, "else")
                else_body = parse_if()
                if peek() != KeyWordToken("end"):
                    raise ParseErr(f"Expected 'end' at index {i}")
                consume(KeyWordToken, "end")  # Consume the 'end' token
                return If(condition, then_body, else_body)
            case _:
                return parse_comparison()
            
    def parse_comparison():
        ast = parse_add()
        while True:
            match peek():
                case OperatorToken(op) if op in {"<", ">", "<=", ">=", "==", "!="}:
                    consume()
                    ast = BinOp(op, ast, parse_add())
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
            case OperatorToken('-'):
                consume()
                return UnOp("-", parse_atom())
            case OperatorToken('\u221a'):
                consume()
                return UnOp("\u221a", parse_atom())
            case OperatorToken('('):
                start_index = i
                consume()
                ast = parse_add()
                if peek() != OperatorToken(')'):
                    raise ParseErr(f"Expected closing parenthesis at index {i} (opened at index {start_index})")
                consume()
                return ast
            case _:
                raise ParseErr(f"Unexpected token at index {i}")

    return parse_if()


unique_id = 0
def get_unique_id():
    global unique_id
    unique_id += 1
    return str(unique_id)

def visualize_ast(tree: AST, dot=None, parent=None):
    if dot is None:
        dot = Digraph()
        dot.attr("node", shape="circle")

    current_id = get_unique_id()

    if isinstance(tree, Number):
        dot.node(current_id, label=str(tree.val))
        
    elif isinstance(tree, BinOp):
        dot.node(current_id, label=tree.op)
        visualize_ast(tree.left, dot, current_id)
        visualize_ast(tree.right, dot, current_id)
        
    elif isinstance(tree, UnOp):
        dot.node(current_id, label=tree.op)
        visualize_ast(tree.right, dot, current_id)
        
    elif isinstance(tree, If):
    
        dot.node(current_id, label="if")
        
        condition_id = get_unique_id()
        dot.node(condition_id, label="condition", shape="diamond")
        dot.edge(current_id, condition_id)
        visualize_ast(tree.condition, dot, condition_id)
        
        then_id = get_unique_id()
        dot.node(then_id, label="then", shape="box")
        dot.edge(current_id, then_id)
        visualize_ast(tree.then_body, dot, then_id)
        
        else_id = get_unique_id()
        dot.node(else_id, label="else", shape="box")
        dot.edge(current_id, else_id)
        visualize_ast(tree.else_body, dot, else_id)

    if parent is not None:
        dot.edge(parent, current_id)

    return dot

def unit_test(expr: str, expected_value, results):
    print(f"Expression: {expr}")
    try:
        ast = parse(expr)
        pprint(ast)
        result = e(ast)
        print(f"Evaluated Result: {result}")
        if result != expected_value:
            error_msg = f"Test failed for expression: {expr}. Expected {expected_value}, but got {result}."
            results.append(("FAILED", expr, error_msg))
            print(error_msg)
            print()
        else:
            results.append(("PASSED", expr, None))
            print("Test passed!\n")
    except Exception as ep:
        error_msg = f"Error evaluating expression: {expr}. Exception: {ep}"
        results.append(("ERROR", expr, error_msg))
        print(error_msg)
        print()

log = []

unit_test("2", 2, log)
unit_test("2+3", 5, log)
unit_test("2+3*5", 17, log)
unit_test("2 + 3*5", 17, log)
unit_test("2 + 3*5^2", 77, log)
unit_test("2 + 3*5^2 - 3", 74, log)
unit_test("2 + 3*5^2 - 3 / 2", 75.5, log)
unit_test("2.5 + 3.5", 6.0, log)
unit_test("2^3^2", 512, log)
unit_test("2.5^3^2", 3814.697265625, log)
unit_test("3 + 2*3.5^4/2", 153.0625, log)
unit_test("2-3-5-6", -12, log)
unit_test("2/3/5", 0.13333333333333333, log)
unit_test("((2+3)*5)/5", 5.0, log)
unit_test("3-(5-6)", 4, log)
unit_test("3*(5-6^3)", -633, log)
unit_test("2- -2", 4, log)
unit_test("2- -(3 - 1)", 4, log)
unit_test("5*-5", -25, log)
unit_test("-5*5", -25, log)
unit_test("2 +-3", -1, log)
unit_test("-(5-2)", -3, log)
unit_test("-((4*5)-(4/5))", -19.2, log)
unit_test("\u221a(4)", 2, log)

exp_sq = "\u221a(4 + 12) + \u221a(9)"
ast = parse(exp_sq)
dot = visualize_ast(ast)
dot.render("ast_sqrt", format="png", cleanup=True)
unit_test(exp_sq, 7.0, log)

exp_cond1 = """
if 2 < 3 then
    0 + 5
else
    1 * 6
end
"""
unit_test("if 2 < 3 then 2 else 3 end", 2, log)
unit_test(exp_cond1, 5, log)

exp = "2<3<2"
unit_test(exp, False, log)

exp_cond = """
if 2 < 3 then 
    if 4 > 5 then 
        1 
    else 
        if 6 <= 7 then 
            8 
        else 
            9 
        end 
    end 
else 
    10 
end
"""
unit_test(exp_cond, 8, log)

exp = "1 < 2 <= 3 == 4"
unit_test(exp, False, log)

print("\nTest Summary:")
for status, expr, error_msg in log:
    if status == "PASSED":
        print(f"PASSED: {expr}")
    else:
        print(f"{status}: {expr}")
        if error_msg:
            print(f"  -> {error_msg}")

ast = parse(exp_cond)
dot = visualize_ast(ast)
dot.render("ast_nested_cond", format="png", cleanup=True)

ast = parse("-((4*5)-(4/5))")
dot = visualize_ast(ast)
dot.render("ast", format="png", cleanup=True)

ast = parse(exp_cond1)
dot = visualize_ast(ast)
dot.render("ast_cond", format="png", cleanup=True)

# program = open("program.txt", "r").read()
# print(program)
