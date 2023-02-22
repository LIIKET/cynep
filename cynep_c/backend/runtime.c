#pragma once

typedef enum ValueType ValueType;
typedef enum ObjectType ObjectType;

typedef struct VM VM;
typedef struct RuntimeValue RuntimeValue;
typedef struct Object Object;
typedef struct StringObject StringObject;
typedef struct CodeObject CodeObject;
typedef struct GlobalVar GlobalVar;
typedef struct Global Global;

RuntimeValue    VM_Eval(VM* vm, CodeObject* co, Global* global);
RuntimeValue    VM_Stack_Peek(VM* vm, size_t offset);
RuntimeValue    VM_Stack_Pop(VM* vm);
uint8_t         VM_Read_Byte(VM* vm);
uint8_t         VM_Peek_Byte(VM* vm);
uint64_t        VM_Read_Address(VM* vm);
void            VM_Stack_Push(VM* vm, RuntimeValue* value);
void            VM_Exception(char* msg);

#pragma region TYPES

enum ValueType 
{
    ValuteType_Number,
    ValueType_Boolean,
    ValueType_Object
};

enum ObjectType 
{
    ObjectType_String,
    ObjectType_Code
};

struct RuntimeValue 
{
    ValueType type;
    union
    {
        float64 number;
        bool boolean;
        Object* object;
    };
};

struct Object 
{
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
    uint64_t constants_last;
};

struct GlobalVar 
{
    char* name;
    RuntimeValue value;
};

struct Global 
{
    GlobalVar* globals;
    int64 globals_size;
    int64 globals_max;
};

struct VM 
{
    uint8_t* ip; // Instruction pointer
    Global* global;

    RuntimeValue* stack; // Array of values
    RuntimeValue* stack_start; // First address of stack
    RuntimeValue* stack_end; // Last address of stack
    RuntimeValue* sp; // Stack pointer
};

#pragma endregion

#pragma region RUNTIME_VALUE

#define NUMBER(value) (RuntimeValue){.type = ValuteType_Number, .number = value}
#define BOOLEAN(value) (RuntimeValue){.type = ValueType_Boolean, .boolean = value}

#define AS_STRING(value) (*(StringObject*)value.object)
#define AS_CODE(value) (*(CodeObject*)value.object)
#define AS_NUMBER(value) (*(float64*)value.number)
#define AS_BOOLEAN(value) (*(bool*)value.boolean)

char* RuntimeValue_ToString(RuntimeValue value){
    size_t buff_size = 100; 
    
    if(value.type == ValuteType_Number){
        char* buf = malloc(sizeof(char) * buff_size);
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
        char* buf = malloc(sizeof(char) * buff_size);
        strncpy(buf, code.name, code.name_length);
        return buf;
    }

    return "VM: Not implemented";
}

RuntimeValue Alloc_String(char* value, size_t length){
    RuntimeValue result;
    result.type = ValueType_Object;

    StringObject* stringObject = malloc(sizeof(StringObject));
    stringObject->object.objectType = ObjectType_String;
    stringObject->string = value;
    stringObject->length = length;
    result.object = (Object*)stringObject;
    
    return result;
}

RuntimeValue Alloc_Code(char* name, size_t name_length){
    RuntimeValue result;
    result.type = ValueType_Object;

    CodeObject* co = malloc(sizeof(CodeObject));
    co->object.objectType = ObjectType_Code;
    co->name = name;
    co->name_length = name_length;
    co->code_last = 0;
    co->constants_last = 0;
    result.object = (Object*)co;
    
    return result;
}

#pragma endregion

#pragma region GLOBAL_OBJECT

GlobalVar Global_Get(Global* global, int64 index){
    return global->globals[index];
}

void Global_Set(Global* global, int64 index, RuntimeValue* value){
    if(index >= global->globals_size){
        printf("\033[0;31mVM Error. Global index %n does not exist.\033[0m\n", index);
        exit(0);
    }

    global->globals[index].value = *value;
}

int64 Global_GetIndex(Global* global, char* name){
    if(global->globals_size > 1){
        for(int64 i = global->globals_size - 1; i >= 0; i--){
            if(strcmp(global->globals[i].name, name) == 0){
                return i;
            }
        }
    }

    return -1;
}

int64 Global_GetIndexBufferString(Global* global, BufferString* name){
    if(global->globals_size > 1){
        for(int64 i = global->globals_size - 1; i >= 0; i--){
            if(strncmp(global->globals[i].name, name->start, name->length) == 0){
                return i;
            }
        }
    }

    return -1;
}

void Global_Define(Global* global, BufferString* name){
    int64 index = Global_GetIndexBufferString(global, name);

    if(index != -1){
        return;
    }

    GlobalVar* var = &global->globals[global->globals_size++];
    var->name = malloc(sizeof(char) * name->length + 1);
    strncpy(var->name, name->start, name->length);
    var->name[name->length] = NULL_CHAR;
    var->value = NUMBER(0); // TODO: Set to null
}

void Global_Add(Global* global, char* name, float64 value){
    if(Global_GetIndex(global, name) != -1){
        return;
    }

    GlobalVar* var = &global->globals[global->globals_size];
    var->name = name;
    var->value = NUMBER(value);

    global->globals_size++;
}

