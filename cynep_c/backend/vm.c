#pragma once

#define OP_HALT             0x00
#define OP_CONST            0x01
#define OP_ADD              0x02
#define OP_SUB              0x03
#define OP_MUL              0x04
#define OP_DIV              0x05
#define OP_CMP              0x06
#define OP_JMP_IF_FALSE     0x07
#define OP_JMP              0x08

// Comparisons
#define OP_CMP_GREATER          0x01
#define OP_CMP_LOWER            0x02
#define OP_CMP_EQUALS           0x03
#define OP_CMP_GREATER_EQUALS   0x04
#define OP_CMP_LOWER_EQUALS     0x05
#define OP_CMP_NOT_EQUALS       0x06

typedef struct VM VM;
typedef struct RuntimeValue RuntimeValue;
typedef enum ValueType ValueType;
typedef enum ObjectType ObjectType;
typedef struct Object Object;
typedef struct StringObject StringObject;
typedef struct CodeObject CodeObject;

RuntimeValue VM_Eval(VM* vm, CodeObject* codeObject);
uint8_t VM_Read_Byte(VM* vm);
RuntimeValue VM_Stack_Pop(VM* vm);
void VM_Stack_Push(VM* vm, RuntimeValue* value);
uint16_t VM_Read_Address(VM* vm);
uint8_t VM_Peek_Byte(VM* vm);

enum ValueType{
    ValuteType_Number,
    ValueType_Boolean,
    ValueType_Object
};

enum ObjectType{
    ObjectType_String,
    ObjectType_Code
};

