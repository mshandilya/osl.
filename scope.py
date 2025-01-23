# def fact(n):
#     return 1 if n <= 1 else n*fact(n-1)

# baz = fact
# print(baz(3)) # 6 [in the above fact(n), the n*fact(n-1), here fact(n-1) is the same fact function]

# def fact(n):
#     return 1

# print(baz(3)) # 3 [in the above fact(n), the n*fact(n-1), here fact(n-1) is the new fact function below]

def make_ctr():
    v = 0
    def inc():
        nonlocal v # nonlocal keyword is used to declare that v is not a local variable
        v = v + 1 
    
    def get():
        return v
    
    return inc, get

i, g = make_ctr()
i()
print(g()) # 1

i1, g1 = make_ctr()
i1()
i1()
print(g1()) # 2