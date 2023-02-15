#pragma once

#define OP_HALT     0x00
#define OP_CONST    0x01
#define OP_ADD      0x02
#define OP_SUB      0x03
#define OP_MUL      0x04
#define OP_DIV      0x05

typedef struct VM VM;
typedef struct Value Value;
typedef enum ValueType ValueType;
typedef enum ObjectType ObjectType;
typedef struct Object Object;
typedef struct StringObject StringObject;
typedef struct CodeObject CodeObject;

Value VM_Eval(VM* vm, CodeObject* codeObject);
uint8_t VM_Read_Byte(VM* vm);
Value VM_Stack_Pop(VM* vm);
void VM_Stack_Push(VM* vm, Value* value);

enum ValueType{
    ValuteType_Number,
    ValueType_Object
};

enum ObjectType{
    ObjectType_String,
    ObjectType_Code
};

struct Value{
    ValueType type;
    union
    {
        float64 number;
        Object* object;
    };
};

struct Object{
    ObjectType objectType;
};

struct StringObject
{
    Object object;
    char* string;
    size_t length;
};

struct CodeObject
{
    Object object;
    char* name;
    size_t name_length;
    uint8_t* code; // Bytearray of opcodes
    size_t code_last;
    Value* constants;
    size_t constants_last;
};

#define NUMBER(value) (Value){.type = ValuteType_Number, .number = value}

#define AS_STRING(value) (*(StringObject*)value.object)
#define AS_CODE(value) (*(CodeObject*)value.object)
#define AS_NUMBER(value) (*(float64*)value.number)

Value ALLOC_STRING(char* value, size_t length){
    Value result;
    result.type = ValueType_Object;

    StringObject* stringObject = malloc(sizeof(StringObject));
    stringObject->object.objectType = ObjectType_String;
    stringObject->string = value;
    stringObject->length = length;
    result.object = (Object*)stringObject;
    
    return result;
}

Value ALLOC_CODE(char* name, size_t length){
    Value result;
    result.type = ValueType_Object;

    CodeObject* co = malloc(sizeof(CodeObject));
    co->object.objectType = ObjectType_Code;
    co->name = name;
    co->name_length = length;
    co->code_last = 0;
    co->constants_last = 0;
    result.object = (Object*)co;
    
    return result;
}

#define STACK_LIMIT 512

struct VM 
{
    uint8_t* ip; // Instruction pointer

    Value* stack; // Array of values
    Value* stack_start; // First address of stack
    Value* stack_end; // Last address of stack
    Value* sp; // Stack pointer
};

Value VM_exec(VM* vm, CodeObject* codeObject){
    
    // Value asd = NUMBER(123);


    vm->stack = malloc(sizeof(Value) * STACK_LIMIT);
    vm->stack_start = vm->stack;
    vm->stack_end = vm->stack_start + sizeof(Value) * STACK_LIMIT;



    //codeObject->constants[0] = ALLOC_STRING("Hello", 5);
    // vm->constants[1] = NUMBER(3);

    // codeObject->code[0] = OP_CONST;
    // codeObject->code[1] = 0;
    // vm->code[2] = OP_CONST;
    // vm->code[3] = 1;
    // vm->code[4] = OP_ADD; 
    // codeObject->code[2] = OP_HALT;

    // Set instruction pointer to the beginning
    vm->ip = &codeObject->code[0];
    vm->sp = &vm->stack[0];

    return VM_Eval(vm, codeObject);
}

#define BINARY_OP(operation)                    \
    do {                                        \
        float64 op2 = VM_Stack_Pop(vm).number;  \
        float64 op1 = VM_Stack_Pop(vm).number;  \
        float64 result = op1 operation op2;     \
        VM_Stack_Push(vm, &NUMBER(result));     \
    } while (false)                             \

Value VM_Eval(VM* vm, CodeObject* codeObject){
    while(true){
        uint8_t opcode = VM_Read_Byte(vm);
        printf("OPCODE: 0x%08X\n", opcode);
        switch (opcode)
        {
            case OP_HALT:{
                return VM_Stack_Pop(vm);
            }       
            case OP_CONST: {
                uint8_t constIndex = VM_Read_Byte(vm);
                Value constant = codeObject->constants[constIndex];
                VM_Stack_Push(vm, &constant);
                break;
            }
            case OP_ADD: BINARY_OP(+); break;
            case OP_SUB: BINARY_OP(-); break;
            case OP_MUL: BINARY_OP(*); break;
            case OP_DIV: BINARY_OP(/); break;
            
            default: {
                printf("VM Error. Unrecognized opcode: %#x", opcode);
                exit(0);
            }

        }
    }
}

uint8_t VM_Read_Byte(VM* vm){
    return *vm->ip++;
}

void VM_Stack_Push(VM* vm, Value* value){
    if(vm->sp == vm->stack_end){
        printf("VM Error. Stack Overflow\n");
        exit(0); 
    }

    *vm->sp = *value;
    vm->sp++;
}

Value VM_Stack_Pop(VM* vm){
    if(vm->sp == vm->stack_start){
        printf("VM Error. Empty stack\n");
        exit(0); 
    }
    vm->sp--;
    return *vm->sp;
}

