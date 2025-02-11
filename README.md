# OSL - The unambiguous language
`calculator.py` is in fact a full fleged compiler! Not just a calculator.
## Project Euler Questions solved in OSL
### Q1
#### OSL Code:
```
fun f(n, sum) is
   if n==1000 then sum
   else if n%3==0 then call(f, n+1, sum+n)
   else if n%5==0 then call(f, n+1, sum+n)
   else call(f, n+1, sum)
in call(f, 0, 0) end
```
#### ABT:
```
Fun(name='f',
    params=[Var(name='n', id=1), Var(name='sum', id=2)],
    body=If(condition_=BinOp(op='==',
                             left=Var(name='n', id=1),
                             right=Number(val='1000')),
            then_=Var(name='sum', id=2),
            else_=If(condition_=BinOp(op='==',
                                      left=BinOp(op='%',
                                                 left=Var(name='n', id=1),
                                                 right=Number(val='3')),
                                      right=Number(val='0')),
                     then_=Call(name='f',
                                arg=[BinOp(op='+',
                                           left=Var(name='n', id=1),
                                           right=Number(val='1')),
                                     BinOp(op='+',
                                           left=Var(name='sum', id=2),
                                           right=Var(name='n', id=1))]),
                     else_=If(condition_=BinOp(op='==',
                                               left=BinOp(op='%',
                                                          left=Var(name='n',
                                                                   id=1),
                                                          right=Number(val='5')),
                                               right=Number(val='0')),
                              then_=Call(name='f',
                                         arg=[BinOp(op='+',
                                                    left=Var(name='n', id=1),
                                                    right=Number(val='1')),
                                              BinOp(op='+',
                                                    left=Var(name='sum', id=2),
                                                    right=Var(name='n', id=1))]),
                              else_=Call(name='f',
                                         arg=[BinOp(op='+',
                                                    left=Var(name='n', id=1),
                                                    right=Number(val='1')),
                                              Var(name='sum', id=2)])))),
    expr=Call(name='f', arg=[Number(val='0'), Number(val='0')]))
```
#### Answer:
233168.0



### Q2)
#### OSL Code
```
fun fib(a, b, sum) is
   if a>4000000 then sum
   else if a%2 then call(fib, b, a+b, sum+a)
   else call(fib, b, a+b, sum)
in call(fib, 0, 1, 0) end
```
#### ABT
```
Fun(name='fib',
    params=[Var(name='a', id=5), Var(name='b', id=6), Var(name='sum', id=7)],
    body=If(condition_=BinOp(op='>',
                             left=Var(name='a', id=5),
                             right=Number(val='4000000')),
            then_=Var(name='sum', id=7),
            else_=If(condition_=BinOp(op='%',
                                      left=Var(name='a', id=5),
                                      right=Number(val='2')),
                     then_=Call(name='fib',
                                arg=[Var(name='b', id=6),
                                     BinOp(op='+',
                                           left=Var(name='a', id=5),
                                           right=Var(name='b', id=6)),
                                     BinOp(op='+',
                                           left=Var(name='sum', id=7),
                                           right=Var(name='a', id=5))]),
                     else_=Call(name='fib',
                                arg=[Var(name='b', id=6),
                                     BinOp(op='+',
                                           left=Var(name='a', id=5),
                                           right=Var(name='b', id=6)),
                                     Var(name='sum', id=7)]))),
    expr=Call(name='fib',
              arg=[Number(val='0'), Number(val='1'), Number(val='0')]))
```
#### Answer:
4613732.0
