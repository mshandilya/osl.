#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DLOG(x...) //printf(x);

typedef enum {
    VAL_CHAR,
    VAL_BOOL,
    VAL_INT,
    VAL_FLOAT,
    VAL_ARR,
    VAL_FUN,
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
    PUSH_NONE   = 0x07,

    // Stack manipulation
    POP             = 0x10,
    DUP             = 0x11,
    SWAP            = 0x12,
    OVER            = 0x13,

    // Identifier manipulation
    GET_FROM        = 0x15,
    SET_TO          = 0x16,

    // Arithmetic operations
    ADD             = 0x20,
    SUB             = 0x21,
    MUL             = 0x22,
    DIV             = 0x23,
    MOD             = 0x24,
    NEG             = 0x25,

    // Bitwise operations
    BITWISE_NOT     = 0x30,
    BITWISE_AND     = 0x31,
    BITWISE_OR      = 0x32,
    BITWISE_XOR     = 0x33,

    // Logical operations
    AND             = 0x34,
    OR              = 0x35,
    NOT             = 0x36,

    // Comparisons
    EQ              = 0x40,
    NEQ             = 0x41,
    LT              = 0x42,
    GT              = 0x43,
    LE              = 0x44,
    GE              = 0x45,

    // Control flow
    JUMP            = 0x50,
    JUMP_IF_ZERO    = 0x51,
    JUMP_IF_NONZERO = 0x52,
    CALL            = 0x53,
    RETURN          = 0x54,
    HALT            = 0x55,

    // Type conversions
    I2F             = 0x60,
    F2I             = 0x61,
    I2D             = 0x62,
    D2I             = 0x63,
    F2D             = 0x64,
    D2F             = 0x65,

    // Heap Object Operations
    NEW_OBJECT      = 0x70,
    GET_FIELD       = 0x71,
    SET_FIELD       = 0x72,

    // I/O Operations
    LOG             = 0x90,

    // Function Operations
    MAKE_FUNC       = 0x91,
    MAKE_CLOSURE    = 0x92,

    // Array Operations
    MAKE_ARRAY      = 0x93,
    ARR_ACC         = 0x94,
    STORE           = 0x95,
    LOAD            = 0x96,
    MAKE_DECL_ARRAY = 0x97,
    BIND            = 0x98,
    
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

unsigned int callScope = 0, sNodeTail = 0, currentRetAddress = 0;

typedef struct llNode llNode;

typedef struct closedObject {
    int64_t id;
    ValueType type;
    void* location;
} closedObject;

typedef struct closureObject {
    int64_t size;
    closedObject* objs;
} closureObject;

typedef struct functionObj {
    int64_t entry, exit;
    closureObject* co;
} functionObj;

// To-do:
// CALL:
//   * pop function object
//   * enhance scope
//   * look for closure object (push as isClosed, push only if pointers are different)
//   * update pc
//   * push return
// MKFUN:
//   * entry and exit point from stack
//   * pushed the object to stack
// MKCLOSURE:
//   * expecting number of closed objects
//   * expecting the ids
//   * expecting the fun object
// RETURN:
//   * free memory for non-closed objects
//   * do not for closed objects
// scope

struct llNode {
    ValueType type;
    unsigned int scopeId, isClosed;
    void* location;
    llNode *prev, *next;
};

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
    for(unsigned int i = 0; i<(1<<15); i++) {
        vals.heads[i] = vals.tails[i] = NULL;
    }
}

