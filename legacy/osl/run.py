import argparse
from src.osl_eval import *
from pprint import pprint
import sys
# from src.codegenLegacy import *
from src.codegen import *
import time
sys.setrecursionlimit(100000000)
from src.visualizer import *

def main():
    parser = argparse.ArgumentParser(description="osl Compiler/Interpreter")
    parser.add_argument(
        "--compile", "-c", 
        action="store_true", 
        help="Compile the code to bytecode"
    )
    parser.add_argument(
        "--run", "-i", 
        action="store_true", 
        help="Interpret and execute the code"
    )
    parser.add_argument(
        "--time", "-t", 
        action="store_true", 
        help="Display time taken for compilation or evaluation"
    )
    args = parser.parse_args()

    with open("code.osl") as f:
        code = f.read()

    if args.compile:
        start_time = time.time()
        parsed = parse(code)
        rcode = resolve(parsed)
        bb = bytearray(codegen(rcode))
        end_time = time.time()
        with open("bytecode.bin", "wb") as bytecode_file:
            bytecode_file.write(bb)
        result = parse_bytecode(bb)
        for opcode, operand in result:
           print(f"{opcode} {operand if operand is not None else ''}")
        print("Bytecode generated and saved to bytecode.bin")
        if args.time:
            print(f"Compilation time: {end_time - start_time:.4f} seconds")
        
    elif args.run:
        parsed = parse(code)
        rcode = resolve(parsed)
        pprint(rcode)
        start_time = time.time()
        result = e(rcode)
        end_time = time.time()
        print(result)
        if args.time:
            print(f"Evaluation time: {end_time - start_time:.4f} seconds")
        
    else:
        print("Please specify either --compile (-c) or --run (-i).")

if __name__ == "__main__":
    main()