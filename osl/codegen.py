from osl_parser import *

PUSH_CHAR   = 0x01
PUSH_SHORT  = 0x02
PUSH_INT    = 0x03
PUSH_LONG   = 0x04
PUSH_FLOAT  = 0x05
PUSH_DOUBLE = 0x06
PUSH_NONE   = 0x07
PUSH_BOOL   = 0x08

POP         = 0x10
DUP         = 0x11
SWAP        = 0x12
OVER        = 0x13

ADD         = 0x20
SUB         = 0x21
MUL         = 0x22
DIV         = 0x23
MOD         = 0x24
NEG         = 0x25

BITWISE_NOT = 0x30
BITWISE_AND = 0x31
BITWISE_OR  = 0x32
BITWISE_XOR = 0x33

EQ          = 0x40
NEQ         = 0x41
LT          = 0x42
GT          = 0x43
LE          = 0x44
GE          = 0x45

JUMP            = 0x50
JUMP_IF_ZERO    = 0x51
JUMP_IF_NONZERO = 0x52
CALL            = 0x53
RETURN          = 0x54
HALT            = 0x55

I2F         = 0x60
F2I         = 0x61
I2D         = 0x62
D2I         = 0x63
F2D         = 0x64
D2F         = 0x65

NEW_OBJECT  = 0x70
GET_FIELD   = 0x71
SET_FIELD   = 0x72

STORE = 0x80
LOAD  = 0x81

LOG = 0x90
NEWF = 0x91
MAKEF = 0x92

full_code = bytearray()


def do_codegen(tree: AST, code: bytearray = None): # returns bytearray
        
    def e_(tree: AST):
        return do_codegen(tree)
    
    if code is None:
        code = bytearray()

    match tree:
        case Program(decls):
            for decl in decls:
                code.extend(e_(decl))
            return code
        
        case Number(val):
            code.append(PUSH_INT)
            code.extend(int(val).to_bytes(4, 'little'))
            return code
            
        case StringLiteral(val):
            return val
        
        case Variable(varName, i):
            code.append(LOAD)
            code.extend(int(i).to_bytes(4, 'little'))
            return code
        
        case Let(Variable(varName, i), e1):
            if e1:
                code.extend(e_(e1))
            else:
                code.append(PUSH_NONE)
            code.append(STORE)
            code.extend(int(i).to_bytes(4, 'little'))
            return code
        
        case Assign(Variable(varName, i), e1):
            code.extend(e_(e1))
            code.append(STORE)
            code.extend(int(i).to_bytes(4, 'little'))
            return code
        
        case LetFun(Variable(varName, i), params, body):
            code.append(PUSH_INT)
            code.extend(int(i).to_bytes(4, 'little'))
            code.append(MAKEF)

            new_code = bytearray()
            # add arguments to stack
            for param in params:
                new_code.append(PUSH_INT)
                new_code.extend(int(param.id).to_bytes(4, 'little'))
            # add number of arguments
            new_code.append(PUSH_INT)
            new_code.extend(int(len(params)).to_bytes(4, 'little'))
            # add function id
            new_code.append(PUSH_INT)
            new_code.extend(int(i).to_bytes(4, 'little'))
            new_code.append(NEWF)

            fbody = do_codegen(body)

            new_code.append(JUMP)
            new_code.extend(len(fbody).to_bytes(2, 'little'))
            global full_code
            new_code.extend(fbody)
            full_code.extend(new_code)
            return code
        
        case CallFun(Variable(varName, i), args):
            for arg in args:
                code.extend(e_(arg))
            
            code.append(PUSH_INT)
            code.extend(int(len(args)).to_bytes(4, 'little'))
            code.append(PUSH_INT)
            code.extend(int(i).to_bytes(4, 'little'))
            code.append(CALL)
            return code
        
        case Statements(stmts):
            for stmt in stmts:
                code.extend(e_(stmt))
            return code
        
        case PrintStmt(expr):
            code.extend(e_(expr))
            code.append(LOG)
            return code
        
        case ReturnStmt(expr):
            if expr:
                code.extend(e_(expr))
            else:
                code.append(PUSH_NONE)
            code.append(RETURN)
            return code
            
        case BinOp("+", left, right):
            code.extend(e_(left))
            code.extend(e_(right))
            code.append(ADD)
            return code
        case BinOp("*", left, right):
            code.extend(e_(left))
            code.extend(e_(right))
            code.append(MUL)
            return code
        case BinOp("/", left, right): 
            code.extend(e_(left))
            code.extend(e_(right))
            code.append(DIV)
            return code
        case BinOp("-", left, right):
            code.extend(e_(left))
            code.extend(e_(right))
            code.append(SUB)
            return code
        case BinOp("<", left, right):
            code.extend(e_(left))
            code.extend(e_(right))
            code.append(LT)
            return code
        case BinOp(">", left, right):
            code.extend(e_(left))
            code.extend(e_(right))
            code.append(GT)
            return code
        case BinOp("<=", left, right):
            code.extend(e_(left))
            code.extend(e_(right))
            code.append(LE)
            return code
        case BinOp(">=", left, right):
            code.extend(e_(left))
            code.extend(e_(right))
            code.append(GE)
            return code
        case BinOp("=", left, right):
            code.extend(e_(left))
            code.extend(e_(right))
            code.append(EQ)
            return code
        case BinOp("!=", left, right):
            code.extend(e_(left))
            code.extend(e_(right))
            code.append(NEQ)
            return code
        case BinOp("%", left, right):
            code.extend(e_(left))
            code.extend(e_(right))
            code.append(MOD)
            return code
        case BinOp("||", left, right):
            code.extend(e_(left))
            code.extend(e_(right))
            code.append(BITWISE_OR)
            return code
        case BinOp("&&", left, right):
            code.extend(e_(left))
            code.extend(e_(right))
            code.append(BITWISE_AND)
            return code
        case UnOp("-", right): return -e_(right)
        case UnOp("\u221a", right): return e_(right) ** 0.5
        
        case If(condition, then_body, else_body): 
            code.extend(e_(condition))
            code.append(JUMP_IF_ZERO)
            code.extend(int(0).to_bytes(2, 'little'))
            jif_pos = len(code)-2
            code.extend(e_(then_body))
            code.append(JUMP)
            code.extend(int(0).to_bytes(2, 'little'))
            j_pos = len(code)-2
            code[jif_pos:jif_pos+2] = int(len(code)-jif_pos-2).to_bytes(2, 'little')
            code.extend(e_(else_body))
            code[j_pos:j_pos+2] = int(len(code)-j_pos-2).to_bytes(2, 'little')
            return code
            
        case IfUnM(condition, then_body):
            code.extend(e_(condition))
            code.append(JUMP_IF_ZERO)
            code.extend(int(0).to_bytes(2, 'little'))
            jif_pos = len(code)-2
            code.extend(e_(then_body))
            code[jif_pos:jif_pos+2] = int(len(code)-jif_pos-2).to_bytes(2, 'little')
            return code

def codegen(t):
    global full_code
    code = do_codegen(t)
    full_code.extend(code)
    full_code.append(HALT)
    return full_code