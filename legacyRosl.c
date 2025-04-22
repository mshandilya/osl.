#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

typedef enum {
    VAL_CHAR,
    VAL_BOOL,
    VAL_INT,
    VAL_FLOAT,
    VAL_ID,
    VAL_OBJ
} ValueType;

typedef struct GCObject GCObject;

typedef struct Value {
    ValueType type;
    union {
        void*    v;
        char     c;
        int16_t  s;
        int32_t  i;
        int64_t  l;
        float    f;
        double   d;
        GCObject* obj;
    };
} Value;

/* GCObject structure:
   - A mark bit is used during GC.
   - field_count indicates how many Value fields the object holds.
   - The fields are allocated as a flexible array member.
*/
struct GCObject {
    uint8_t marked;
    GCObject* next;
    uint8_t field_count;
    Value fields[];
};

GCObject* gc_objects = NULL;
size_t total_allocated = 0;
size_t gc_threshold = 1024 * 10;  // Threshold (10KB)

typedef enum {
    // Data push instructions
    PUSH_CHAR   = 0x01,
    PUSH_SHORT  = 0x02,
    PUSH_INT    = 0x03,
    PUSH_LONG   = 0x04,
    PUSH_FLOAT  = 0x05,
    PUSH_DOUBLE = 0x06,

    // Stack manipulation
    POP         = 0x10,
    DUP         = 0x11,
    SWAP        = 0x12,
    OVER        = 0x13,

    // Arithmetic operations
    ADD         = 0x20,
    SUB         = 0x21,
    MUL         = 0x22,
    DIV         = 0x23,
    MOD         = 0x24,
    NEG         = 0x25,

    // Bitwise / Logical operations
    BITWISE_NOT = 0x30,
    BITWISE_AND = 0x31,
    BITWISE_OR  = 0x32,
    BITWISE_XOR = 0x33,

    // Comparisons
    EQ          = 0x40,
    NEQ         = 0x41,
    LT          = 0x42,
    GT          = 0x43,
    LE          = 0x44,
    GE          = 0x45,

    // Control flow
    JUMP            = 0x50,
    JUMP_IF_ZERO    = 0x51,
    JUMP_IF_NONZERO = 0x52,
    CALL            = 0x53,
    RETURN          = 0x54,
    HALT            = 0x55,

    // Type conversions
    I2F         = 0x60,
    F2I         = 0x61,
    I2D         = 0x62,
    D2I         = 0x63,
    F2D         = 0x64,
    D2F         = 0x65,

    // Heap Object Operations
    NEW_OBJECT  = 0x70,
    GET_FIELD   = 0x71,
    SET_FIELD   = 0x72,

    LOG         = 0x90
} Opcode;

GCObject* gc_alloc(uint8_t field_count) {
    size_t size = sizeof(GCObject) + sizeof(Value) * field_count;
    GCObject* obj = (GCObject*) malloc(size);
    if (!obj) {
        fprintf(stderr, "Out of memory\n");
        exit(1);
    }
    obj->marked = 0;
    obj->field_count = field_count;
    for (int i = 0; i < field_count; i++) {
        Value v;
        v.type = VAL_INT;
        v.i = 0;
        obj->fields[i] = v;
    }
    obj->next = gc_objects;
    gc_objects = obj;
    total_allocated += size;
    return obj;
}

void gc_mark(GCObject* obj) {
    if (obj == NULL || obj->marked) return;
    obj->marked = 1;
    for (int i = 0; i < obj->field_count; i++) {
        Value v = obj->fields[i];
        if (v.type == VAL_OBJ && v.obj != NULL) {
            gc_mark(v.obj);
        }
    }
}

void gc_mark_value(Value v) {
    if (v.type == VAL_OBJ && v.obj != NULL) {
        gc_mark(v.obj);
    }
}

void gc_mark_roots(Value* stack, int top) {
    for (int i = 0; i < top; i++) {
        gc_mark_value(stack[i]);
    }
}

