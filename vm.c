#include <stdio.h>
#include <stdint.h>

enum opcode {
    OP_HALT = 0,
    OP_NOP,
    OP_PUSH,
    OP_POP,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NEG
};

int execute(uint8_t *instructions)
{
    size_t PC = 0;
    int stack[256], top = 0;
    #define PUSH(v) (stack[top++] = v)
    #define POP() (stack[--top])

    int l, r;

    while (1)
    {
        // printf("PC=%zu, top=%d, op=%d\n", PC, top, instructions[PC]);
        // for (int i = 0; i < top; i++) printf("%d ", stack[i]);
        // printf("\n");   
        switch (instructions[PC])
        {
            case OP_HALT: goto end;
            case OP_NOP: break;
            case OP_PUSH:
                PUSH(instructions[PC + 1]); break;
            case OP_ADD:
                l = POP(); r = POP();
                PUSH(l + r); break;
            case OP_SUB:
                l = POP(); r = POP();
                PUSH(l - r); break;
            case OP_MUL:
                l = POP(); r = POP();
                PUSH(l * r); break;
            case OP_DIV:
                l = POP(); r = POP();
                PUSH(r / l); break;
            case OP_NEG:
                l = POP();
                PUSH(-l); break;
        }
        PC += 2;
    }
    end:
        return POP();
    
    #undef PUSH
    #undef POP
}

int main()
{
    uint8_t instructions[] = {
        OP_PUSH, 5,
        OP_PUSH, 3,
        OP_ADD,  0,
        OP_PUSH, 2,
        OP_MUL,  0,
        OP_PUSH, 4,
        OP_DIV,  0,
        OP_NEG,  0,
        OP_HALT, 0
    };
    printf("%d\n", execute(instructions));
    return 0;
}