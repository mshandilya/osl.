from src.osl_eval import e
from src.osl_parser import parse, resolve
import time
import sys
from colorama import Fore, Style
# from codegen import *
# from vm import StackVM, Code

sys.setrecursionlimit(100000000)

def run_test(exp, expected, label):
    print(f"\n{label} osl Code:")
    print(exp)
    start_time = time.time()
    result = e(resolve(parse(exp)))
    end_time = time.time() - start_time

    # code = Code(bytecode=result)

    # stack = StackVM(code)
    # t2 = time.time()
    # result = stack.execute()
    # t1 = time.time() - t2
    print(f"Expected: {expected}")
    print(f"Result: {result}")
    # print(f"osl compilation Time: {Fore.CYAN}{end_time:.6f} seconds{Style.RESET_ALL}")
    # print(f"osl execution Time: {Fore.CYAN}{t1:.6f} seconds{Style.RESET_ALL}")
    print(f"osl execution Time: {Fore.CYAN}{end_time:.6f} seconds{Style.RESET_ALL}")
    # print(f"osl total Time: {Fore.CYAN}{t1+end_time:.6f} seconds{Style.RESET_ALL}")
    return end_time

# Euler Problem 1: Sum of multiples of 3 or 5
exp1 = """
def F(x, s) {
    while (x < 1000) {
        if (x % 3 = 0 || x % 5 = 0) {
            s := s + x;
        }
        x := x + 1;
    }
    return s;
}
F(0, 0);
"""
t1 = run_test(exp1, 233168, "Problem 1")

def F(x, s):
    while x < 1000:
        if x % 3 == 0 or x % 5 == 0:
            s += x
        x += 1
    return s

start_time = time.time()
py_result1 = F(0, 0)
t2 = time.time() - start_time
print(f"Python Result: {py_result1}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")

# Euler Problem 2: Even Fibonacci numbers
exp2 = """
def fib(a, b, s) {
    while (a < 4000000) {
        if (a % 2 = 0) {
            s := s + a;
        }
        var temp := a;
        a := b;
        b := temp + b;
    }
    return s;
}
fib(0, 1, 0);
"""
t1 = run_test(exp2, 4613732, "Problem 2")

def fib(a, b, s):
    while a < 4000000:
        if a % 2 == 0:
            s += a
        a, b = b, a + b
    return s

start_time = time.time()
py_result2 = fib(0, 1, 0)
t2 = time.time() - start_time
print(f"Python Result: {py_result2}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")

# Euler Problem 3: Largest prime factor
exp3 = """
def prime(n, i) {
    while (i * i <= n) {
        if (n % i = 0) {
            n := n / i;
        } else {
            i := i + 1;
        }
    }
    return n;
}
var n := 600851475143;
prime(n, 2);
"""
t1 = run_test(exp3, 6857, "Problem 3")

def largest_prime_factor(n, i):
    while i * i <= n:
        if n % i == 0:
            n //= i
        else:
            i += 1
    return n

start_time = time.time()
py_result3 = largest_prime_factor(600851475143, 2)
t2 = time.time() - start_time
print(f"Python Result: {py_result3}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")

# Euler Problem 4: Largest palindrome product
exp4 = """
def isPal(n) {
    var rev := 0;
    var nn := n;
    while (n > 0)
    {
        rev := rev * 10 + n % 10;
        n := n / 10;
    }
    return rev=nn;
}

var maxPal := 0;
var i := 999;
var j;
var prod;
while (i >= 1) {
    j := i;
    while (j >= 1) {
        prod := i * j;
        if ((prod > maxPal) && (isPal(prod))) {
            maxPal := prod;
        }
        j := j - 1;
    }
    i := i - 1;
}
maxPal;
"""

t1 = run_test(exp4, 906609, "Problem 4")

def is_palindrome(n):
    return str(n) == str(n)[::-1]

def F():
    max_pal = 0
    i = 999
    while i >= 100:
        j = i
        while j >= 100:
            prod = i * j
            if prod > max_pal and is_palindrome(prod):
                max_pal = prod
            j -= 1
        i -= 1
    return max_pal

start_time = time.time()
py_result4 = F()
t2 = time.time() - start_time
print(f"Python Result: {py_result4}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")


# Euler Problem 5: Smallest multiple
exp5 = """
def gcd(a, b) {
    while (b != 0) {
        var temp := b;
        b := a % b;
        a := temp;
    }
    return a;
}
def lcm(a, b) {
    return a * b / gcd(a, b);
}
def F(n, i) {
    while (i > 1) {
        n := lcm(n, i - 1);
        i := i - 1;
    }
    return n;
}
F(1, 20);
"""
t1 = run_test(exp5, 232792560, "Problem 5")

def gcd(a, b):
    while b != 0:
        a, b = b, a % b
    return a

def lcm(a, b):
    return a * b // gcd(a, b)

def F(n, i):
    while i > 1:
        n = lcm(n, i - 1)
        i -= 1
    return n

start_time = time.time()
py_result5 = F(1, 20)
t2 = time.time() - start_time
print(f"Python Result: {py_result5}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")