void gc_sweep() {
    GCObject** obj = &gc_objects;
    while (*obj) {
        if (!(*obj)->marked) {
            GCObject* unreached = *obj;
            *obj = unreached->next;
            total_allocated -= (sizeof(GCObject) + sizeof(Value) * unreached->field_count);
            free(unreached);
        } else {
            (*obj)->marked = 0;
            obj = &(*obj)->next;
        }
    }
}

void gc_collect(Value* stack, int top) {
    gc_mark_roots(stack, top);
    gc_sweep();
}
/*
unsigned int callScope = 0, sNodeTail = 0, currentRetAddress = 0;

typedef struct {
    ValueType type;
    unsigned int scopeId;
    void* location;
    llNode *prev, *next;
} llNode;

typedef struct {
    llNode* heads[1<<15];
    llNode* tails[1<<15];
} adjList;

typedef struct {
    unsigned int id, scopeId;
} sNode;

adjList vals;

sNode sNodeStack[1<<15];

void initialize() {

}

void set(unsigned int id, ValueType t, void* val) {
    if(vals.tails[id] == NULL) {
        vals.heads[id] = (llNode*)malloc(sizeof(llNode));
        vals.tails[id] = vals.heads[id];
        vals.tails[id]->next = vals.tails[id]->prev = NULL;
        vals.tails[id]->type = t;
        vals.tails[id]->location = val;
        vals.tails[id]->scopeId = callScope;
        sNodeStack[sNodeTail++] = (sNode){id, callScope};
    }
    else if(vals.tails[id]->scopeId == callScope || vals.tails[id]->scopeId == 0) {
        vals.tails[id]->type = t;
        vals.tails[id]->location = val;
    }
    else {
        vals.tails[id]->next = (llNode*)malloc(sizeof(llNode));
        vals.tails[id]->next->prev = vals.tails[id];
        vals.tails[id] = vals.tails[id]->next;
        vals.tails[id]->next = NULL;
        vals.tails[id]->type = t;
        vals.tails[id]->location = val;
        vals.tails[id]->scopeId = callScope;
        sNodeStack[sNodeTail++] = (sNode){id, callScope};
    }
}

void get(unsigned int id) {
    PUSH(vals.tails[id]->location);
    PUSH(vals.tails[id]->type);
}

// to be called before the actual call occurs
void enhanceScope(unsigned int nRetAddress) {
    PUSH(currentRetAddress);
    callScope++;
    currentRetAddress = nRetAddress;
}*/


#define STACK_SIZE 4096

#define PUSH(v) do { \
    if (top < STACK_SIZE) { \
        stack[top++] = (v); \
    } else { \
        fprintf(stderr, "Stack overflow\n"); \
        exit(1); \
    } \
} while(0)

#define POP() ( top > 0 ? stack[--top] : (fprintf(stderr, "Stack underflow\n"), exit(1), stack[0]) )

