from src.osl_lexer import *
from more_itertools import peekable

cnt = 0
def fresh():
    global cnt
    cnt = cnt + 1
    return cnt

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
            case KeyWordToken("def"):
                return parse_func()
            case KeyWordToken("var"):
                return parse_let()
            case _:
                return parse_statement()
            
    def parse_func():
        consume(KeyWordToken, "def")
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
        # var = Variable(consume(VariableToken).varName)
        lhs = parse_secondary()
        if isinstance(lhs, Variable):
            e1 = None
            if peek() == OperatorToken(":="):
                consume(OperatorToken, ":=")
                e1 = parse_expression()
            consume(OperatorToken, ";")
            return Let(lhs, e1)
        elif isinstance(lhs, ArrAccess):
            if peek() == OperatorToken(":="):
                raise ParseErr("To initialize an array, don't specify the size in the declaration")
            consume(OperatorToken, ";")
            return ArrDecl(lhs)
    
    def parse_statement():
        match peek():
            case KeyWordToken("if"):
                return parse_if()
            case KeyWordToken("log"):
                consume(KeyWordToken, "log")
                # consume(OperatorToken, "(")
                expr = parse_expression()
                # consume(OperatorToken, ")")
                consume(OperatorToken, ";")
                return PrintStmt(expr)

            case KeyWordToken("while"):
                consume(KeyWordToken, "while")
                expr = parse_expression()
                body = parse_block()
                return WhileStmt(expr, body)
            
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
        ast = parse_expB()
        if not (isinstance(ast, Variable) or isinstance(ast, ArrAccess)) and peek() == OperatorToken(":="):
            raise ParseErr(f"Expected variable or array on the left side of assignment := operator at index {i}")
        if isinstance(ast, Variable) and peek() == OperatorToken(":="):
            consume(OperatorToken, ":=")
            e1 = parse_expB()
            return Assign(ast, e1)
        if isinstance(ast, ArrAccess) and peek() == OperatorToken(":="):
            consume(OperatorToken, ":=")
            e1 = parse_expB()
            return AssignArr(ast, e1)
        return ast

    def parse_expB():
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
            case OperatorToken(op) if op in {"<", ">", "<=", ">=", "=", "~="}:
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
        ast = parse_unary()
        while True:
            match peek():
                # case OperatorToken('^'):
                #     consume()
                #     ast = BinOp("^", ast, parse_exponentiation())
                case _:
                    return ast

    def parse_unary():
        match peek():
            case OperatorToken('-'):
                consume()
                return UnOp("-", parse_unary())
            case OperatorToken('~'):
                consume()
                return UnOp("~", parse_unary())
            case OperatorToken("["):
                return parse_arr_decl()
            case _:
                return parse_secondary()

    def parse_arr_decl():
        consume(OperatorToken, "[")
        arrr = []
        ctr = 0
        if peek() != OperatorToken("]"):
            while True:
                arrr.append(parse_expB()) # parse_expression() will allow assignments also, but we don't allow them for now
                ctr += 1
                if peek() == OperatorToken(","):
                    consume(OperatorToken, ",")
                else:
                    break
        consume(OperatorToken, "]")
        return Arr(arrr, ctr) # lhs1 is now a CallFun() or a Variable

    def parse_secondary():
        lhs = parse_primary()
        if isinstance(lhs, Variable):
            match peek():
                case OperatorToken('('):
                    return parse_calls(lhs)
                case OperatorToken('['):
                    return parse_arr_accesses(lhs)
        return lhs

    def parse_calls(lhs1):
        lhs = parse_cl(lhs1)
        if peek() == OperatorToken('('):
            return parse_calls(lhs)
        return lhs

           
    def parse_cl(lhs1):
        consume(OperatorToken, "(")
        args = []
        if peek() != OperatorToken(")"):
            while True:
                args.append(parse_expB()) # parse_expression() will allow assignments also, but we don't allow them for now
                if peek() == OperatorToken(","):
                    consume(OperatorToken, ",")
                else:
                    break
        consume(OperatorToken, ")")
        return CallFun(lhs1, args) # lhs1 is now a CallFun() or a Variable
    
    def parse_arr_accesses(lhs1):
        lhs = parse_ac(lhs1)
        if peek() == OperatorToken('['):
            return parse_arr_accesses(lhs)
        return lhs
        
    def parse_ac(lhs1):
        consume(OperatorToken, "[")
        index = parse_expB()
        consume(OperatorToken, "]")
        return ArrAccess(lhs1, index) # lhs1 is now a CallFun() or a Variable
    
    def parse_primary():
        match peek():
            case NumberToken(v):
                consume()
                val = float(v) if '.' in v else int(v)
                return Number(val)
            
            case CharToken(c):
                consume()
                return Character(c)
            
            case StringToken(v):
                consume()
                return Arr([Character(c) for c in v], len(v))
            
            case VariableToken(varName):
                consume()
                return Variable(varName)
            
            case OperatorToken("("):
                consume()
                ast = parse_expB()
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
        
        case Character(_) as S:
            return S
        
        case Let(Variable(varName, _), e1):
            re1 = resolve_(e1) if e1 else None
            env.add(varName, i := fresh())
            return Let(Variable(varName, i), re1)
        
        case ArrDecl(ArrAccess(arr) as arrD) as arrDecl:
            while isinstance(arrD, ArrAccess) and isinstance(arrD.arr, ArrAccess):
                arrD.index = resolve_(arrD.index)
                arrD = arrD.arr
            # print(arrD)
            arrD.index = resolve_(arrD.index)
            env.add(arrD.arr.varName, i := fresh())
            arrD.arr = Variable(arrD.arr.varName, env.get(arrD.arr.varName))
            return arrDecl
        
        case Assign(Variable(varName, _), e1):
            re1 = resolve_(e1)
            return Assign(Variable(varName, env.get(varName)), re1)
        
        case AssignArr(ArrAccess(arr, index), e1):
            rarr = resolve_(arr)
            index = resolve_(index)
            re1 = resolve_(e1)
            return AssignArr(ArrAccess(rarr, index), re1)
    
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
        
        case ArrAccess(arr, index):
            arr = resolve_(arr)
            index = resolve_(index)
            return ArrAccess(arr, index)
        
        case Arr(arr, size):
            new_arr = [resolve_(elem) for elem in arr]
            return Arr(new_arr, size)
        
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
        
        case WhileStmt(condition, body):
            condition = resolve_(condition)
            body = resolve_(body)
            return WhileStmt(condition, body)