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