int execute(uint8_t *code, size_t codeSize) {
    size_t pc = 0;
    Value stack[STACK_SIZE];
    int top = 0;

    while (pc < codeSize) {
        uint8_t op = code[pc];
        switch (op) {
            case HALT:
                goto end;

            case 0: // NOP
                pc += 1;
                break;

            case LOG: {
                if (top < 1) { fprintf(stderr, "Stack underflow on LOG\n"); exit(1); }
                Value v = POP();
                if (v.type == VAL_INT) {
                    printf("%lld\n", *(int64_t*)v.v);
                } else if (v.type == VAL_CHAR) {
                    printf("%c\n", *(char*)v.v);
                } else if (v.type == VAL_FLOAT) {
                    printf("%f\n", *(float*)v.v);
                }else {
                    fprintf(stderr, "LOG supports only INT, CHAR, and FLOAT values.\n");
                    exit(1);
                }
                pc += 1;
                break;
            }

            // Data Push Instructions
            case PUSH_CHAR: {
                if (pc + 1 >= codeSize) { fprintf(stderr, "Unexpected end (PUSH_CHAR)\n"); exit(1); }
                Value v; v.type = VAL_CHAR;
                v.v = malloc(sizeof(char));
                memcpy(v.v, &code[pc+1], sizeof(int64_t));
                PUSH(v);
                pc += 2;
                break;
            }
            /*case PUSH_SHORT: {
                if (pc + 2 >= codeSize) { fprintf(stderr, "Unexpected end (PUSH_SHORT)\n"); exit(1); }
                Value v; v.type = VAL_SHORT;
                int16_t s = code[pc+1] | (code[pc+2] << 8);
                v.s = s;
                PUSH(v);
                pc += 3;
                break;
            }*/
            case PUSH_INT: {
                if (pc + 8 >= codeSize) { fprintf(stderr, "Unexpected end (PUSH_INT)\n"); exit(1); }
                Value v; v.type = VAL_INT;
                int64_t l = 0;
                for (int j = 0; j < 8; j++) {
                    l |= ((int64_t)code[pc+1+j]) << (8*j);
                }
                v.v = malloc(sizeof(int64_t));
                memcpy(v.v, &l, sizeof(int64_t));
                PUSH(v);
                pc += 9;
                break;
            }
            /*case PUSH_LONG: {
                if (pc + 8 >= codeSize) { fprintf(stderr, "Unexpected end (PUSH_LONG)\n"); exit(1); }
                Value v; v.type = VAL_LONG;
                int64_t l = 0;
                for (int j = 0; j < 8; j++) {
                    l |= ((int64_t)code[pc+1+j]) << (8*j);
                }
                v.l = l;
                PUSH(v);
                pc += 9;
                break;
            }*/
            case PUSH_FLOAT: {
                if (pc + 4 >= codeSize) { fprintf(stderr, "Unexpected end (PUSH_FLOAT)\n"); exit(1); }
                Value v; v.type = VAL_FLOAT;
                uint32_t tmp = code[pc+1] | (code[pc+2] << 8) |
                               (code[pc+3] << 16) | (code[pc+4] << 24);
                float f;
                memcpy(&f, &tmp, sizeof(f));
                v.v = malloc(sizeof(float));
                memcpy(v.v, &f, sizeof(float));
                PUSH(v);
                pc += 5;
                break;
            }
            /*case PUSH_DOUBLE: {
                if (pc + 8 >= codeSize) { fprintf(stderr, "Unexpected end (PUSH_DOUBLE)\n"); exit(1); }
                Value v; v.type = VAL_DOUBLE;
                uint64_t tmp = 0;
                for (int j = 0; j < 8; j++) {
                    tmp |= ((uint64_t)code[pc+1+j]) << (8*j);
                }
                double d;
                memcpy(&d, &tmp, sizeof(d));
                v.d = d;
                PUSH(v);
                pc += 9;
                break;
            }*/

            // Stack Manipulation
            case POP: {
                (void) POP();
                pc += 1;
                break;
            }
            case DUP: {
                if (top == 0) { fprintf(stderr, "Stack underflow on DUP\n"); exit(1); }
                PUSH(stack[top-1]);
                pc += 1;
                break;
            }
            case SWAP: {
                if (top < 2) { fprintf(stderr, "Stack underflow on SWAP\n"); exit(1); }
                Value temp = stack[top-1];
                stack[top-1] = stack[top-2];
                stack[top-2] = temp;
                pc += 1;
                break;
            }
            case OVER: {
                if (top < 2) { fprintf(stderr, "Stack underflow on OVER\n"); exit(1); }
                PUSH(stack[top-2]);
                pc += 1;
                break;
            }

            // Arithmetic Operations
            case ADD: {
                Value b = POP();
                Value a = POP();
                if (a.type == VAL_INT && b.type == VAL_INT) {
                    Value result; result.type = VAL_INT;
                    result.v = malloc(sizeof(int64_t));
                    *(int64_t*)result.v = *(int64_t*)a.v + *(int64_t*)b.v;
                    PUSH(result);
                } else if (a.type == VAL_FLOAT || b.type == VAL_FLOAT) {
                    Value result; result.type = VAL_FLOAT;
                    result.v = malloc(sizeof(float));
                    *(float*)result.v = *(float*)a.v + *(float*)b.v;
                    PUSH(result);
                }else if (a.type == VAL_CHAR || b.type == VAL_CHAR) {
                    Value result; result.type = VAL_CHAR;
                    result.v = malloc(sizeof(char));
                    *(char*)result.v = *(char*)a.v + *(char*)b.v;
                    PUSH(result);
                }else { fprintf(stderr, "ADD supports only INT, FLOAT, DOUBLE, and CHAR values.\n"); exit(1); }

                /*if (a.type == VAL_INT && b.type == VAL_INT) {
                    Value result; result.type = VAL_INT; result.i = a.i + b.i;
                    PUSH(result);
                } else { fprintf(stderr, "ADD supports only INT values.\n"); exit(1); }*/
                pc += 1;
                break;
            }
            case SUB: {
                Value b = POP();
                Value a = POP();
                if (a.type == VAL_INT && b.type == VAL_INT) {
                    Value result; result.type = VAL_INT;
                    result.v = malloc(sizeof(int64_t));
                    *(int64_t*)result.v = *(int64_t*)a.v - *(int64_t*)b.v;
                    PUSH(result);
                } else if (a.type == VAL_FLOAT || b.type == VAL_FLOAT) {
                    Value result; result.type = VAL_FLOAT;
                    result.v = malloc(sizeof(float));
                    *(float*)result.v = *(float*)a.v - *(float*)b.v;
                    PUSH(result);
                }else if (a.type == VAL_CHAR || b.type == VAL_CHAR) {
                    Value result; result.type = VAL_CHAR;
                    result.v = malloc(sizeof(char));
                    *(char*)result.v = *(char*)a.v - *(char*)b.v;
                    PUSH(result);
                }else { fprintf(stderr, "SUB supports only INT values.\n"); exit(1); }
                pc += 1;
                break;
            }
            case MUL: {
                Value b = POP();
                Value a = POP();
                if (a.type == VAL_INT && b.type == VAL_INT) {
                    Value result; result.type = VAL_INT;
                    result.v = malloc(sizeof(int64_t));
                    *(int64_t*)result.v = *(int64_t*)a.v * *(int64_t*)b.v;
                    PUSH(result);
                } else if (a.type == VAL_FLOAT || b.type == VAL_FLOAT) {
                    Value result; result.type = VAL_FLOAT;
                    result.v = malloc(sizeof(float));
                    *(float*)result.v = *(float*)a.v * *(float*)b.v;
                    PUSH(result);
                }else if (a.type == VAL_CHAR || b.type == VAL_CHAR) {
                    Value result; result.type = VAL_CHAR;
                    result.v = malloc(sizeof(char));
                    *(char*)result.v = *(char*)a.v * *(char*)b.v;
                    PUSH(result);
                }else { fprintf(stderr, "MUL supports only INT values.\n"); exit(1); }
                pc += 1;
                break;
            }
            case DIV: {
                Value b = POP();
                Value a = POP();
                if (a.type == VAL_INT && b.type == VAL_INT) {
                    Value result; result.type = VAL_INT;
                    result.v = malloc(sizeof(int64_t));
                    *(int64_t*)result.v = *(int64_t*)a.v / *(int64_t*)b.v;
                    PUSH(result);
                } else if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
                    Value result; result.type = VAL_FLOAT;
                    result.v = malloc(sizeof(float));
                    *(float*)result.v = *(float*)a.v / *(float*)b.v;
                    PUSH(result);
                }else if(a.type == VAL_FLOAT || b.type == VAL_FLOAT){
                    Value result; result.type = VAL_INT;
                    result.v = malloc(sizeof(int64_t));
                    *(int64_t*)result.v = *(int64_t*)a.v / *(int64_t*)b.v;
                    PUSH(result);
                }
                else { fprintf(stderr, "DIV supports only INT values.\n"); exit(1); }
                pc += 1;
                break;
            }
            case MOD: {
                Value b = POP();
                Value a = POP();
                if (a.type == VAL_INT && b.type == VAL_INT) {
                    if(*(int64_t*)b.v == 0) { fprintf(stderr, "Modulo by zero\n"); exit(1); }
                    Value result; result.type = VAL_INT;
                    result.v = malloc(sizeof(int64_t));
                    *(int64_t*)result.v = *(int64_t*)a.v % *(int64_t*)b.v;
                    PUSH(result);
                }else { fprintf(stderr, "MOD supports only INT values.\n"); exit(1); }
                pc += 1;
                break;
            }
            case NEG: {
                Value a = POP();
                if(a.type == VAL_INT){
                    Value result; result.type = VAL_INT;
                    result.v = malloc(sizeof(int64_t));
                    *(int64_t*)result.v = -*(int64_t*)a.v;
                    PUSH(result);
                }else if(a.type == VAL_FLOAT){
                    Value result; result.type = VAL_FLOAT;
                    result.v = malloc(sizeof(float));
                    *(float*)result.v = -*(float*)a.v;
                    PUSH(result);
                }else { fprintf(stderr, "NEG supports only INT and FLOAT values.\n"); exit(1); }
                pc += 1;
                break;
            }

            // Bitwise Operations
            case BITWISE_NOT: {
                Value a = POP();
                if (a.type == VAL_INT) { a.i = ~a.i; PUSH(a); }
                else { fprintf(stderr, "BITWISE_NOT supports only INT values.\n"); exit(1); }
                pc += 1;
                break;
            }
            case BITWISE_AND: {
                Value b = POP();
                Value a = POP();
                if (a.type == VAL_INT && b.type == VAL_INT) {
                    Value result; result.type = VAL_INT; result.i = a.i & b.i;
                    PUSH(result);
                } else { fprintf(stderr, "BITWISE_AND supports only INT values.\n"); exit(1); }
                pc += 1;
                break;
            }
            case BITWISE_OR: {
                Value b = POP();
                Value a = POP();
                if (a.type == VAL_INT && b.type == VAL_INT) {
                    Value result; result.type = VAL_INT; result.i = a.i | b.i;
                    PUSH(result);
                } else { fprintf(stderr, "BITWISE_OR supports only INT values.\n"); exit(1); }
                pc += 1;
                break;
            }
            case BITWISE_XOR: {
                Value b = POP();
                Value a = POP();
                if (a.type == VAL_INT && b.type == VAL_INT) {
                    Value result; result.type = VAL_INT; result.i = a.i ^ b.i;
                    PUSH(result);
                } else { fprintf(stderr, "BITWISE_XOR supports only INT values.\n"); exit(1); }
                pc += 1;
                break;
            }

            // Comparison Operations
            case EQ: {
                Value b = POP();
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.i = (a.type == VAL_INT && b.type == VAL_INT && a.i == b.i) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }
            case NEQ: {
                Value b = POP();
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.i = (a.type == VAL_INT && b.type == VAL_INT && a.i != b.i) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }
            case LT: {
                Value b = POP();
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.i = (a.type == VAL_INT && b.type == VAL_INT && a.i < b.i) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }
            case GT: {
                Value b = POP();
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.i = (a.type == VAL_INT && b.type == VAL_INT && a.i > b.i) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }
            case LE: {
                Value b = POP();
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.i = (a.type == VAL_INT && b.type == VAL_INT && a.i <= b.i) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }
            case GE: {
                Value b = POP();
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.i = (a.type == VAL_INT && b.type == VAL_INT && a.i >= b.i) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }

            // Control Flow
            case JUMP: {
                if (pc + 2 >= codeSize) { fprintf(stderr, "Unexpected end in JUMP\n"); exit(1); }
                int16_t offset = code[pc+1] | (code[pc+2] << 8);
                pc += 3;
                pc += offset;
                break;
            }
            case JUMP_IF_ZERO: {
                if (pc + 2 >= codeSize) { fprintf(stderr, "Unexpected end in JUMP_IF_ZERO\n"); exit(1); }
                int16_t offset = code[pc+1] | (code[pc+2] << 8);
                Value cond = POP();
                pc += 3;
                if (cond.type == VAL_INT && cond.i == 0) {
                    pc += offset;
                }
                break;
            }
            case JUMP_IF_NONZERO: {
                if (pc + 2 >= codeSize) { fprintf(stderr, "Unexpected end in JUMP_IF_NONZERO\n"); exit(1); }
                int16_t offset = code[pc+1] | (code[pc+2] << 8);
                Value cond = POP();
                pc += 3;
                if (cond.type == VAL_INT && cond.i != 0) {
                    pc += offset;
                }
                break;
            }
            case CALL: {
                if (pc + 2 >= codeSize) { fprintf(stderr, "Unexpected end in CALL\n"); exit(1); }
                int16_t addr = code[pc+1] | (code[pc+2] << 8);
                Value ret; ret.type = VAL_INT; ret.i = pc + 3;
                PUSH(ret);
                pc = addr;
                break;
            }
            case RETURN: {
                Value ret = POP();
                if (ret.type != VAL_INT) { fprintf(stderr, "Invalid return address type\n"); exit(1); }
                pc = ret.i;
                break;
            }

            // Type Conversion
            case I2F: {
                Value a = POP();
                if (a.type != VAL_INT) { fprintf(stderr, "I2F supports only INT values\n"); exit(1); }
                Value result; result.type = VAL_FLOAT; result.f = (float)a.i;
                PUSH(result);
                pc += 1;
                break;
            }
            case F2I: {
                Value a = POP();
                if (a.type != VAL_FLOAT) { fprintf(stderr, "F2I supports only FLOAT values\n"); exit(1); }
                Value result; result.type = VAL_INT; result.i = (int32_t)a.f;
                PUSH(result);
                pc += 1;
                break;
            }
            /*case I2D: {
                Value a = POP();
                if (a.type != VAL_INT) { fprintf(stderr, "I2D supports only INT values\n"); exit(1); }
                Value result; result.type = VAL_DOUBLE; result.d = (double)a.i;
                PUSH(result);
                pc += 1;
                break;
            }
            case D2I: {
                Value a = POP();
                if (a.type != VAL_DOUBLE) { fprintf(stderr, "D2I supports only DOUBLE values\n"); exit(1); }
                Value result; result.type = VAL_INT; result.i = (int32_t)a.d;
                PUSH(result);
                pc += 1;
                break;
            }
            case F2D: {
                Value a = POP();
                if (a.type != VAL_FLOAT) { fprintf(stderr, "F2D supports only FLOAT values\n"); exit(1); }
                Value result; result.type = VAL_DOUBLE; result.d = (double)a.f;
                PUSH(result);
                pc += 1;
                break;
            }
            case D2F: {
                Value a = POP();
                if (a.type != VAL_DOUBLE) { fprintf(stderr, "D2F supports only DOUBLE values\n"); exit(1); }
                Value result; result.type = VAL_FLOAT; result.f = (float)a.d;
                PUSH(result);
                pc += 1;
                break;
            }*/

            // Heap Object Operations
            case NEW_OBJECT: {
                if (pc + 1 >= codeSize) { fprintf(stderr, "Unexpected end in NEW_OBJECT\n"); exit(1); }
                uint8_t field_count = code[pc+1];
                GCObject* obj = gc_alloc(field_count);
                // Trigger GC if necessary
                if (total_allocated > gc_threshold) {
                    gc_collect(stack, top);
                }
                Value v; v.type = VAL_OBJ; v.obj = obj;
                PUSH(v);
                pc += 2;
                break;
            }
            case GET_FIELD: {
                if (pc + 1 >= codeSize) { fprintf(stderr, "Unexpected end in GET_FIELD\n"); exit(1); }
                uint8_t field_index = code[pc+1];
                Value objVal = POP();
                if (objVal.type != VAL_OBJ || objVal.obj == NULL) {
                    fprintf(stderr, "GET_FIELD: Not an object\n");
                    exit(1);
                }
                if (field_index >= objVal.obj->field_count) {
                    fprintf(stderr, "GET_FIELD: Field index out of bounds\n");
                    exit(1);
                }
                PUSH(objVal.obj->fields[field_index]);
                pc += 2;
                break;
            }
            case SET_FIELD: {
                if (pc + 1 >= codeSize) { fprintf(stderr, "Unexpected end in SET_FIELD\n"); exit(1); }
                uint8_t field_index = code[pc+1];
                Value valueToSet = POP();
                Value objVal = POP();
                if (objVal.type != VAL_OBJ || objVal.obj == NULL) {
                    fprintf(stderr, "SET_FIELD: Not an object\n");
                    exit(1);
                }
                if (field_index >= objVal.obj->field_count) {
                    fprintf(stderr, "SET_FIELD: Field index out of bounds\n");
                    exit(1);
                }
                objVal.obj->fields[field_index] = valueToSet;
                pc += 2;
                break;
            }

            default:
                fprintf(stderr, "Unknown opcode: %d\n", op);
                exit(1);
        }
    }
    end:
    if (top > 0) {
        Value result = POP();
        switch (result.type) {
            case VAL_INT:    printf("Result: %d\n", result.i); break;
            case VAL_CHAR:   printf("Result: %c\n", result.c); break;
            //case VAL_SHORT:  printf("Result: %d\n", result.s); break;
            //case VAL_LONG:   printf("Result: %lld\n", (long long)result.l); break;
            case VAL_FLOAT:  printf("Result: %f\n", result.f); break;
            //case VAL_DOUBLE: printf("Result: %lf\n", result.d); break;
            case VAL_OBJ:    printf("Result: [object at %p]\n", (void*)result.obj); break;
            default:         printf("Unknown result type\n"); break;
        }
    }
    return 0;
}

