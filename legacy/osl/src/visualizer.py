import struct

def parse_bytecode(bytecode: bytearray):
    opcodes = {
        0x01: ("PUSH_CHAR", 1), 0x02: ("PUSH_SHORT", 2), 0x03: ("PUSH_INT", 8), 0x04: ("PUSH_LONG", 8),
        0x05: ("PUSH_FLOAT", 4), 0x06: ("PUSH_DOUBLE", 8), 0x07: ("PUSH_NONE", 0),
        0x10: ("POP", 0), 0x11: ("DUP", 0), 0x12: ("SWAP", 0), 0x13: ("OVER", 0),
        0x20: ("ADD", 0), 0x21: ("SUB", 0), 0x22: ("MUL", 0), 0x23: ("DIV", 0), 0x24: ("MOD", 0), 0x25: ("NEG", 0),
        0x30: ("BITWISE_NOT", 0), 0x31: ("BITWISE_AND", 0), 0x32: ("BITWISE_OR", 0), 0x33: ("BITWISE_XOR", 0),
        0x34: ("AND", 0), 0x35: ("OR", 0), 0x36: ("NOT", 0),
        0x40: ("EQ", 0), 0x41: ("NEQ", 0), 0x42: ("LT", 0), 0x43: ("GT", 0), 0x44: ("LE", 0), 0x45: ("GE", 0),
        0x50: ("JUMP", 4), 0x51: ("JUMP_IF_ZERO", 4), 0x52: ("JUMP_IF_NONZERO", 4), 0x53: ("CALL", 0),
        0x54: ("RETURN", 0), 0x55: ("HALT", 0),
        0x60: ("I2F", 0), 0x61: ("F2I", 0), 0x62: ("I2D", 0), 0x63: ("D2I", 0), 0x64: ("F2D", 0), 0x65: ("D2F", 0),
        0x70: ("NEW_OBJECT", 1), 0x71: ("GET_FIELD", 1), 0x72: ("SET_FIELD", 1),
        0x16: ("SET", 8), 0x15: ("GET", 8),
        0x90: ("LOG", 0), 0x91: ("MAKE_FUNC", 0), 0x92: ("MAKE_CLOSURE", 0), 0x93: ("MAKE_ARRAY", 2), 0x94: ("ARRACC", 0),
        0x95: ("STORE", 0), 0x96: ("LOAD", 0), 0x97: ("MAKE_ARRAY_DECL", 2)
    }
    
    index = 0
    output = []
    
    while index < len(bytecode):
        opcode = bytecode[index]
        index += 1
        
        if opcode in opcodes:
            name, operand_size = opcodes[opcode]
            operand = None
            
            if operand_size > 0:
                operand_bytes = bytecode[index:index + operand_size]
                index += operand_size
                
                if operand_size == 1:
                    operand = operand_bytes[0]
                elif operand_size == 2:
                    operand = struct.unpack('<h', operand_bytes)[0]  # Little-endian short
                elif operand_size == 4:
                    if opcode in [0x05]:  # PUSH_FLOAT
                        operand = struct.unpack('<f', operand_bytes)[0]
                    else:
                        operand = struct.unpack('<i', operand_bytes)[0]
                elif operand_size == 8:
                    if opcode in [0x06]:  # PUSH_DOUBLE
                        operand = struct.unpack('<d', operand_bytes)[0]
                    else:
                        operand = struct.unpack('<q', operand_bytes)[0]
                
            output.append((name, operand))
        else:
            output.append((f"UNKNOWN_OPCODE_{opcode:02X}", None))
    
    return output

# Example usage
#bytecode = bytearray([0x01, 0x41, 0x03, 0x78, 0x56, 0x34, 0x12, 0x20, 0x10, 0x55])
# result = parse_bytecode(bytecode)
# for opcode, operand in result:
#    print(f"{opcode} {operand if operand is not None else ''}")
