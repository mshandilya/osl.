from calculator_extended import parse, e, resolve
from pprint import pprint
from colorama import Fore, Style

def unit_test(expr: str, expected_value, results):
    print(f"Expression: {expr}")
    try:
        # ast = resolve(parse(expr))
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
        results.append(("ERROR", expr, f"{Fore.RED}{error_msg}{Style.RESET_ALL}"))
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
unit_test(exp_sq, 7.0, log)

exp_cond1 = """
if 2 < 3 then
    0 + 5
else
    1 * 6
"""
unit_test("if 2 < 3 then 2 else 3", 2, log)
unit_test(exp_cond1, 5, log)


exp_cond = """
if 2 < 3 then 
    if 4 > 5 then 
        1 
    else if 6 <= 7 then 
        8 
    else 
        9 
else 
    10 
"""
unit_test(exp_cond, 8, log)


# Euler Problem 1
exp = """
letFunc func(x, s)
{
     if x = 1000 then
         s
     else if x % 3 = 0 || x % 5 = 0 then
         func(x + 1, s + x)
     else
         func(x + 1, s)
}
in
func(0, 0)
end
"""
unit_test(exp, 233168, log)


# Euler Problem 2
exp = """
letFunc fib(a, b, s)
{
    if a >= 4000000 then
        s
    else if a % 2 = 0 then
        fib(b, a + b, s + a)
    else
        fib(b, a + b, s)
}
in
fib(0, 1, 0)
end
"""
unit_test(exp, 4613732, log)

# Factorial
exp = """
letFunc fact(n)
{
    if n = 0 then
        1
    else
        let x := fact(n - 1) in
        n * x
        end
}
in
fact(5)
end
"""

unit_test(exp, 120, log)

# Fixed this -> added CallFun at the highest precedence in parse_atom()
exp = """
letFunc f(a)
{
    a + a
}
in
f(2) + f(3)
end
"""

unit_test(exp, 10, log)

exp = """
let x := 5 in
letFunc f(y) {
    x
} 
in
letFunc g(z) { 
    let x := 6 
    in f(z)
    end
}
in
g(0)
end
end
end
"""

unit_test(exp, 5, log)


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
