import dis

x = 5

def foo():
    x = 10
    def bar():
        x = 15
        def foobar():
            x = 20
            return x
        print(x)
        return foobar
    print(x)
    return bar

f = foo() # bar
g = f()   # foobar
print(g())
print(x)

dis.dis(foo)
print("\n=============================\n")
dis.dis(f)
print("\n=============================\n")
dis.dis(g)