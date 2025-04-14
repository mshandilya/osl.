from dataclasses import dataclass
from typing import List, Dict, Optional
import struct

@dataclass
class Value:
    pass

@dataclass
class Integer(Value):
    val: int

class Environment:
    envs: List[Dict[int, Value]]
    
    def __init__(self):
        self.envs = [{}]
    
    def enter_scope(self):
        self.envs.append({})
    
    def exit_scope(self):
        assert self.envs, "Cannot exit global scope"
        self.envs.pop()
    
    def add(self, var: int, val: Value):
        # assert var not in self.envs[-1], f"Variable {var} already defined"
        self.envs[-1][var] = val
    
    def get(self, var: int) -> Value:
        for env in reversed(self.envs):
            if var in env:
                return env[var]
        # raise ValueError(f"Variable {var} not defined")
        return None
    
    def update(self, var: int, val: Value):
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
class FunObj(Value):
    entry: int
    args: Optional[List[int]]
    env: Environment

@dataclass
class CallFrame:
    env: Environment
    ret: int
    
@dataclass
class Code:
    bytecode: bytearray
    # env: Environment   

class Opcode:
    PUSH_INT    = 0x03
    PUSH_NONE   = 0x07
    POP         = 0x10
    DUP         = 0x11
    ADD         = 0x20
    SUB         = 0x21
    MUL         = 0x22
    DIV         = 0x23
    MOD         = 0x24
    NEG         = 0x25
    EQ          = 0x40
    LT          = 0x42
    GT          = 0x43
    JUMP        = 0x50
    JUMP_IF_ZERO    = 0x51
    JUMP_IF_NONZERO = 0x52
    CALL        = 0x53
    RETURN      = 0x54
    HALT        = 0x55
    STORE       = 0x80
    LOAD        = 0x81
    ENTER_SCOPE = 0x82
    EXIT_SCOPE  = 0x83
    LOG         = 0x90
    NEWF        = 0x91
    MAKEF       = 0x92
    