int main(int argc, char *argv[]) {
    /* This program computes: -( (5 + 3) * 2 / 4 ) to show arithmetic,
       then it allocates a new object with 2 fields, sets field 0 to the arithmetic
       result, and then retrieves field 0. */
    /*uint8_t program[] = {
        NEW_OBJECT, 2,            // Allocate new object with 2 fields
        DUP,                      // Duplicate the object pointer.
        // Arithmetic: (5 + 3) * 2 / 4, then NEG -> -4
        PUSH_INT,  5,  0,  0,  0,   // Push 5
        PUSH_INT,  3,  0,  0,  0,   // Push 3
        ADD,                      // 5 + 3 = 8
        PUSH_INT,  2,  0,  0,  0,   // Push 2
        MUL,                      // 8 * 2 = 16
        PUSH_INT,  4,  0,  0,  0,   // Push 4
        DIV,                      // 16 / 4 = 4
        NEG,                      // Negate: -4
        // We want to store the arithmetic result (-4) into field 0.
        SET_FIELD, 0,             // Pop value (-4) and object; store into field 0.
        // Push the object again, by duplicating it.
        GET_FIELD, 0,             // Get field 0 from the object.
        HALT                      // Terminate
    };
    size_t progSize = sizeof(program) / sizeof(program[0]);*/
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program.bin>\n", argv[0]);
        exit(1);
    }
    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        fprintf(stderr, "Error: Could not open file %s\n", argv[1]);
        exit(1);
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fileSize > STACK_SIZE) {
        fprintf(stderr, "Error: File size exceeds maximum allowed size (%d bytes)\n", STACK_SIZE);
        fclose(file);
        exit(1);
    }

    uint8_t program[fileSize];
    if (fread(program, 1, fileSize, file) != fileSize) {
        fprintf(stderr, "Error: Could not read file %s\n", argv[1]);
        fclose(file);
        exit(1);
    }
    fclose(file);

    size_t progSize = fileSize;
    execute(program, progSize);
    return 0;
}