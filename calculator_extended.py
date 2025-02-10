from dataclasses import dataclass
from collections.abc import Iterator
from more_itertools import peekable
from pprint import pprint
from graphviz import Digraph # type: ignore
from typing import List

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
    
    def make(name):
        return Variable(name, fresh())
    
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
        
        case BinOp(op, left, right):
            le = resolve_(left)
            ri = resolve_(right)
            return BinOp(op, le, ri)
        case UnOp(op, right):
            ri = resolve_(right)
            return UnOp(op, ri)
        


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
            env.add(f"{varName}:{i}", v1) # CHECK THIS OUT ONCE! v or varName
            v2 = e_(e2)
            env.exit_scope()
            return v2
        
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
        case UnOp("\u221a", right): return e_(right) ** 0.5 # Square root Symbol
        case If(condition, then_body, else_body): 
            if e_(condition):
                return e_(then_body) 
            else:
                e_(else_body)
                
        case UnOp("-", right): return -e_(right)


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
            if t in {"if", "then", "else", "end", "let", "be", "in"}:
                yield KeyWordToken(t)
            else:
                yield VariableToken(t)
        
        elif s[i].isdigit():
            t = s[i]
            i = i + 1
            while i < len(s) and (s[i].isdigit() or s[i] == '.'):
                t = t + s[i]
                i = i + 1
            yield NumberToken(t)
        else:
            # Handle multi-character operators first
            if s[i:i+2] in {"<=", ">=", "!=", ":="}:
                yield OperatorToken(s[i:i+2])
                i += 2
            else:
                match t := s[i]:
                    case '+' | '*' | '/' | '^' | '-' | '(' | ')' | '<' | '>' | '=' | '%' | '\u221a':
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
        if expected_value and getattr(token, "op", None) != expected_value and getattr(token, "varName", None) != expected_value:
            raise ParseErr(f"Expected '{expected_value}' at index {i}, got '{getattr(token, 'op', getattr(token, 'varName', None))}'")
        i += len(str(token.val if isinstance(token, NumberToken) else getattr(token, "op", getattr(token, "varName", ""))))
        return token
    
    def peek():
        return t.peek(None)
    
    def parse_let():
        match peek():
            case KeyWordToken("let"):
                consume(KeyWordToken, "let")
                var = consume(VariableToken)
                consume(KeyWordToken, "be")
                e1 = parse_let()
                consume(KeyWordToken, "in")
                e2 = parse_let()
                consume(KeyWordToken, "end")
                return Let(Variable(var.varName), e1, e2)
            case _:
                return parse_if()
    
    def parse_if():
        match peek():
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

    return parse_let()



exp = Let(Variable("a"), Number("3"),
		Let(Variable("b"), BinOp("+", Variable("a"), Number("2")),
		    BinOp("+", Variable("a"), Variable("b"))))

# exp = Let(Variable("a"), Number("3"), BinOp("+", Variable("a"), Variable("a")))

expL = "let a be 3 in let b be a + 2 in a + b end end"
# expL = "let a be 3 in a + a end"

pprint(exp)
print()
for l in lex(expL):
    print(l)
print()
pprint(resolve(exp))
print()
pprint(parse(expL))
print()
pprint(resolve(parse(expL)))
print()
print(e(parse(expL)))
print(e(resolve(parse(expL))))
# print(e(exp))
# print(e(resolve(exp)))

exit()


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
# ast = parse(exp_sq)
# dot = visualize_ast(ast)
# dot.render("./imgaes/ast_sqrt", format="png", cleanup=True)
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

# exp = "2<3<2"
# unit_test(exp, False, log)

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

# exp = "1 < 2 <= 3 == 4"
# unit_test(exp, False, log)

print("\nTest Summary:")
for status, expr, error_msg in log:
    if status == "PASSED":
        print(f"PASSED: {expr}")
    else:
        print(f"{status}: {expr}")
        if error_msg:
            print(f"  -> {error_msg}")

# ast = parse(exp_cond)
# dot = visualize_ast(ast)
# dot.render("./imgaes/ast_nested_cond", format="png", cleanup=True)

# ast = parse("-((4*5)-(4/5))")
# dot = visualize_ast(ast)
# dot.render("./imgaes/ast", format="png", cleanup=True)

# ast = parse(exp_cond1)
# dot = visualize_ast(ast)
# dot.render("./imgaes/ast_cond", format="png", cleanup=True)

# program = open("program.txt", "r").read()
# print(program)
