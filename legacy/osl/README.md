# Python Compiler and C VM

Write the osl code in `code.osl`.
To simply run the tree-walk interpreter:

```bash
python3 run.py [-i | --run]
```

To compile to bytcode:

```bash
python3 run.py [-c | --compile]
```

To execute the bytocode using C VM:

```bash
./rosl <path_to_bytecode_file>
```


# Coverage report
To generate the coverage report, run:
```bash
~/Desktop/osl./legacy/osl$ coverage run --include="coverage run --include="src/codegen.py,src/visualizer.py,src/osl_lexer.py,src/osl_parser.py" run.py code_cover.osl -c

coverage html
```