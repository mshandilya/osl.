from src.osl_parser import *
import struct

PUSH_CHAR   = 0x01
PUSH_SHORT  = 0x02
PUSH_INT    = 0x03
PUSH_LONG   = 0x04
PUSH_FLOAT  = 0x05
PUSH_DOUBLE = 0x06
PUSH_NONE   = 0x07
PUSH_BOOL   = 0x08
# PUSH_ID     = 0x09

POP         = 0x10
DUP         = 0x11
SWAP        = 0x12
OVER        = 0x13
GET         = 0x15
SET         = 0x16

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

AND         = 0x34
OR          = 0x35
NOT         = 0x36

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

LOG = 0x90
NEWF = 0x91
MAKEF = 0x92

MAKE_ARRAY = 0x93
ARRACC = 0x94

STORE = 0x95
LOAD  = 0x96

MAKE_ARRAY_DECL = 0x97



fnEntryDict = {}
full_code = bytearray()
def do_codegen(tree: AST):  # returns bytearray
    global full_code
    global fnEntryDict

    def e_(tree: AST):
        return do_codegen(tree)

    match tree:
        case Program(decls):
            for decl in decls:
                e_(decl)
            return
        
        case Number(val):
            if isinstance(val, int):
                full_code.append(PUSH_INT)
                full_code.extend(int(val).to_bytes(8, 'little'))
            elif isinstance(val, float):
                full_code.append(PUSH_FLOAT)
                full_code.extend(struct.pack('<f', val))
            return
        
        case Character(val):
            full_code.append(PUSH_CHAR)
            full_code.extend(val.encode('utf-8'))
            return
        
        case Variable(varName, i):
            full_code.append(GET)
            full_code.extend(int(i).to_bytes(8, 'little'))
            return
        
        case Let(Variable(varName, i), e1):
            if e1:
                e_(e1)
            else:
                full_code.append(PUSH_NONE)
            full_code.append(SET)
            full_code.extend(int(i).to_bytes(8, 'little'))
            return
        
        case Assign(Variable(varName, i), e1):
            e_(e1)
            full_code.append(SET)
            full_code.extend(int(i).to_bytes(8, 'little'))
            return
        
        case LetFun(Variable(varName, i), params, body):
            full_code.append(JUMP)
            full_code.extend(int(0).to_bytes(4, 'little'))
            entry_point = len(full_code)
            fnEntryDict[i] = entry_point
            for param in params:
                full_code.append(SET)
                full_code.extend(int(param.id).to_bytes(8, 'little'))
            e_(body)
            body_pos = len(full_code)
            full_code[entry_point - 4: entry_point] = int(body_pos - entry_point).to_bytes(4, 'little')
            return

        case Arr(arr, size):
            for a in arr[::-1]:
                e_(a)
            full_code.append(MAKE_ARRAY)
            full_code.extend(int(size).to_bytes(2, 'little'))
            return
            
        case ArrAccess(arr, index):
            e_(index)
            e_(arr)
            full_code.append(ARRACC)
            return
        
        case AssignArr(arr, e1):
            e_(e1)
            e_(arr)
            full_code.append(STORE)
            return
        
        case ArrDecl(arr):
            indices = []
            while isinstance(arr, ArrAccess):
                indices.append(arr.index)
                arr = arr.arr
            indices = indices[::-1]
            for i in indices:
                e_(i)
            full_code.append(MAKE_ARRAY_DECL)
            full_code.extend(int(len(indices)).to_bytes(2, 'little'))
            full_code.append(SET)
            full_code.extend(int(arr.id).to_bytes(8, 'little'))
            return
        
        case CallFun(Variable(varName, i), args):
            full_code.append(PUSH_INT)
            full_code.extend(int(0).to_bytes(8, 'little'))  # Return address
            i_pos = len(full_code)
            for arg in args[::-1]:
                e_(arg)
            full_code.append(CALL)
            entry_point = fnEntryDict[i]
            full_code.extend(int(entry_point).to_bytes(4, 'little'))
            full_code[i_pos - 8 : i_pos] = int(len(full_code)).to_bytes(8, 'little')
            return
        
        case Statements(stmts):
            for stmt in stmts:
                e_(stmt)
            return
        
        case PrintStmt(expr):
            e_(expr)
            full_code.append(LOG)
            return
        
        case ReturnStmt(expr):
            if expr:
                e_(expr)
            else:
                full_code.append(PUSH_NONE)
            full_code.append(RETURN)
            return
            
        case BinOp(op, left, right):
            e_(left)
            e_(right)
            op_map = {
                "+": ADD, "-": SUB, "*": MUL, "/": DIV, "%": MOD,
                "<": LT, ">": GT, "<=": LE, ">=": GE, "=": EQ, "~=": NEQ,
                "||": OR, "&&": AND, "~": NOT
            }
            full_code.append(op_map[op])
            return
        
        case UnOp("-", right):
            e_(right)
            full_code.append(NEG)
            return
        
        case If(condition, then_body, else_body): 
            e_(condition)
            full_code.append(JUMP_IF_ZERO)
            full_code.extend(int(0).to_bytes(4, 'little'))
            jif_pos = len(full_code) - 4
            e_(then_body)
            full_code.append(JUMP)
            full_code.extend(int(0).to_bytes(4, 'little'))
            j_pos = len(full_code) - 4
            full_code[jif_pos:jif_pos+4] = int(len(full_code) - jif_pos - 4).to_bytes(4, 'little')
            e_(else_body)
            full_code[j_pos:j_pos+4] = int(len(full_code) - j_pos - 4).to_bytes(4, 'little')
            return
        
        case WhileStmt(condition, body):
            cond_start = len(full_code)
            e_(condition)
            full_code.append(JUMP_IF_ZERO)
            full_code.extend(int(0).to_bytes(4, 'little'))
            jif_pos = len(full_code) - 4
            e_(body)
            full_code.append(JUMP)
            full_code.extend(int(cond_start - len(full_code) - 4).to_bytes(4, 'little', signed=True))
            full_code[jif_pos:jif_pos+4] = int(len(full_code) - jif_pos - 4).to_bytes(4, 'little')
            return
            
        case IfUnM(condition, then_body):
            e_(condition)
            full_code.append(JUMP_IF_ZERO)
            full_code.extend(int(0).to_bytes(4, 'little'))
            jif_pos = len(full_code) - 4
            e_(then_body)
            full_code[jif_pos:jif_pos+4] = int(len(full_code) - jif_pos - 4).to_bytes(4, 'little')
            return

def codegen(t):
    global full_code
    global fnEntryDict
    do_codegen(t)
    full_code.append(HALT)
    return full_code