struct RuntimeValue{
    ValueType type;
    union
    {
        float64 number;
        bool boolean;
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
    RuntimeValue* constants;
    uint16_t constants_last;
};



#define NUMBER(value) (RuntimeValue){.type = ValuteType_Number, .number = value}
#define BOOLEAN(value) (RuntimeValue){.type = ValueType_Boolean, .boolean = value}

#define AS_STRING(value) (*(StringObject*)value.object)
#define AS_CODE(value) (*(CodeObject*)value.object)
#define AS_NUMBER(value) (*(float64*)value.number)
#define AS_BOOLEAN(value) (*(bool*)value.boolean)

#define DEBUG_BUFFSIZE 100
char* RuntimeValueToString(RuntimeValue value){
    
    if(value.type == ValuteType_Number){
        char* buf = malloc(sizeof(char)*DEBUG_BUFFSIZE);
        gcvt(value.number, 6, buf);
        return buf;
    }
    if(value.type == ValueType_Boolean){
        if(value.boolean == true)
            return "true";
        else
            return "false";
    }
    if(value.type == ValueType_Object && value.object->objectType == ObjectType_Code){
        CodeObject code = AS_CODE(value);
        char* buf = malloc(sizeof(char)*DEBUG_BUFFSIZE);
        strncpy(buf, code.name, code.name_length);
        return buf;
    }

    return "not implemented";
}

RuntimeValue ALLOC_STRING(char* value, size_t length){
    RuntimeValue result;
    result.type = ValueType_Object;

    StringObject* stringObject = malloc(sizeof(StringObject));
    stringObject->object.objectType = ObjectType_String;
    stringObject->string = value;
    stringObject->length = length;
    result.object = (Object*)stringObject;
    
    return result;
}

RuntimeValue ALLOC_CODE(char* name, size_t length){
    RuntimeValue result;
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


//
//  VM Implementation
//

#define STACK_LIMIT 1000000000

struct VM 
{
    uint8_t* ip; // Instruction pointer

    RuntimeValue* stack; // Array of values
    RuntimeValue* stack_start; // First address of stack
    RuntimeValue* stack_end; // Last address of stack
    RuntimeValue* sp; // Stack pointer
};

RuntimeValue VM_exec(VM* vm, CodeObject* codeObject){

    vm->stack = malloc(sizeof(RuntimeValue) * STACK_LIMIT);
    vm->stack_start = vm->stack;
    vm->stack_end = vm->stack_start + sizeof(RuntimeValue) * STACK_LIMIT;

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

RuntimeValue VM_Eval(VM* vm, CodeObject* co){
    while(true){
        uint8_t opcode = VM_Read_Byte(vm);
        // printf("OPCODE: 0x%02X\n", opcode);
        switch (opcode)
        {
            case OP_HALT:{
                return VM_Stack_Pop(vm);
            }     

            case OP_CONST: {
                //uint8_t constIndex = VM_Read_Byte(vm); 

                // printf("%i\n",constIndex2);
                uint16_t constIndex = VM_Read_Address(vm); // Behöver vi köra 64 bit addresser?
                RuntimeValue constant = co->constants[constIndex];
                VM_Stack_Push(vm, &constant);
                break;
            }

            case OP_ADD: BINARY_OP(+); break;
            case OP_SUB: BINARY_OP(-); break;
            case OP_MUL: BINARY_OP(*); break;
            case OP_DIV: BINARY_OP(/); break;
            
            case OP_CMP:{
                uint8_t cmp_type = VM_Read_Byte(vm);
                RuntimeValue op2 = VM_Stack_Pop(vm);
                RuntimeValue op1 = VM_Stack_Pop(vm);

                if(op2.type == ValuteType_Number && op1.type == ValuteType_Number){

                    bool res;
                    switch (cmp_type)
                    {
                    case OP_CMP_GREATER:
                        res = op1.number > op2.number;
                        // printf("%f\n", op1.number);
                        //                         printf("%f\n", op2.number);
                        break;
                    case OP_CMP_LOWER:
                        res = op1.number < op2.number;
                        break;
                    case OP_CMP_EQUALS:
                        res = op1.number == op2.number;
                        break;
                    case OP_CMP_GREATER_EQUALS:
                        res = op1.number >= op2.number;
                        break;
                    case OP_CMP_LOWER_EQUALS:
                        res = op1.number <= op2.number;
                        break;
                    case OP_CMP_NOT_EQUALS:
                        res = op1.number != op2.number;
                        break;
                    default:
                        printf("VM Error. Illegal comparison");
                        exit(0);
                    }
                    VM_Stack_Push(vm, &BOOLEAN(res));
                }
                else{
                    // TODO: String comparisons etc.
                }
                break;
            }
            case OP_JMP_IF_FALSE:{
                bool condition = VM_Stack_Pop(vm).boolean;
                uint16_t address = VM_Read_Address(vm);

                if(!condition){
                    vm->ip = &co->code[address];
                }
                break;
            }
            case OP_JMP:{
                uint16_t address = VM_Read_Address(vm);
                printf("%i\n", address);


                vm->ip = &co->code[address];

                uint8_t test = VM_Peek_Byte(vm);
                if(test == 17){
                    printf("APAASD");
                }

                break;
            }

            default: {
                if(opcode == 17){
                    int asd = 0;
                }
                // printf("VM Error. Unrecognized opcode: %#x", opcode);
                printf("\033[0;31mVM Error. Unrecognized opcode: %#x \033[0m\n", opcode);
                exit(0);
            }

        }
    }
}

uint16_t VM_Read_Address(VM* vm){ // Reads 16 bit address
    vm->ip += 2; // 24 32 40 48 56 64
    return ((vm->ip[-2] << 8) | (vm->ip[-1]));
}

uint8_t VM_Read_Byte(VM* vm){
    return *vm->ip++;
}

uint8_t VM_Peek_Byte(VM* vm){
    return *vm->ip;
}

void VM_Stack_Push(VM* vm, RuntimeValue* value){
    if(vm->sp == vm->stack_end){
        printf("VM Error. Stack Overflow\n");
        exit(0); 
    }
    RuntimeValue asd = *value;
    *vm->sp = *value;
    vm->sp++;
}

RuntimeValue VM_Stack_Pop(VM* vm){
    if(vm->sp == vm->stack_start){
        printf("VM Error. Empty stack\n");
        exit(0); 
    }
    vm->sp--;
    return *vm->sp;
}

