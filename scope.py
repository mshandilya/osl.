def fact(n):
    return 1 if n <= 1 else n*fact(n-1)

baz = fact
print(baz(3)) # 6 [in the above fact(n), the n*fact(n-1), here fact(n-1) is the same fact function]

def fact(n):
    return 1

print(baz(3)) # 3 [in the above fact(n), the n*fact(n-1), here fact(n-1) is the new fact function below]