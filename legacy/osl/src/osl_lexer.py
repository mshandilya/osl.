from collections.abc import Iterator
from src.cosl import *

def lex(s: str) -> Iterator[Token]:
    i = 0

    while True:
        while i < len(s) and s[i].isspace():
            i += 1

        if i >= len(s):
            return

        if s[i].isalpha() or s[i] == '_':
            start = i
            while i < len(s) and (s[i].isalpha() or s[i].isdigit() or s[i] == '_'):
                i += 1
                name = s[start:i]

            if name in {"if", "else", "var", "def", "log", "return", "while"}:
                yield KeyWordToken(name)

            else:
                yield VariableToken(name)
        
        elif s[i] == '/' and i + 1 < len(s) and s[i + 1] == '/':
            i += 2
            while i < len(s) and s[i] != '\n':
                i += 1
        
        elif s[i] == "'":
            i += 1
            if i < len(s) and s[i] != "'":
                char = s[i]
                i += 1
                if i < len(s) and s[i] == "'":
                    i += 1
                    yield CharToken(char)
                else:
                    raise ValueError("Unterminated character literal")
            else:
                raise ValueError("Empty character literal")
        
        elif s[i] == '"':
            i += 1
            start = i
            string = ""
            while i < len(s):
                char = s[i]
                if char == '"':
                    i += 1
                    yield StringToken(string)
                    break
                if char == '\\':
                    i += 1
                    if i >= len(s):
                        raise ValueError("Unterminated string")
                    escape_char = s[i]
                    if escape_char == '"':
                        string += '"'
                    elif escape_char == '\\':
                        string += '\\'
                    elif escape_char == '/':
                        string += '/'
                    elif escape_char == 'b':
                        string += '\b'
                    elif escape_char == 'f':
                        string += '\f'
                    elif escape_char == 'n':
                        string += '\n'
                    elif escape_char == 'r':
                        string += '\r'
                    elif escape_char == 't':
                        string += '\t'
                    elif escape_char == 'u':
                        if i + 4 >= len(s):
                            raise ValueError("Unterminated unicode escape sequence")
                        hex_value = s[i+1:i+5]
                        if all(c in '0123456789abcdefABCDEF' for c in hex_value):
                            string += chr(int(hex_value, 16))
                            i += 4
                        else:
                            raise ValueError(f"Invalid unicode escape sequence at index {i}: \\u{hex_value}")
                    else:
                        raise ValueError(f"Invalid escape character at index {i}: \\{escape_char}")
                else:
                    string += char
                i += 1
            else:
                raise ValueError("Unterminated string")
        
        elif s[i].isdigit():
            start = i
            while i < len(s) and (s[i].isdigit() or s[i] == '.'):
                i += 1
            prev_token = NumberToken(s[start:i])
            yield prev_token

        else:
            if s[i:i+2] in {"<=", ">=", "~=", "||", "&&", ":="}:
                prev_token = OperatorToken(s[i:i+2])
                yield prev_token
                i += 2
            elif s[i] in {'+', '*', '/', '^', '-', '(', ')', '<', '>', '=', '%', ",", "{", "}", ";", '[', ']', '~'}:
                prev_token = OperatorToken(s[i])
                yield prev_token
                i += 1
            else:
                raise ParseErr(f"Unexpected character: {s[i]} at position {i}")