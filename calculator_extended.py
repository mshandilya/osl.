from dataclasses import dataclass
from collections.abc import Iterator
from more_itertools import peekable
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
        case If(condition, then_body, else_body): return e(then_body) if e(condition) else e(else_body)
        case UnOp("-", right): return -e(right)
         

# expr = BinOp("+", Number("2"), BinOp("*", Number("3"), Number("4")))
# print(expr)
# print(e(expr))
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
        while i < len(s) and s[i].isspace():
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
                    case '+' | '*' | '/' | '^' | '-' | '(' | ')' | '<' | '>':
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


def visualize_ast(tree: AST, dot=None, parent=None, node_id=0) -> Digraph:
    """
    Visualizes the AST using Graphviz.

    Args:
        tree: The AST to visualize.
        dot: The Digraph instance (created if None).
        parent: The parent node ID (None for the root).
        node_id: The current node ID (unique for each node).

    Returns:
        The Digraph instance.
    """
    if dot is None:
        dot = Digraph()
        dot.attr("node", shape="circle")

    current_id = str(node_id)

    if isinstance(tree, Number):
        label = f"Number: {tree.val}"
        dot.node(current_id, label=label, shape="ellipse")
    elif isinstance(tree, BinOp):
        label = f"BinOp: {tree.op}"
        dot.node(current_id, label=label)
        visualize_ast(tree.left, dot, current_id, node_id * 2 + 1)
        visualize_ast(tree.right, dot, current_id, node_id * 2 + 2)
    elif isinstance(tree, UnOp):
        label = f"UnOp: {tree.op}"
        dot.node(current_id, label=label)
        visualize_ast(tree.right, dot, current_id, node_id * 2 + 1)
    elif isinstance(tree, If):
        dot.node(current_id, label="If")

        # Create child nodes for Condition, Then, and Else
        condition_id = str(node_id * 2 + 1)
        then_id = str(node_id * 2 + 2)
        else_id = str(node_id * 2 + 3)

        # Add labels for Condition, Then, Else branches
        dot.node(condition_id, label="Condition", shape="diamond")
        dot.node(then_id, label="Then", shape="box")
        dot.node(else_id, label="Else", shape="box")

        # Draw edges from the If node to these branches
        dot.edge(current_id, condition_id, label="condition")
        dot.edge(current_id, then_id, label="then")
        dot.edge(current_id, else_id, label="else")

        # Visualize subtrees
        visualize_ast(tree.condition, dot, condition_id, node_id * 4 + 1)
        visualize_ast(tree.then_body, dot, then_id, node_id * 4 + 2)
        visualize_ast(tree.else_body, dot, else_id, node_id * 4 + 3)

    if parent is not None:
        dot.edge(parent, current_id)

    return dot


print(parse("2"))
print(parse("2+3"))
print(parse("2+3*5"))
print()
print(e(parse("2 + 3*5")))
print()

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
print()
print(parse("-((4*5)-(4/5))"))
print(e(parse("-((4*5)-(4/5))")))
ast = parse("-((4*5)-(4/5))")
dot = visualize_ast(ast)
dot.render("ast", format="png", cleanup=True)

try:
    print()
    print("-((4*5-(4/5))")
    print(parse("-((4*5-(4/5))")) # Will throw error "Expected closing bracket"
    print(e(parse("-((4*5-(4/5))")))
except Exception as ep:
    print(ep)
    print()

try:
    print()
    print("2 +")
    print(parse("2 +")) # Will throw error "Unexpected end of input"
    print(e(parse("2 +")))
except Exception as ep:
    print(ep)
    print()

try:
    print()
    print("2 + * 3")
    print(parse("2 + * 3")) # Will throw error "Expected Token, got OperatorToken"
    print(e(parse("2 + * 3")))
except Exception as ep:
    print(ep)
    print()
    
try:
    print()
    print("2 + $ 3")
    print(parse("2 + $ 3"))
    print(e(parse("2 + $ 3")))
except Exception as ep:
    print(ep)
    print()
    
print()
print(parse("if 2 < 3 then 2 else 3 end"))
print(e(parse("if 2 < 3 then 2 else 3 end")))

print()
print(parse("if 2 < 3 then 0+5 else 1*6 end"))
ast = parse("if 2 < 3 then 0+5 else 1*6 end")
dot = visualize_ast(ast)
dot.render("ast_cond", format="png", cleanup=True)
print(e(parse("if 2 < 3 then 0+5 else 1*6 end")))

exp = "if 2 < 3 then if 4 > 5 then 1 else if 6 <= 7 then 8 else 9 end end else 10 end"
print()
print(parse(exp))
print(e(parse(exp)))
ast = parse(exp)
dot = visualize_ast(ast)
dot.render("ast_nested_cond", format="png", cleanup=True)

print()
print(parse("1 < 2 <= 3 == 4"))
print(e(parse("1 < 2 <= 3 == 4")))