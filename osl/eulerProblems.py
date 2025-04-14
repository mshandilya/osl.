# from osl_package import e, resolve, parse
import time
import sys
from colorama import Fore, Style
from codegen import *
from vm import StackVM, Code

sys.setrecursionlimit(100000000)

def run_test(exp, expected, label):
    print(f"\n{label} osl Code:")
    print(exp)
    start_time = time.time()
    result = codegen(resolve(parse(exp)))
    end_time = time.time() - start_time

    code = Code(bytecode=result)

    stack = StackVM(code)
    t2 = time.time()
    result = stack.execute()
    t1 = time.time() - t2
    print(f"Expected: {expected}")
    print(f"osl compilation Time: {Fore.CYAN}{end_time:.6f} seconds{Style.RESET_ALL}")
    print(f"osl execution Time: {Fore.CYAN}{t1:.6f} seconds{Style.RESET_ALL}")
    print(f"osl total Time: {Fore.CYAN}{t1+end_time:.6f} seconds{Style.RESET_ALL}")
    return t1

# Euler Problem 1: Sum of multiples of 3 or 5
exp1 = """
fn F(x, s) {
    if (x = 1000) return s;
    if (x % 3 = 0) 
        return F(x + 1, s + x);
    if (x % 5 = 0)
        return F(x + 1, s + x);
    return F(x + 1, s);
}
log F(0, 0);
"""
t1 = run_test(exp1, 233168, "Problem 1")

def F(x, s):
    if x == 1000: return s
    if x % 3 == 0 or x % 5 == 0:
        return F(x + 1, s + x)
    return F(x + 1, s)

start_time = time.time()
py_result1 = F(0, 0)
t2 = time.time() - start_time
print(f"Python Result: {py_result1}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")

# Euler Problem 2: Even Fibonacci numbers
exp2 = """
fn fib(a, b, s) {
    if (a >= 4000000) return s;
    if (a % 2 = 0) 
        return fib(b, a + b, s + a);
    return fib(b, a + b, s);
}
log fib(0, 1, 0);
"""
t1 = run_test(exp2, 4613732, "Problem 2")

def fib(a, b, s):
    if a >= 4000000: return s
    if a % 2 == 0:
        return fib(b, a + b, s + a)
    return fib(b, a + b, s)

start_time = time.time()
py_result2 = fib(0, 1, 0)
t2 = time.time() - start_time
print(f"Python Result: {py_result2}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")

# Euler Problem 3: Largest prime factor
exp3 = """
fn prime(n, i) {
    if (i * i > n) return n;
    if (n % i = 0)
        return prime(n / i, i);
    return prime(n, i + 1);
}
var n := 600851475143;
log prime(n, 2);
"""
t1 = run_test(exp3, 6857, "Problem 3")

def largest_prime_factor(n, i):
    if i * i > n: return n
    if n % i == 0:
        return largest_prime_factor(n // i, i)
    return largest_prime_factor(n, i + 1)

start_time = time.time()
py_result3 = largest_prime_factor(600851475143, 2)
t2 = time.time() - start_time
print(f"Python Result: {py_result3}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")

# Euler Problem 4: Largest palindrome product
exp4 = """
fn isPal(n, rev, org) {
    if (n = 0)
    {
        if (org = rev) return 1;
        return 0;
    }
    return isPal(n/10, rev*10 + n%10, org);
}
fn F(i, j, maxPal) {
    if (i < 100) return maxPal;
    if (j < 100) return F(i - 1, i - 1, maxPal);
    var prod := i * j;
    if ((prod > maxPal) && (isPal(prod, 0, prod)))
        maxPal := prod;
    return F(i, j - 1, maxPal);
}
log F(999, 999, 0);
"""
t1 = run_test(exp4, 906609, "Problem 4")

def isPal(n, rev, org):
    if n == 0: return org == rev
    return isPal(n // 10, rev * 10 + n % 10, org)

def F(i, j, maxPal):
    if i < 100: return maxPal
    if j < 100: return F(i - 1, i - 1, maxPal)
    prod = i * j
    if prod > maxPal and isPal(prod, 0, prod):
        maxPal = prod
    return F(i, j - 1, maxPal)

start_time = time.time()
py_result4 = F(999, 999, 0)
t2 = time.time() - start_time
print(f"Python Result: {py_result4}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")


# Euler Problem 5: Smallest multiple
exp5 = """
fn gcd(a, b) {
    if (b = 0) return a;
    return gcd(b, a % b);
}
fn lcm(a, b) {
    return a * b / gcd(a, b);
}
fn F(n, i) {
    if (i = 1) return n;
    return F(lcm(n, i - 1), i - 1);
}
log F(1, 20);
"""
t1 = run_test(exp5, 232792560, "Problem 5")

def gcd(a, b):
    if b == 0: return a
    return gcd(b, a % b)

def lcm(a, b):
    return a * b // gcd(a, b)

def F(n, i):
    if i == 1: return n
    return F(lcm(n, i - 1), i - 1)

start_time = time.time()
py_result5 = F(1, 20)
t2 = time.time() - start_time
print(f"Python Result: {py_result5}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")

# Euler Problem 6: Sum square difference
exp6 = """
fn F(n, sum, sumSq) {
    if (n = 0) return sum * sum - sumSq;
    return F(n - 1, sum + n, sumSq + n * n);
}
log F(100, 0, 0);
"""
t1 = run_test(exp6, 25164150, "Problem 6")

def F(n, sum, sumSq):
    if n == 0: return sum * sum - sumSq
    return F(n - 1, sum + n, sumSq + n * n)

start_time = time.time()
py_result6 = F(100, 0, 0)
t2 = time.time() - start_time
print(f"Python Result: {py_result6}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")