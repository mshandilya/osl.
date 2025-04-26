import dis

# def counter():
#     c = 0
#     def inner():
#         nonlocal c
#         c += 1
#         return c
#     return inner

# print(dis.dis(counter))
# print(dis.dis(counter()))
# print(counter.__closure__)

def f(a):
    def g(b):
        def h(c):
            return a + b + c
        return h
    return g

print(dis.dis(f))