# osl.

```python
exp_cond1 = """
if 2 < 3 then
    0 + 5
else
    1 * 6
end
"""
ast = parse(exp_cond1)
dot = visualize_ast(ast)
dot.render("ast_cond", format="png", cleanup=True)
```

<div align = "center">
    <img src = "https://github.com/guntas-13/CS327-Compilers/blob/master/images/ast_cond.png" style="width: 50%">
</div>

```python
exp_cond = """
if 2 < 3 then
    if 4 > 5 then
        1
    else
        if 6 <= 7 then
            8
        else
            9
        end
    end
else
    10
end
"""
ast = parse(exp_cond)
dot = visualize_ast(ast)
dot.render("ast_nested_cond", format="png", cleanup=True)
```

<div align = "center">
    <img src = "https://github.com/guntas-13/CS327-Compilers/blob/master/images/ast_nested_cond.png" style="width: 50%">
</div>

```python
exp_sq = "\u221a(4 + 12) + \u221a(9)"
ast = parse(exp_sq)
dot = visualize_ast(ast)
dot.render("ast_sqrt", format="png", cleanup=True)
```

&radic;(4 + 12) + &radic;(9)

<div align = "center">
    <img src = "https://github.com/guntas-13/CS327-Compilers/blob/master/images/ast_sqrt.png" style="width: 20%">
</div>