#define STACK_SIZE 1<<16

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
    initialize();

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

            case PUSH_NONE: {
                Value v; v.type = VAL_INT;
                v.v = malloc(sizeof(int64_t));
                *(int64_t*)v.v = 0;
                PUSH(v);
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
                DLOG("pushing an int\n")
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

            // Identifier manipulation
            case GET_FROM: {
                if (pc + 8 >= codeSize) { fprintf(stderr, "Unexpected end (GET_FROM)\n"); exit(1); }
                int64_t id = 0;
                for (int j = 0; j < 8; j++) {
                    id |= ((int64_t)code[pc+1+j]) << (8*j);
                }

                // printf("get id: %lld\n",id);
                
                Value val; val.type = vals.tails[id]->type;
                val.v = ((Value*)(vals.tails[id]->location))->v;

                // if(val.type == VAL_INT){
                //     printf("get val: %lld\n",*(int64_t*)val.v);
                // }else if(val.type == VAL_FLOAT){
                //     printf("get val: %f\n",*(float*)val.v);
                // }else if(val.type == VAL_CHAR){
                //     printf("get val: %c\n",*(char*)val.v);
                // }
                
                PUSH(val);
                pc += 9;
                break;
            }
            
            case BIND: {
                if (pc + 8 >= codeSize) { fprintf(stderr, "Unexpected end (BIND)\n"); exit(1); }
                Value* v = (Value*)malloc(sizeof(Value)); 
                v->type = VAL_ID;
                int64_t id = 0;
                for (int j = 0; j < 8; j++) {
                    id |= ((int64_t)code[pc+1+j]) << (8*j);
                }
                // printf("bind id: %lld\n",id);
                Value vv = POP();
                DLOG("bind id: %u\n",id)

                ValueType t = vv.type;
                v->v = vv.v;

                if(vals.tails[id] == NULL) {
                    vals.heads[id] = (llNode*)malloc(sizeof(llNode));
                    vals.tails[id] = vals.heads[id];
                    vals.tails[id]->next = vals.tails[id]->prev = NULL;
                    vals.tails[id]->type = t;
                    vals.tails[id]->location = v;
                    vals.tails[id]->scopeId = callScope;
                    vals.tails[id]->isClosed = 0;
                    sNodeStack[sNodeTail++] = (sNode){id, callScope};
                }
                else if(vals.tails[id]->scopeId == callScope) {
                    fprintf(stderr, "BIND may only be called once on each ID at each call scope at PC = %lld\n", pc); 
                    exit(1); 
                }
                else {
                    vals.tails[id]->next = (llNode*)malloc(sizeof(llNode));
                    vals.tails[id]->next->prev = vals.tails[id];
                    vals.tails[id] = vals.tails[id]->next;
                    vals.tails[id]->next = NULL;
                    vals.tails[id]->type = t;
                    vals.tails[id]->location = v;
                    vals.tails[id]->scopeId = callScope;
                    vals.tails[id]->isClosed = 0;
                    sNodeStack[sNodeTail++] = (sNode){id, callScope};
                }
                pc += 9;
                break;
            }

            case SET_TO: {
                if (pc + 8 >= codeSize) { fprintf(stderr, "Unexpected end (SET_TO)\n"); exit(1); }
                int64_t id = 0;
                for (int j = 0; j < 8; j++) {
                    id |= ((int64_t)code[pc+1+j]) << (8*j);
                }
                //printf("set id: %lld\n",id);
                Value vv = POP();
                DLOG("set id: %u\n",id)

                ValueType t = vv.type;
                void* val = vv.v;

                if(vals.tails[id] == NULL) {
                    fprintf(stderr, "ID not available to set to...\n"); 
                    exit(1); 
                }
                else {
                    vals.tails[id]->type = t;
                    ((Value*)(vals.tails[id]->location))->v = val;
                }
                pc += 9;
                break;
            }

            case MAKE_FUNC: {
                Value exitAddr = POP();
                Value entryAddr = POP();
                functionObj* f = (functionObj*)malloc(sizeof(functionObj));
                if(entryAddr.type == VAL_INT && exitAddr.type == VAL_INT) {
                    f->entry = *(int64_t*)(entryAddr.v);
                    f->exit = *(int64_t*)(exitAddr.v);
                }
                else {
                    fprintf(stderr, "Entry and Exit points must be integers\n"); exit(1);
                }
                Value func;
                func.type = VAL_FUN;
                func.v = f;
                PUSH(func);
                pc += 1;
                break;
            }

            case MAKE_CLOSURE: {
                DLOG("make closure entered\n")
                Value numIds = POP();
                closureObject* co = (closureObject*)malloc(sizeof(closureObject));
                if(numIds.type == VAL_INT) {
                    co->size = *(int64_t*)numIds.v; // fixed this
                    DLOG("make closure size: %lld\n",co->size)
                    co->objs = (closedObject*)malloc(co->size * sizeof(closedObject));
                    for(int j = 0; j<co->size; j++) {
                        Value id = POP();
                        if(id.type != VAL_INT) {
                            fprintf(stderr, "Closed ID must be integer\n"); exit(1);
                        }
                        closedObject cid;
                        cid.id = *(int32_t*)(id.v);
                        cid.location = vals.tails[cid.id]->location;
                        cid.type = vals.tails[cid.id]->type;
                        vals.tails[cid.id]->isClosed = 1;
                        co->objs[j] = cid;
                    }
                }
                else {
                    fprintf(stderr, "Closure object count must be integer\n"); exit(1);
                }
                Value f = POP();
                DLOG("Popping function object\n")
                if(f.type == VAL_FUN) {
                    ((functionObj*)(f.v))->co = co;
                }
                else {
                    fprintf(stderr, "Closure object must be tied to a function object on the stack\n"); exit(1);
                }
                pc += 1;
                DLOG("make closure exited\n")
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
                    *(float*)result.v = 0;
                    if(a.type == VAL_INT){
                        *(float*)result.v += (float)*(int32_t*)a.v;
                    }else{
                        *(float*)result.v += *(float*)a.v;
                    }
                    if(b.type == VAL_INT){
                        *(float*)result.v += (float)*(int32_t*)b.v;
                    }else{
                        *(float*)result.v += *(float*)b.v;
                    }
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
                    *(float*)result.v = 0;
                    if(a.type == VAL_INT){
                        *(float*)result.v += (float)*(int32_t*)a.v;
                    }else{
                        *(float*)result.v += *(float*)a.v;
                    }
                    if(b.type == VAL_INT){
                        *(float*)result.v -= (float)*(int32_t*)b.v;
                    }else{
                        *(float*)result.v -= *(float*)b.v;
                    }
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
                    *(float*)result.v = 0;
                    if(a.type == VAL_INT){
                        *(float*)result.v += (float)*(int32_t*)a.v;
                    }else{
                        *(float*)result.v += *(float*)a.v;
                    }
                    if(b.type == VAL_INT){
                        *(float*)result.v *= (float)*(int32_t*)b.v;
                    }else{
                        *(float*)result.v *= *(float*)b.v;
                    }
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
                    *(int64_t*)result.v = 0;
                    if(a.type == VAL_INT){
                        *(int64_t*)result.v = *(int64_t*)a.v;
                    }else{
                        *(int64_t*)result.v = (int64_t)*(float*)a.v;
                    }
                    if(b.type == VAL_INT){
                        *(int64_t*)result.v /= *(int64_t*)b.v;
                    }else{
                        *(int64_t*)result.v /= (int64_t)*(float*)b.v;
                    }
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
            
            // Logical Operations
            case AND: {
                Value b = POP();
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.v = malloc(sizeof(int64_t));
                *(int64_t*)result.v = (*(int64_t*)a.v && *(int64_t*)b.v) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }
            case OR: {
                Value b = POP();
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.v = malloc(sizeof(int64_t));
                *(int64_t*)result.v = (*(int64_t*)a.v || *(int64_t*)b.v) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }
            case NOT: {
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.v = malloc(sizeof(int64_t));
                *(int64_t*)result.v = (*(int64_t*)a.v == 0) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }

            // Comparison Operations
            case EQ: {
                Value b = POP();
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.v = malloc(sizeof(int64_t));
                if (a.type == VAL_INT && b.type == VAL_INT) {
                    *(int64_t*)result.v = (*(int64_t*)a.v == *(int64_t*)b.v) ? 1 : 0;
                } else if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
                    *(int64_t*)result.v = (*(float*)a.v == *(float*)b.v) ? 1 : 0;
                } else if (a.type == VAL_CHAR && b.type == VAL_CHAR) {
                    *(int64_t*)result.v = (*(char*)a.v == *(char*)b.v) ? 1 : 0;
                } else { fprintf(stderr, "EQ supports only INT, FLOAT, and CHAR values.\n"); exit(1); }
                //result.i = (a.type == VAL_INT && b.type == VAL_INT && a.i == b.i) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }
            case NEQ: {
                Value b = POP();
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.v = malloc(sizeof(int64_t));
                if (a.type == VAL_INT && b.type == VAL_INT) {
                    *(int64_t*)result.v = (*(int64_t*)a.v != *(int64_t*)b.v) ? 1 : 0;
                } else if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
                    *(int64_t*)result.v = (*(float*)a.v != *(float*)b.v) ? 1 : 0;
                } else if (a.type == VAL_CHAR && b.type == VAL_CHAR) {
                    *(int64_t*)result.v = (*(char*)a.v != *(char*)b.v) ? 1 : 0;
                } else { fprintf(stderr, "NEQ supports only INT, FLOAT, and CHAR values.\n"); exit(1); }
                //result.i = (a.type == VAL_INT && b.type == VAL_INT && a.i != b.i) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }
            case LT: {
                Value b = POP();
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.v = malloc(sizeof(int64_t));
                if (a.type == VAL_INT && b.type == VAL_INT) {
                    *(int64_t*)result.v = (*(int64_t*)a.v < *(int64_t*)b.v) ? 1 : 0;
                } else if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
                    *(int64_t*)result.v = (*(float*)a.v < *(float*)b.v) ? 1 : 0;
                } else { fprintf(stderr, "LT supports only INT and FLOAT values.\n"); exit(1); }
                //result.i = (a.type == VAL_INT && b.type == VAL_INT && a.i < b.i) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }
            case GT: {
                Value b = POP();
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.v = malloc(sizeof(int64_t));
                if (a.type == VAL_INT && b.type == VAL_INT) {
                    *(int64_t*)result.v = (*(int64_t*)a.v > *(int64_t*)b.v) ? 1 : 0;
                } else if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
                    *(int64_t*)result.v = (*(float*)a.v > *(float*)b.v) ? 1 : 0;
                } else { fprintf(stderr, "GT supports only INT and FLOAT values.\n"); exit(1); }
                //result.i = (a.type == VAL_INT && b.type == VAL_INT && a.i > b.i) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }
            case LE: {
                Value b = POP();
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.v = malloc(sizeof(int64_t));
                if (a.type == VAL_INT && b.type == VAL_INT) {
                    *(int64_t*)result.v = (*(int64_t*)a.v <= *(int64_t*)b.v) ? 1 : 0;
                } else if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
                    *(int64_t*)result.v = (*(float*)a.v <= *(float*)b.v) ? 1 : 0;
                } else { fprintf(stderr, "LE supports only INT and FLOAT values.\n"); exit(1); }
                //result.i = (a.type == VAL_INT && b.type == VAL_INT && a.i <= b.i) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }
            case GE: {
                Value b = POP();
                Value a = POP();
                Value result; result.type = VAL_INT;
                result.v = malloc(sizeof(int64_t));
                if (a.type == VAL_INT && b.type == VAL_INT) {
                    *(int64_t*)result.v = (*(int64_t*)a.v >= *(int64_t*)b.v) ? 1 : 0;
                } else if (a.type == VAL_FLOAT && b.type == VAL_FLOAT) {
                    *(int64_t*)result.v = (*(float*)a.v >= *(float*)b.v) ? 1 : 0;
                } else { fprintf(stderr, "GE supports only INT and FLOAT values.\n"); exit(1); }
                //result.i = (a.type == VAL_INT && b.type == VAL_INT && a.i >= b.i) ? 1 : 0;
                PUSH(result);
                pc += 1;
                break;
            }

            // Control Flow
            case JUMP: {
                if (pc + 4 >= codeSize) { fprintf(stderr, "Unexpected end in JUMP\n"); exit(1); }
                int32_t offset = 0;
                for (int j = 0; j < 4; j++) {
                    offset |= ((int32_t)code[pc+1+j]) << (8*j);
                }
                pc += 5;
                pc += offset;
                DLOG("jump to %zu\n", pc)
                break;
            }
            case JUMP_IF_ZERO: {
                if (pc + 4 >= codeSize) { fprintf(stderr, "Unexpected end in JUMP_IF_ZERO\n"); exit(1); }
                int32_t offset = 0;
                for (int j = 0; j < 4; j++) {
                    offset |= ((int32_t)code[pc+1+j]) << (8*j);
                }
                Value cond = POP();
                void* cv = cond.v;
                DLOG("condition popped: %lld\n", *(int64_t*)cv)
                pc += 5;
                if (cond.type == VAL_INT && *(int64_t*)cond.v == 0) {
                    pc += offset;
                }
                break;
            }
            case JUMP_IF_NONZERO: {
                if (pc + 2 >= codeSize) { fprintf(stderr, "Unexpected end in JUMP_IF_NONZERO\n"); exit(1); }
                int16_t offset = code[pc+1] | (code[pc+2] << 8);
                Value cond = POP();
                pc += 3;
                if (cond.type == VAL_INT && *(int64_t*)cond.v != 0) {
                    pc += offset;
                }
                break;
            }
            case CALL: {
                DLOG("call entered\n")
                Value f = POP();

                if(f.type == VAL_FUN) {
                    int64_t addr = ((functionObj*)(f.v))->entry;
                    // Value ret; ret.type = VAL_INT;
                    // ret.v = malloc(sizeof(int64_t));
                    // *(int64_t*)ret.v = pc + 1;
                    // DLOG("return address: %zu\n", *(int64_t*)ret.v)
                    // PUSH(ret);
                    callScope++;
                    closureObject* co = ((functionObj*)(f.v))->co;
                    for(int j = 0; j < co->size; j++) {
                        if(vals.tails[co->objs[j].id] == NULL) {
                            vals.heads[co->objs[j].id] = (llNode*)malloc(sizeof(llNode));
                            vals.tails[co->objs[j].id] = vals.heads[co->objs[j].id];
                            vals.tails[co->objs[j].id]->next = vals.tails[co->objs[j].id]->prev = NULL;
                            vals.tails[co->objs[j].id]->type = co->objs[j].type;
                            vals.tails[co->objs[j].id]->location = co->objs[j].location;
                            vals.tails[co->objs[j].id]->scopeId = callScope;
                            vals.tails[co->objs[j].id]->isClosed = 1;
                            sNodeStack[sNodeTail++] = (sNode){co->objs[j].id, callScope};
                        }
                        else if(vals.tails[co->objs[j].id]->location != co->objs[j].location) {
                            vals.tails[co->objs[j].id]->next = (llNode*)malloc(sizeof(llNode));
                            vals.tails[co->objs[j].id]->next->prev = vals.tails[co->objs[j].id];
                            vals.tails[co->objs[j].id] = vals.tails[co->objs[j].id]->next;
                            vals.tails[co->objs[j].id]->next = NULL;
                            vals.tails[co->objs[j].id]->type = co->objs[j].type;
                            vals.tails[co->objs[j].id]->location = co->objs[j].location;
                            vals.tails[co->objs[j].id]->scopeId = callScope;
                            vals.tails[co->objs[j].id]->isClosed = 1;
                            sNodeStack[sNodeTail++] = (sNode){co->objs[j].id, callScope};
                        }
                        // else no need to push the id (already there)
                    }
                    callScope++;
                    pc = addr;
                    // printf("Called Successfully\n");
                }
                else {
                    fprintf(stderr, "Only function objects can be called\n"); exit(1);
                }
                DLOG("call exited\n")
                break;
            }
            case RETURN: {
                if (top < 1) { 
                    fprintf(stderr, "Stack underflow on RETURN\n"); 
                    exit(1); 
                }

                Value returnAddress;
                Value returnValue;

                if (top == 1) {
                    returnAddress = POP();
                } else {
                    returnValue = POP();
                    returnAddress = POP();
                    PUSH(returnValue);
                }

                if (returnAddress.type != VAL_INT) { 
                    fprintf(stderr, "Invalid return address type\n"); 
                    exit(1); 
                }

                while (sNodeTail > 0 && sNodeStack[sNodeTail - 1].scopeId == callScope) {
                    unsigned int id = sNodeStack[--sNodeTail].id;
                    if (vals.tails[id] != NULL) {
                        llNode* temp = vals.tails[id];
                        vals.tails[id] = vals.tails[id]->prev;
                        if (vals.tails[id] == NULL) {
                            vals.heads[id] = NULL;
                        } else {
                            vals.tails[id]->next = NULL;
                        }
                        if (temp->isClosed) {
                            free(temp);
                        } else {
                            free(temp->location);
                            free(temp);
                        }
                    }
                }

                callScope--;

                while (sNodeTail > 0 && sNodeStack[sNodeTail - 1].scopeId == callScope) {
                    unsigned int id = sNodeStack[--sNodeTail].id;
                    if (vals.tails[id] != NULL) {
                        llNode* temp = vals.tails[id];
                        vals.tails[id] = vals.tails[id]->prev;
                        if (vals.tails[id] == NULL) {
                            vals.heads[id] = NULL;
                        } else {
                            vals.tails[id]->next = NULL;
                        }
                        if (temp->isClosed) {
                            free(temp);
                        } else {
                            free(temp->location);
                            free(temp);
                        }
                    }
                }

                callScope--;

                pc = *(int64_t*)returnAddress.v;
                DLOG("return to %zu\n", pc)
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

            // Array Operations
            case MAKE_ARRAY: {
                DLOG("reached at make array\n")
                if (pc + 2 >= codeSize) { fprintf(stderr, "Unexpected end in MAKE_ARRAY\n"); exit(1);}
                uint16_t nElems = code[pc+1] | (code[pc+2]<<8);
                Value v;
                DLOG("%d\n", nElems)
                v.type = VAL_ARR;
                v.v = malloc(nElems * sizeof(Value)); // fixed! sizeof(Value) not sizeof(Value*)
                DLOG("atleast here\n")
                for (int i = 0; i < nElems; i++) {
                    ((Value*)(v.v))[i] = POP();
                    DLOG("pushed an element to array\n")
                }
                DLOG("pushed everything to array object\n")
                PUSH(v);
                if (v.type == VAL_ARR) { DLOG("pushed the array object to stack\n")}
                pc += 3;
                break;
            }
            case ARR_ACC: {
                DLOG("reached at array access\n")
                Value arr = POP(), ind = POP();
                if(arr.type != VAL_ARR) { fprintf(stderr, "Unexpected type for array"); exit(1);}
                if(ind.type != VAL_INT) { fprintf(stderr, "Unexpected type for index"); exit(1);}
                int64_t idx = *((int64_t*)ind.v);
                PUSH(((Value*)(arr.v))[idx]);
                pc += 1;
                break;
            }
            case MAKE_DECL_ARRAY: {
                DLOG("reached at make decl array\n")
                if (pc + 2 >= codeSize) { fprintf(stderr, "Unexpected end in MAKE_DECL_ARRAY\n"); exit(1);}
                uint16_t nDims = code[pc+1] | (code[pc+2]<<8);
                Value v;
                v.type = VAL_ARR;
                
                // for(int i = 0; i < nDims; i++) {
                //     Value nd = POP();
                //     if(nd.type != VAL_INT) { fprintf(stderr, "Unexpected type for index"); exit(1);}
                //     v.v = malloc((*(int64_t*)(nd.v)) *sizeof(Value));
                //     for(int j = 0; j<(*(int64_t*)(nd.v)); j++) {
                //         ((Value*)v.v)[j].type = VAL_ARR;
                //     }
                // }
                
                int64_t sizes[nDims];
                for (int d = 0; d < nDims; d++) {
                    Value dim = POP();
                    if (dim.type != VAL_INT) {
                        fprintf(stderr, "Unexpected type for dimension size\n");
                        exit(1);
                    }
                    sizes[d] = *(int64_t*)(dim.v);
                }

                Value vv; // innermost array
                vv.type = VAL_ARR;
                vv.v = malloc(sizes[0] * sizeof(Value));
                for (int i = 0; i < sizes[0]; i++){
                    ((Value*)vv.v)[i].type = VAL_INT;
                    ((Value*)vv.v)[i].v = malloc(sizeof(int64_t));
                    *(int64_t*)((Value*)vv.v)[i].v = 0;
                }

                for (int d = 1; d < nDims; d++) {
                    Value vvv;
                    vvv.type = VAL_ARR;
                    vvv.v = malloc(sizes[d] * sizeof(Value));
                    for (int i = 0; i < sizes[d]; i++) {
                        ((Value*)vvv.v)[i].type = VAL_ARR;
                        ((Value*)vvv.v)[i].v = malloc(sizes[d - 1] * sizeof(Value));
                        memcpy(((Value*)vvv.v)[i].v, vv.v, sizes[d - 1] * sizeof(Value));
                    }
                    vv = vvv;
                }
                // innermost array now becomes the outermost array
                PUSH(vv);
                pc += 3;
                break;
            }
            case LOAD: {
                DLOG("reached at load\n")
                Value loc = POP();
                PUSH(*(Value*)loc.v);
                pc++;
                break;
            }
            case STORE: {
                DLOG("reached at store\n")
                Value loc = POP(), val = POP();
                switch(val.type) {
                    case VAL_INT:
                        *(int64_t*)(loc.v) = *(int64_t*)(val.v);
                        break;
                    case VAL_FLOAT:
                        *(float*)(loc.v) = *(float*)(val.v);
                        break;
                    case VAL_CHAR:
                        *(char*)(loc.v) = *(char*)(val.v);
                        break;
                    case VAL_BOOL:
                        *(int64_t*)(loc.v) = *(int64_t*)(val.v);
                        break;
                    case VAL_ARR:
                        *(Value*)(loc.v) = *(Value*)(val.v);
                        break;
                }
                pc += 1;
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
            // case VAL_INT:    printf("Result: %d\n", result.i); break;
            // case VAL_CHAR:   printf("Result: %c\n", result.c); break;
            // //case VAL_SHORT:  printf("Result: %d\n", result.s); break;
            // //case VAL_LONG:   printf("Result: %lld\n", (long long)result.l); break;
            // case VAL_FLOAT:  printf("Result: %f\n", result.f); break;
            // //case VAL_DOUBLE: printf("Result: %lf\n", result.d); break;
            // case VAL_OBJ:    printf("Result: [object at %p]\n", (void*)result.obj); break;
            // default:         printf("Unknown result type\n"); break;
            case VAL_INT: {
                printf("Result: %lld\n", *(int64_t*)result.v);
                free(result.v);
                break;
            }
            case VAL_FLOAT: {
                printf("Result: %f\n", *(float*)result.v);
                free(result.v);
                break;
            }
            case VAL_CHAR: {
                printf("Result: %c\n", *(char*)result.v);
                free(result.v);
                break;
            }
            case VAL_OBJ: {
                printf("Result: [object at %p]\n", (void*)result.obj);
                free(result.v);
                break;
            }
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