from calculator_extended_resolved import parse, e, resolve
from pprint import pprint
from colorama import Fore, Style
import sys
from io import StringIO

def unit_test(expr: str, expected_value, results, expect_print=False):
    print(f"Expression: {expr}")
    try:
        ast = resolve(parse(expr))
        pprint(ast)
        
        # Capture print output if expected
        if expect_print:
            old_stdout = sys.stdout
            new_stdout = StringIO()
            sys.stdout = new_stdout
            result = e(ast)
            sys.stdout = old_stdout
            captured_output = new_stdout.getvalue().strip()
            captured_output += "\n" + str(result)
            print(f"Captured Output: {captured_output}")
            if captured_output != expected_value:
                error_msg = f"Test failed for expression: {expr}. Expected output:\n'{expected_value}'\n but got:\n'{captured_output}'."
                results.append(("FAILED", expr, error_msg))
                print(error_msg)
                print()
            else:
                results.append(("PASSED", expr, None))
                print("Test passed!\n")
        else:
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
        results.append(("ERROR", expr, f"{Fore.RED}{error_msg}{Style.RESET_ALL}"))
        print()

log = []

# Basic Arithmetic
unit_test("2;", 2, log)
unit_test("2 + 3;", 5, log)
unit_test("2 + 3 * 5;", 17, log)
unit_test("(2 + 3) * 5;", 25, log)
unit_test("2 + 3 * 5 ^ 2;", 77, log)
unit_test("2.5 + 3.5;", 6.0, log)
unit_test("2 ^ 3 ^ 2;", 512, log)
unit_test("-5 * -5;", 25, log)
unit_test("10 % 3;", 1, log)
unit_test("\u221a(16);", 4, log)

# Variables and Assignment
unit_test("var x := 5; x;", 5, log)
unit_test("var x := 5; x := x + 1; x;", 6, log)
unit_test("var x := 10; var y := x * 2; y;", 20, log)

# Functions
unit_test("letFunc f(x) { return x + 1; } f(5);", 6, log)
unit_test("letFunc f(x) { var y := x * 2; return y; } f(3);", 6, log)

# Multi-line osl code in separate variables
func_test1 = """
letFunc f(x) { 
    return x ^ 2; 
}
f(4);
"""
unit_test(func_test1, 16, log)

nested_func_test = """
var x := 5;
letFunc f(y) { 
    return x + y; 
}
f(3);
"""
unit_test(nested_func_test, 8, log)

closure_test = """
letFunc outer() {
    var x := 10;
    letFunc inner() { 
        return x; 
    }
    return inner;
}
var f := outer();
f();
"""
unit_test(closure_test, 10, log)

closure_test1 = """
letFunc outer(x) {
    letFunc inner() { 
        return x; 
    }
    return inner;
}
var f := outer(10);
f();
"""
unit_test(closure_test1, 10, log)

closure_test2 = """
var x := 6;

letFunc F(x)
{
    letFunc G()
    {
        return x;
    }
    return G;
}

var y := F(5);
y() * y();
"""
unit_test(closure_test2, 25, log)

lexical_scoping_test = """
var x := 5;
letFunc f(y) 
{
    return x;
} 
print(x);
print(f(2));
letFunc g(z) 
{ 
    var x := 6;
    return f(z);
}
g(0);
"""
unit_test(lexical_scoping_test, "5\n5\n5", log, expect_print=True)

# Control Flow
unit_test("if (5 > 3) 10; else 20;", 10, log)
unit_test("if (5 < 3) 10; else 20;", 20, log)
unit_test("var x := 15; if (x > 10) x + 1; else x - 1;", 16, log)

# Print Statements
print_test1 = """
var x := 42;
print(x);
"""
unit_test(print_test1, "42\nNone", log, expect_print=True)

print_test2 = """
letFunc f(x) {
    print(x * 2);
}
f(5);
"""
unit_test(print_test2, "10\nNone", log, expect_print=True)

# Recursion
factorial_test = """
letFunc fact(n) {
    if (n = 0) return 1;
    return n * fact(n - 1);
}
fact(5);
"""
unit_test(factorial_test, 120, log)

# Complex Expressions
unit_test("2 + 3 * (5 - 2) / 1.5;", 8.0, log)
unit_test("letFunc f(x) { return x * \u221a(4); } f(3);", 6, log)

# Edge Cases
unit_test("0 / 1;", 0, log)
unit_test("-0;", 0, log)
unit_test("var x := 0; x := x + 0; x;", 0, log)

# Print Summary
print("\nTest Summary:")
passed_count = 0
for status, expr, error_msg in log:
    if status == "PASSED":
        print(f"{Fore.GREEN}PASSED: {expr}{Style.RESET_ALL}")
        passed_count += 1
    else:
        print(f"{Fore.RED}{status}: {expr}{Style.RESET_ALL}")
        if error_msg:
            print(f"  -> {error_msg}")

print(f"\nTotal Passed: {passed_count} out of {len(log)}")
assert passed_count == len(log), "Some tests failed!"
print("All tests passed successfully!")
