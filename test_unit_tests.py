import pytest
from calculator_extended_resolved import parse, e, resolve
from io import StringIO
import sys

# Fixture to capture print output
@pytest.fixture
def capture_output():
    def _capture_output(func):
        old_stdout = sys.stdout
        new_stdout = StringIO()
        sys.stdout = new_stdout
        try:
            e = func()
            return new_stdout.getvalue().strip() + "\n" + str(e)
        finally:
            sys.stdout = old_stdout
    return _capture_output

def test_basic_arithmetic():
    assert e(resolve(parse("2;"))) == 2
    assert e(resolve(parse("2 + 3;"))) == 5
    assert e(resolve(parse("2 + 3 * 5;"))) == 17
    assert e(resolve(parse("(2 + 3) * 5;"))) == 25
    assert e(resolve(parse("2 + 3 * 5 ^ 2;"))) == 77
    assert e(resolve(parse("2.5 + 3.5;"))) == 6.0
    assert e(resolve(parse("2 ^ 3 ^ 2;"))) == 512
    assert e(resolve(parse("-5 * -5;"))) == 25
    assert e(resolve(parse("10 % 3;"))) == 1
    assert e(resolve(parse("\u221a(16);"))) == 4

def test_variables_and_assignment():
    assert e(resolve(parse("var x := 5; x;"))) == 5
    assert e(resolve(parse("var x := 5; x := x + 1; x;"))) == 6
    assert e(resolve(parse("var x := 10; var y := x * 2; y;"))) == 20

def test_functions():
    assert e(resolve(parse("letFunc f(x) { return x + 1; } f(5);"))) == 6
    assert e(resolve(parse("letFunc f(x) { var y := x * 2; return y; } f(3);"))) == 6

func_test1 = """
letFunc f(x) { 
    return x ^ 2; 
}
f(4);
"""
def test_function_multiline():
    assert e(resolve(parse(func_test1))) == 16

nested_func_test = """
var x := 5;
letFunc f(y) { 
    return x + y; 
}
f(3);
"""
def test_nested_function():
    assert e(resolve(parse(nested_func_test))) == 8

closure_test1 = """
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

closure_test2 = """
letFunc fun(F, x)
{
    return F(x);
}

letFunc square(x)
{
    return x ^ 2;
}

fun(square, 5);
"""

closure_test3 = """
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

def test_closure():
    assert e(resolve(parse(closure_test1))) == 10
    assert e(resolve(parse(closure_test2))) == 25
    assert e(resolve(parse(closure_test3))) == 25

def test_control_flow():
    assert e(resolve(parse("if (5 > 3) 10; else 20;"))) == 10
    assert e(resolve(parse("if (5 < 3) 10; else 20;"))) == 20
    assert e(resolve(parse("var x := 15; if (x > 10) x + 1; else x - 1;"))) == 16
    
lexical_scoping_test1 = """
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
def test_lexical_scoping(capture_output):
    output = capture_output(lambda: e(resolve(parse(lexical_scoping_test1))))
    assert output == "5\n5\n5"
    
block_test1 = """
var x := 5;
{
    var x := 10;
    x;
}
"""

block_test2 = """
var x := 5;
{
    var x := 10;
}
x;
"""

def test_block_scope():
    assert e(resolve(parse(block_test1))) == 10
    assert e(resolve(parse(block_test2))) == 5

print_test1 = """
var x := 42;
print(x);
"""
def test_print_simple(capture_output):
    output = capture_output(lambda: e(resolve(parse(print_test1))))
    assert output == "42\nNone"

print_test2 = """
letFunc f(x) {
    print(x * 2);
}
f(5);
"""
def test_print_function(capture_output):
    output = capture_output(lambda: e(resolve(parse(print_test2))))
    assert output == "10\nNone"

factorial_test = """
letFunc fact(n) {
    if (n = 0) return 1;
    return n * fact(n - 1);
}
fact(5);
"""
def test_recursion():
    assert e(resolve(parse(factorial_test))) == 120

def test_complex_expressions():
    assert e(resolve(parse("2 + 3 * (5 - 2) / 1.5;"))) == 8.0
    assert e(resolve(parse("letFunc f(x) { return x * \u221a(4); } f(3);"))) == 6

def test_edge_cases():
    assert e(resolve(parse("0 / 1;"))) == 0
    assert e(resolve(parse("-0;"))) == 0
    assert e(resolve(parse("var x := 0; x := x + 0; x;"))) == 0
    
euler_p1 = """
letFunc F(x, s) {
    if (x = 1000) return s;
    if (x % 3 = 0 || x % 5 = 0) 
        return F(x + 1, s + x);
    return F(x + 1, s);
}
F(0, 0);
"""

euler_p2 = """
letFunc fib(a, b, s) {
    if (a >= 4000000) return s;
    if (a % 2 = 0) 
        return fib(b, a + b, s + a);
    return fib(b, a + b, s);
}
fib(0, 1, 0);
"""

euler_p3 = """
letFunc prime(n, i) {
    if (i * i > n) return n;
    if (n % i = 0)
        return prime(n / i, i);
    return prime(n, i + 1);
}
var n := 600851475143;
prime(n, 2);
"""

euler_p4 = """
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

def test_euler():
    assert e(resolve(parse(euler_p1))) == 233168
    assert e(resolve(parse(euler_p2))) == 4613732
    assert e(resolve(parse(euler_p3))) == 6857
    assert e(resolve(parse(euler_p4))) == 906609