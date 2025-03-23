from calculator_extended_resolved import e, resolve, parse
import time
import sys
from colorama import Fore, Style

sys.setrecursionlimit(100000000)

def run_test(exp, expected, label):
    print(f"\n{label} osl Code:")
    print(exp)
    start_time = time.time()
    result = e(resolve(parse(exp)))
    t1 = time.time() - start_time
    print(f"Result: {result}")
    print(f"osl Time: {Fore.CYAN}{t1:.6f} seconds{Style.RESET_ALL}")
    assert result == expected, f"{label} failed: Expected {expected}, got {result}"
    return t1

# Euler Problem 1: Sum of multiples of 3 or 5
exp1 = """
letFunc F(x, s) {
    if (x = 1000) return s;
    if (x % 3 = 0 || x % 5 = 0) 
        return F(x + 1, s + x);
    return F(x + 1, s);
}
F(0, 0);
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
letFunc fib(a, b, s) {
    if (a >= 4000000) return s;
    if (a % 2 = 0) 
        return fib(b, a + b, s + a);
    return fib(b, a + b, s);
}
fib(0, 1, 0);
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
letFunc prime(n, i) {
    if (i * i > n) return n;
    if (n % i = 0)
        return prime(n / i, i);
    return prime(n, i + 1);
}
var n := 600851475143;
prime(n, 2);
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
letFunc isPal(n, rev, org) {
    if (n = 0) return rev = org;
    return isPal(n/10, rev*10 + n%10, org);
}
letFunc F(i, j, maxPal) {
    if (i < 100) return maxPal;
    if (j < 100) return F(i - 1, i - 1, maxPal);
    var prod := i * j;
    if ((prod > maxPal) && (isPal(prod, 0, prod)))
        maxPal := prod;
    return F(i, j - 1, maxPal);
}
F(999, 999, 0);
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