class StackVM:
    def __init__(self, code: Code):
        self.code = code
        self.stack: List[Value] = []
        self.pc = 0
        self.call_stack: List[CallFrame] = []
        self.STACK_SIZE = 10000
        c = CallFrame(
            env=Environment(),
            ret=None)
        self.call_stack.append(c)

    def push(self, value: Value):
        if len(self.stack) >= self.STACK_SIZE:
            raise RuntimeError("Stack overflow")
        self.stack.append(value)

    def pop(self) -> Value:
        if not self.stack:
            raise RuntimeError("Stack underflow")
        return self.stack.pop()
    
    def current_env(self) -> Environment:
        """Get the environment of the current call frame."""
        if not self.call_stack:
            raise RuntimeError("No active call frame")
        return self.call_stack[-1].env
    
    def execute(self):
        while self.pc < len(self.code.bytecode):
            op = self.code.bytecode[self.pc]
            #print(f"Opcode: {hex(op)}")
            if op == Opcode.HALT:
                break
            
            elif op == Opcode.PUSH_INT:
                if self.pc + 4 > len(self.code.bytecode):
                    raise RuntimeError("Invalid PUSH_INT instruction")
                val = struct.unpack('<i', self.code.bytecode[self.pc + 1:self.pc + 5])[0] # < denotes little-endian, i denotes int
                self.push(Integer(val))
                self.pc += 5
                
            elif op == Opcode.POP:
                self.pop()
                self.pc += 1
                
            elif op == Opcode.DUP:
                if not self.stack:
                    raise RuntimeError("Stack underflow")
                self.push(self.stack[-1])
                self.pc += 1
                
            elif op == Opcode.ADD:
                right = self.pop()
                left = self.pop()
                if isinstance(left, Integer) and isinstance(right, Integer):
                    self.push(Integer(left.val + right.val))
                else:
                    raise TypeError("Invalid types for ADD")
                self.pc += 1
                
            elif op == Opcode.SUB:
                right = self.pop()
                left = self.pop()
                if isinstance(left, Integer) and isinstance(right, Integer):
                    self.push(Integer(left.val - right.val))
                else:
                    raise TypeError("Invalid types for SUB")
                self.pc += 1
            
            elif op == Opcode.MUL:
                right = self.pop()
                left = self.pop()
                if isinstance(left, Integer) and isinstance(right, Integer):
                    self.push(Integer(left.val * right.val))
                else:
                    raise TypeError("Invalid types for MUL")
                self.pc += 1
            
            elif op == Opcode.DIV:
                right = self.pop()
                left = self.pop()
                if isinstance(left, Integer) and isinstance(right, Integer):
                    if right.val == 0:
                        raise ZeroDivisionError("Division by zero")
                    self.push(Integer(left.val // right.val))
                else:
                    raise TypeError("Invalid types for DIV")
                self.pc += 1
            
            elif op == Opcode.MOD:
                right = self.pop()
                left = self.pop()
                if isinstance(left, Integer) and isinstance(right, Integer):
                    if right.val == 0:
                        raise ZeroDivisionError("Division by zero")
                    self.push(Integer(left.val % right.val))
                else:
                    raise TypeError("Invalid types for MOD")
                self.pc += 1
                
            elif op == Opcode.NEG:
                right = self.pop()
                if isinstance(right, Integer):
                    self.push(Integer(-right.val))
                else:
                    raise TypeError("Invalid type for NEG")
                self.pc += 1
                
            elif op == Opcode.EQ:
                b = self.pop()
                a = self.pop()
                if isinstance(a, Integer) and isinstance(b, Integer):
                    self.push(Integer(int(a.val == b.val)))
                else:
                    raise TypeError("Invalid types for EQ")
                self.pc += 1
            
            elif op == Opcode.LT:
                b = self.pop()
                a = self.pop()
                if isinstance(a, Integer) and isinstance(b, Integer):
                    self.push(Integer(int(a.val < b.val)))
                else:
                    raise TypeError("Invalid types for LT")

                self.pc += 1

            elif op == Opcode.GT:
                b = self.pop()
                a = self.pop()
                if isinstance(a, Integer) and isinstance(b, Integer):
                    self.push(Integer(int(a.val > b.val)))
                else:
                    raise TypeError("Invalid types for GT")

                self.pc += 1
            
            elif op == Opcode.JUMP:
                if self.pc + 2 > len(self.code.bytecode):
                    raise RuntimeError("Invalid JUMP instruction")
                offset = struct.unpack('<h', self.code.bytecode[self.pc + 1:self.pc + 3])[0]
                self.pc += 3 + offset
            
            elif op == Opcode.JUMP_IF_ZERO:
                if self.pc + 2 > len(self.code.bytecode):
                    raise RuntimeError("Invalid JUMP_IF_ZERO instruction")
                offset = struct.unpack('<h', self.code.bytecode[self.pc + 1:self.pc + 3])[0]
                cond = self.pop()
                if not isinstance(cond, Integer):
                    raise TypeError("Invalid type for JUMP_IF_ZERO")
                
                self.pc += 3 + (offset if cond.val == 0 else 0)
                
            elif op == Opcode.JUMP_IF_NONZERO:
                if self.pc + 2 > len(self.code.bytecode):
                    raise RuntimeError("Invalid JUMP_IF_NONZERO instruction")
                offset = struct.unpack('<h', self.code.bytecode[self.pc + 1:self.pc + 3])[0]
                cond = self.pop()
                if not isinstance(cond, Integer):
                    raise TypeError("Invalid type for JUMP_IF_NONZERO")
                
                self.pc += 3 + (offset if cond.val != 0 else 0)
                
            elif op == Opcode.STORE:
                if self.pc + 4 > len(self.code.bytecode):
                    raise RuntimeError("Invalid STORE instruction")
                
                id = struct.unpack('<i', self.code.bytecode[self.pc + 1:self.pc + 5])[0]
                val = self.pop()
                try:
                    self.current_env().update(id, val)
                except ValueError:
                    self.current_env().add(id, val)
                self.pc += 5
                
            elif op == Opcode.LOAD:
                if self.pc + 4 > len(self.code.bytecode):
                    raise RuntimeError("Invalid LOAD instruction")
                
                id = struct.unpack('<i', self.code.bytecode[self.pc + 1:self.pc + 5])[0]
                val = self.current_env().get(id)
                self.push(val)
                self.pc += 5

            
            elif op == Opcode.CALL:
                """
                The stack is as follows:
                Function address
                Number of arguments
                Argument 1's val
                ...
                (All are 4 bytes)
                """
                if self.pc + 1 > len(self.code.bytecode):
                    raise RuntimeError("Invalid CALL instruction")
                
                fun_id = self.pop().val
                funObject = self.current_env().get(fun_id)
                call_env = funObject.env.copy()
                call_env.enter_scope()
                num_args = self.pop().val

                for it in range(num_args):
                    val = self.pop()
                    call_env.add(funObject.args[it], val)

                self.pc += 1
                
                c = CallFrame(
                    env = call_env,
                    ret = self.pc
                )
                self.call_stack.append(c)
                self.pc = funObject.entry
                
            elif op == Opcode.RETURN:
                if not self.call_stack:
                    raise RuntimeError("RETURN outside function")
                frame = self.call_stack.pop()
                return_value = self.pop() if self.stack else None
                if return_value is not None:
                    self.push(return_value)
                self.pc = frame.ret if frame.ret is not None else len(self.code.bytecode)

            elif op == Opcode.LOG:
                if not self.stack:
                    raise RuntimeError("No elements to print (empty stack)")
                val = self.pop()
                print(val.val)
                self.pc += 1

            elif op == Opcode.PUSH_NONE:
                self.push(None)
                self.pc += 1

            elif op == Opcode.NEWF:
                fun_id = self.pop().val
                num_args = self.pop().val
                args_ids = []
                for _ in range(num_args):
                    args_ids.append(self.pop().val)

                newFunObj = FunObj(self.pc+4, args_ids, None)
                self.current_env().add(fun_id, newFunObj)
                self.pc += 1

            elif op == Opcode.MAKEF:
                fun_id = self.pop().val
                funObject = self.current_env().get(fun_id)
                
                self.current_env().add(fun_id, funObject)
                funObject.env = self.current_env().copy()
                self.pc += 1
                
            else:
                raise RuntimeError(f"Unknown opcode: {hex(op)} at PC {self.pc}")    
                
        return self.stack[-1].val if self.stack else None       
              
# Example 1 (Addition: 5 + 3) 
                
# fnobj =  FunObj(
#     entry=3,
#     args=[2, 3]
# )

# env = Environment()
# env.add(1, fnobj)

# From here

# code = Code(
#     bytecode=bytearray([
#         Opcode.JUMP, 0x0C, 0x00,
#         Opcode.LOAD, 0x02, 0x00,0x00,0x00,
#         Opcode.LOAD, 0x03, 0x00,0x00,0x00,
#         Opcode.ADD,
#         Opcode.RETURN,
#         Opcode.PUSH_INT, 0x02,0x00,0x00,0x00, # argument 1's id = 2
#         Opcode.PUSH_INT, 0x08,0x00,0x00,0x00, # argument 1's value = 8
#         Opcode.PUSH_INT, 0x03,0x00,0x00,0x00, # argument 2's id = 3
#         Opcode.PUSH_INT, 0x05,0x00,0x00,0x00, # argument 2's value = 5
#         Opcode.PUSH_INT, 0x02,0x00,0x00,0x00, # Number of arguments = 2
#         Opcode.PUSH_INT, 0x03,0x00,0x00,0x00, # Function's entry = 3
#         Opcode.CALL,
#         Opcode.HALT
#         ]),
#     env=Environment()
# )

# stack = StackVM(code)
# result = stack.execute()
# print(result)  # Should print 5 + 3 = 8





# Example 2 (factorial of 5)

# fnobj =  FunObj(
#     entry=3,
#     args=[2],
# )

# env = Environment()
# env.add(1, fnobj)

# code = Code(
#     bytecode=bytearray([
#         Opcode.JUMP, 0x36, 0x00, # This is a jump of 16*3 = 48 bytes
#         Opcode.LOAD, 0x02, 0x00, 0x00, 0x00,
#         Opcode.PUSH_INT, 0x01, 0x00, 0x00, 0x00,
#         Opcode.EQ,
#         Opcode.JUMP_IF_ZERO, 0x06, 0x00,
#         Opcode.LOAD, 0x02, 0x00,0x00,0x00,
#         Opcode.RETURN,
#         Opcode.LOAD, 0x02, 0x00,0x00,0x00,
#         Opcode.PUSH_INT, 0x02, 0x00, 0x00, 0x00, # id = 2
#         Opcode.LOAD, 0x02, 0x00,0x00,0x00,
#         Opcode.PUSH_INT, 0x01, 0x00, 0x00, 0x00,
#         Opcode.SUB,
#         Opcode.PUSH_INT, 0x01, 0x00, 0x00, 0x00, # Number of args
#         Opcode.PUSH_INT, 0x03, 0x00, 0x00, 0x00, # Function's entry
#         Opcode.CALL,
#         Opcode.MUL,
#         Opcode.RETURN,
#         Opcode.PUSH_INT, 0x02, 0x00, 0x00, 0x00, # id = 2
#         Opcode.PUSH_INT, 0x05, 0x00, 0x00, 0x00, # val = 5
#         Opcode.PUSH_INT, 0x01, 0x00, 0x00, 0x00, # Number of args
#         Opcode.PUSH_INT, 0x03, 0x00, 0x00, 0x00, # Function's entry
#         Opcode.CALL,
#         Opcode.HALT
#         ]),
#     env=Environment()
# )

# stack = StackVM(code)
# result = stack.execute()
# print(result)  # Should print 5 + 3 = 8

# Uncomment from here

with open("bytecode.bin", "rb") as f:
    inp = bytearray(f.read())

code = Code(
    bytecode=inp,
    # env=Environment()
)

stack = StackVM(code)
result = stack.execute()