Global* Create_Global(){
    Global* global = malloc(sizeof(Global));
    global->globals = malloc(sizeof(GlobalVar) * 10); // TODO: DANGER! Handle memory when adding
    global->globals_max = 10;
    global->globals_size = 0;
}

#pragma endregion

#pragma region VIRTUAL_MACHINE

#define OP_HALT             0x00
#define OP_CONST            0x01
#define OP_ADD              0x02
#define OP_SUB              0x03
#define OP_MUL              0x04
#define OP_DIV              0x05
#define OP_CMP              0x06
#define OP_JMP_IF_FALSE     0x07
#define OP_JMP              0x08
#define OP_POP              0x09
#define OP_GET_GLOBAL       0x0A
#define OP_SET_GLOBAL       0x0B

#define OP_CMP_GT           0x01
#define OP_CMP_LT           0x02
#define OP_CMP_EQ           0x03
#define OP_CMP_GE           0x04
#define OP_CMP_LE           0x05
#define OP_CMP_NE           0x06

#define STACK_LIMIT 1000000000 // DANGER! TODO: Reduce this when scopes are implemented.

RuntimeValue VM_exec(VM* vm, Global* global, CodeObject* codeObject){
    vm->global = global;
    vm->stack = malloc(sizeof(RuntimeValue) * STACK_LIMIT);
    vm->stack_start = vm->stack;
    vm->stack_end = vm->stack_start + sizeof(RuntimeValue) * STACK_LIMIT;
    
    vm->ip = &codeObject->code[0];
    vm->sp = &vm->stack[0];

    return VM_Eval(vm, codeObject, global);
}

#define BINARY_OP(operation)                    \
    do {                                        \
        float64 op2 = VM_Stack_Pop(vm).number;  \
        float64 op1 = VM_Stack_Pop(vm).number;  \
        float64 result = op1 operation op2;     \
        VM_Stack_Push(vm, &NUMBER(result));     \
    } while (false)                             \

RuntimeValue VM_Eval(VM* vm, CodeObject* co, Global* global){
    int64 t1 = timestamp();

    while(true){
        uint8_t opcode = VM_Read_Byte(vm);

        switch (opcode)
        {
            case OP_HALT:{
                int64 t2 = timestamp();
                printf("Execution time: %d ms\n", t2/1000-t1/1000);
                
                return VM_Stack_Pop(vm);
            }     

            case OP_CONST: {
                uint64_t constIndex = VM_Read_Address(vm);
                RuntimeValue constant = co->constants[constIndex];
                VM_Stack_Push(vm, &constant);

                break;
            }

            case OP_POP:{
                VM_Stack_Pop(vm);

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
                    case OP_CMP_GT:
                        res = op1.number > op2.number;
                        break;
                    case OP_CMP_LT:
                        res = op1.number < op2.number;
                        break;
                    case OP_CMP_EQ:
                        res = op1.number == op2.number;
                        break;
                    case OP_CMP_GE:
                        res = op1.number >= op2.number;
                        break;
                    case OP_CMP_LE:
                        res = op1.number <= op2.number;
                        break;
                    case OP_CMP_NE:
                        res = op1.number != op2.number;
                        break;
                    default:
                        VM_Exception("Illegal comparison");
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
                uint64_t address = VM_Read_Address(vm);

                if(!condition){
                    vm->ip = &co->code[address];
                }

                break;
            }
            case OP_JMP:{
                uint64_t address = VM_Read_Address(vm);
                vm->ip = &co->code[address];

                break;
            }

            case OP_GET_GLOBAL:{
                int64 address = VM_Read_Address(vm);
                RuntimeValue value = Global_Get(global, address).value;
                VM_Stack_Push(vm, &value);

                break;
            }

            case OP_SET_GLOBAL:{
                int64 index = VM_Read_Address(vm);
                RuntimeValue value = VM_Stack_Peek(vm, 0);
                Global_Set(global, index, &value);

                break;
            }

            default: {
                printf("\033[0;31mVM Error. Unrecognized opcode: %#x \033[0m\n", opcode);
                exit(0);
            }

        }
    }
}

#pragma endregion

#pragma region VIRTUAL_MACHINE_HELPER

// Reads 64 bit address.
uint64_t VM_Read_Address(VM* vm){ 
    uint64_t result = *vm->ip;
    // uint64_t result = *(int64_t*)(vm->ip);
    memcpy(&result, vm->ip, sizeof(uint64_t));
    vm->ip += 8;

    return result;
}

uint8_t VM_Read_Byte(VM* vm){
    return *vm->ip++;
}

uint8_t VM_Peek_Byte(VM* vm){
    return *vm->ip;
}

void VM_Stack_Push(VM* vm, RuntimeValue* value){
    if(vm->sp == vm->stack_end){
        VM_Exception("Stack Overflow");
    }
    RuntimeValue asd = *value;
    *vm->sp = *value;
    vm->sp++;
}

RuntimeValue VM_Stack_Pop(VM* vm){
    if(vm->sp < vm->stack_start){
        VM_Exception("Stack Empty");
    }
    vm->sp--;
    return *vm->sp;
}

RuntimeValue VM_Stack_Peek(VM* vm, size_t offset){
    if(vm->sp < vm->stack_start){
        VM_Exception("Stack Empty");
    }
    return *(vm->sp -1 - offset);
}

void VM_Exception(char* msg){
    printf("\033[0;31mVM: %s \033[0m\n", msg);
    exit(0);
}

#pragma endregion