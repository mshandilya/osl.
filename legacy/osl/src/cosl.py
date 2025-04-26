from dataclasses import dataclass
from typing import List, Optional, Dict

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
class StringToken(Token):
    val: str
    
@dataclass
class CharToken(Token):
    val: str

# @dataclass
# class FunCallToken(Token):
#     funName: str


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
class AssignArr(AST):
    ArrAccessNode: AST
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
class Arr(AST):
    arr: List[AST]
    size: int

@dataclass
class ArrAccess(AST):
    arr: AST
    index: AST

@dataclass
class ArrDecl(AST):
    arr: AST

@dataclass
class FunObj:
    params: List[AST]
    body: AST
    env: Environment
    entry: Optional[int] = None 
    
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
class WhileStmt(AST):
    cond: AST
    body: AST

@dataclass
class Program(AST):
    decls: List[AST]

class ParseErr(Exception):
    pass

@dataclass
class Character(AST):
    val: str