# Euler Problem 6: Sum square difference
exp6 = """
def F(n, sum, sumSq) {
    while (n > 0) {
        sum := sum + n;
        sumSq := sumSq + n * n;
        n := n - 1;
    }
    return sum * sum - sumSq;
}
F(100, 0, 0);
"""
t1 = run_test(exp6, 25164150, "Problem 6")

def F(n, sum, sumSq):
    while n > 0:
        sum += n
        sumSq += n * n
        n -= 1
    return sum * sum - sumSq

start_time = time.time()
py_result6 = F(100, 0, 0)
t2 = time.time() - start_time
print(f"Python Result: {py_result6}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")

# Euler Problem 7: 10001st prime number
exp7 = """
def find_nth_prime(n)
{
    var limit := 150000;
    var is_prime[limit + 1];
    
    var i := 0;
    var j;
    
    while (i < limit + 1) 
    {
        is_prime[i] := 1;
        i := i + 1;
    }
    
    is_prime[0] := 0;
    is_prime[1] := 0;
    
    i := 2;
    while (i * i <= limit)
    {
        if (is_prime[i])
        {
            j := i * i;
            while (j <= limit)
            {
                is_prime[j] := 0;
                j := j + i;
            }       
        }
        i := i + 1;
    }
    
    var count := 0;
    var num := 2;
    while (num <= limit)
    {
        if (is_prime[num])
        {
            count := count + 1;
            if (count = n) return num;
        }
        num := num + 1;
    }
    return -1;   
}

var n := 10001;
find_nth_prime(n);
"""

t1 = run_test(exp7, 104743, "Problem 7")

def find_nth_prime(n):
    limit = 150000
    is_prime = [1] * (limit + 1)
    is_prime[0] = is_prime[1] = 0
    
    i = 2
    while i * i <= limit:
        if is_prime[i]:
            j = i * i
            while j <= limit:
                is_prime[j] = 0
                j += i
        i += 1

    count = 0
    num = 2
    while num <= limit:
        if is_prime[num]:
            count += 1
            if count == n:
                return num
        num += 1
    
    return -1

n = 10001

start_time = time.time()
py_result6 = find_nth_prime(n)
t2 = time.time() - start_time
print(f"Python Result: {py_result6}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")

# Euler Problem 9: Special Pythagorean triplet
exp9 = """
def find_pythagorean_triplet()
{
    var a := 1;
    var denom;
    var b;
    var c;
    while (a < 333)
    {
        denom := 1000 - a;
        if ((500000 % denom) = 0)
        {
            b := 1000 - (500000/denom);
            if (a < b)
            {
                c := 1000 - a - b;
                if ((b < c) && ((a * a) + (b * b) = (c * c)))
                    return a * b * c;
            }
                
        }
        a := a + 1;
    }
    return -1;
}
find_pythagorean_triplet();
"""

t1 = run_test(exp9, 31875000, "Problem 9")

def find_pythagorean_triplet():
    a = 1
    while a < 333:
        denom = 1000 - a
        if 500000 % denom == 0:
            b = 1000 - 500000 // denom
            if a < b:
                c = 1000 - a - b
                if b < c and a * a + b * b == c * c:
                    return a * b * c
        a += 1
    return -1

start_time = time.time()
py_result6 = find_pythagorean_triplet()
t2 = time.time() - start_time
print(f"Python Result: {py_result6}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")

# Euler Problem 10: Summation of primes
exp10 = """
def find_nth_prime()
{
    var limit := 2000000;
    var is_prime[limit];
    
    var i := 0;
    var j;
    
    while (i < limit) 
    {
        is_prime[i] := 1;
        i := i + 1;
    }
    
    is_prime[0] := 0;
    is_prime[1] := 0;
    
    i := 2;
    while (i * i < limit)
    {
        if (is_prime[i])
        {
            j := i * i;
            while (j < limit)
            {
                is_prime[j] := 0;
                j := j + i;
            }       
        }
        i := i + 1;
    }
    
    var sum := 0;
    var num := 2;
    while (num < limit)
    {
        if (is_prime[num]) sum := sum + num;
        num := num + 1;
    }
    return sum;   
}

find_nth_prime();
"""

t1 = run_test(exp10, 142913828922, "Problem 10")

def sum_primes_below_n():
    n = 2000000
    is_prime = [1] * n
    is_prime[0] = 0
    is_prime[1] = 0
    
    i = 2
    while i * i < n:
        if is_prime[i]:
            j = i * i
            while j < n:
                is_prime[j] = 0
                j += i
        i += 1
    
    total = 0
    num = 2
    while num < n:
        if is_prime[num]:
            total += num
        num += 1
    
    return total

start_time = time.time()
py_result6 = sum_primes_below_n()
t2 = time.time() - start_time
print(f"Python Result: {py_result6}")
print(f"Python Time: {Fore.CYAN}{t2:.6f} seconds{Style.RESET_ALL}")
print(f"osl is {int(t1//t2)}x slower than Python")