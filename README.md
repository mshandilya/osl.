# CS327-Compilers

```python
ast = parse("if 2 < 3 then 0+5 else 1*6 end")
dot = visualize_ast(ast)
dot.render("ast_cond", format="png", cleanup=True)
```

<div align = "center">
    <img src = "https://github.com/guntas-13/CS327-Compilers/blob/master/ast_cond.png" style="width: 50%">
</div>

```python
exp = "if 2 < 3 then if 4 > 5 then 1 else if 6 <= 7 then 8 else 9 end end else 10 end"
ast = parse(exp)
dot = visualize_ast(ast)
dot.render("ast_nested_cond", format="png", cleanup=True)
```

<div align = "center">
    <img src = "https://github.com/guntas-13/CS327-Compilers/blob/master/ast_comparison.png" style="width: 50%">
</div>
