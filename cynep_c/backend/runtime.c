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
typedef struct LocalVar LocalVar;

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
    ValueType_Null,
    ValueType_Number,
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
};

struct CodeObject 
{
    Object object;
    char* name;
    size_t name_length;
    uint8_t* code; // Array of opcodes
    size_t code_last;
    RuntimeValue* constants;
    uint64_t constants_last;

    int8_t scope_level;
    LocalVar* locals;
    uint64 locals_size;
    uint64 locals_max;
};

struct GlobalVar 
{
    char* name;
    RuntimeValue value;
};

struct LocalVar 
{
    char* name;
    int8_t scope_level;
    RuntimeValue value;
};

struct Global 
{
    GlobalVar* globals; // Array of global variables
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
    RuntimeValue* bp; // Base pointer / Frame pointer
};

#pragma endregion

#pragma region RUNTIME_VALUE

#define NUMBER(value) (RuntimeValue){.type = ValueType_Number, .number = value}
#define BOOLEAN(value) (RuntimeValue){.type = ValueType_Boolean, .boolean = value}
#define RUNTIME_NULL() (RuntimeValue){.type = ValueType_Null }

#define AS_STRING(value) (*(StringObject*)value.object)
#define AS_CODE(value) (*(CodeObject*)value.object)
#define AS_NUMBER(value) (*(float64*)value.number)
#define AS_BOOLEAN(value) (*(bool*)value.boolean)

char* RuntimeValue_ToString(RuntimeValue value){
    size_t buff_size = 100; 
    
    if(value.type == ValueType_Number){
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
    if(value.type == ValueType_Null){
        return "null";
    }
    if(value.type == ValueType_Object && value.object->objectType == ObjectType_Code){
        CodeObject code = AS_CODE(value);
        char* buf = malloc(sizeof(char) * buff_size);
        strncpy(buf, code.name, code.name_length);
        return buf;
    }
    if(value.type == ValueType_Object && value.object->objectType == ObjectType_String){
        StringObject str = AS_STRING(value);
        char* buf = malloc(sizeof(char) * buff_size);
        strcpy(buf, str.string);
        return buf;
    }

    return "VM: ToString not implemented";
}

RuntimeValue Alloc_String(char* value){
    RuntimeValue result;
    result.type = ValueType_Object;

    StringObject* stringObject = malloc(sizeof(StringObject));

    stringObject->object.objectType = ObjectType_String;
    stringObject->string = value;
    result.object = (Object*)stringObject;
    
    return result;
}

RuntimeValue Alloc_String_Combine(StringObject* one, StringObject* two){
    RuntimeValue result;
    result.type = ValueType_Object;

    char* str1 = one->string;
    char* str2 = two->string;

    uint64 str1_size = strlen(str1);
    uint64 str2_size = strlen(str2);

    // Put string and string object in same memory block for performance
    void* memory = malloc(sizeof(StringObject) + (str1_size + str2_size) * sizeof(char));
    char* string = memory + sizeof(StringObject);

    strcpy(string, str1);
    strcpy(&string[str1_size], str2);

    StringObject* stringObject = memory;
    stringObject->object.objectType = ObjectType_String;

    stringObject->string = string;

    result.object = (Object*)stringObject;
    
    return result;
}

RuntimeValue Alloc_String_From_BufferString(BufferString* value){
    RuntimeValue result;
    result.type = ValueType_Object;

    // Put string and string object in same memory block for performance
    void* memory = malloc(sizeof(StringObject) + (value->length * sizeof(char)) + 1);
    char* string = memory + sizeof(StringObject);

    strncpy(string, value->start, value->length);

    string[value->length] = NULL_CHAR;

    StringObject* stringObject = memory;
    stringObject->object.objectType = ObjectType_String;
    stringObject->string = string;

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
    co->scope_level = -1;

    co->locals = malloc(sizeof(LocalVar) * 10); // TODO: DANGER! Handle memory when adding
    co->locals_size = 0;
    co->locals_max = 10;

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
            uint64 current_length = strlen(global->globals[i].name);
            if(current_length == name->length && strncmp(name->start, global->globals[i].name, name->length) == 0){
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

void Global_Add(Global* global, char* name, RuntimeValue value){
    if(Global_GetIndex(global, name) != -1){
        return;
    }

    GlobalVar* var = &global->globals[global->globals_size];
    var->name = name;
    var->value = value;

    global->globals_size++;
}

Global* Create_Global(){
    Global* global = malloc(sizeof(Global));
    global->globals = malloc(sizeof(GlobalVar) * 10); // TODO: DANGER! Handle memory when adding
    global->globals_max = 10;
    global->globals_size = 0;
}

#pragma endregion

#pragma region LOCALS

int64 Local_GetIndexBufferString(CodeObject* co, BufferString* name){
    if(co->locals_size > 0){
        for(int64 i = co->locals_size - 1; i >= 0; i--){
            uint64 current_length = strlen(co->locals[i].name);

            if(current_length == name->length 
            && strncmp(name->start, co->locals[i].name, name->length) == 0
            && co->locals[i].scope_level == co->scope_level){
                return i;
            }
        }
    }

    return -1;
}

void Local_Define(CodeObject* co, BufferString* name){
    int64 index = Local_GetIndexBufferString(co, name);

    if(index != -1){
        return;
    }

    LocalVar* var = &co->locals[co->locals_size++];
    var->scope_level = co->scope_level;
    var->name = malloc(sizeof(char) * name->length + 1);
    strncpy(var->name, name->start, name->length);
    var->name[name->length] = NULL_CHAR;
    var->value = RUNTIME_NULL(); // TODO: Set to null
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
#define OP_GET_LOCAL        0x0C
#define OP_SET_LOCAL        0x0D
#define OP_SCOPE_EXIT       0x0E

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
    vm->bp = &vm->stack[0];

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

            case OP_ADD: {
                RuntimeValue op2 = VM_Stack_Pop(vm);
                RuntimeValue op1 = VM_Stack_Pop(vm);

                if(op1.type == ValueType_Number && op2.type == ValueType_Number){
                    float64 result = op1.number + op2.number;

                    VM_Stack_Push(vm, &NUMBER(result));

                    break;
                }
                else if(op1.type == ValueType_Object && op1.object->objectType == ObjectType_String
                     && op2.type == ValueType_Object && op2.object->objectType == ObjectType_String){
                    
                    StringObject str1 = AS_STRING(op1);
                    StringObject str2 = AS_STRING(op2);
                    RuntimeValue value = Alloc_String_Combine(&str1, &str2);

                    VM_Stack_Push(vm, &value);

                    break;
                }

                VM_Exception("Illegal add operation.");
            }

            case OP_SUB: BINARY_OP(-); break;
            case OP_MUL: BINARY_OP(*); break;
            case OP_DIV: BINARY_OP(/); break;
            
            case OP_CMP:{
                uint8_t cmp_type = VM_Read_Byte(vm);

                RuntimeValue op2 = VM_Stack_Pop(vm);
                RuntimeValue op1 = VM_Stack_Pop(vm);
                bool res;

                if(op2.type == ValueType_Number && op1.type == ValueType_Number)
                {
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
                            VM_Exception("Illegal comparison.");
                    }  
                }
                else if(op2.type == ValueType_Boolean && op1.type == ValueType_Boolean)
                {
                    switch (cmp_type)
                    {
                        case OP_CMP_EQ:
                            res = op1.boolean == op2.boolean;
                            break;
                        case OP_CMP_NE:
                            res = op1.boolean != op2.boolean;
                            break;
                        default:
                            VM_Exception("Illegal comparison.");
                    }  
                }
                else if(op1.type == ValueType_Object && op1.object->objectType == ObjectType_String
                     && op2.type == ValueType_Object && op2.object->objectType == ObjectType_String)
                {
                    StringObject str1 = AS_STRING(op1);
                    StringObject str2 = AS_STRING(op2);

                    switch (cmp_type)
                    {
                        case OP_CMP_EQ:
                            res = strcmp(str1.string, str2.string) == 0;
                            break;
                        case OP_CMP_NE:
                            res = strcmp(str1.string, str2.string) != 0;
                            break;
                        default:
                            VM_Exception("Illegal comparison.");
                    }  
                }
                else if(op2.type == ValueType_Null || op1.type == ValueType_Null)
                {
                    switch (cmp_type)
                    {
                        case OP_CMP_EQ:
                            res = op2.type == ValueType_Null && op1.type == ValueType_Null;
                            break;
                        case OP_CMP_NE:
                            res = op2.type != ValueType_Null || op1.type != ValueType_Null;
                            break;
                        default:
                            VM_Exception("Illegal comparison.");
                    }  
                }
                else{
                    VM_Exception("Illegal comparison.");
                }

                VM_Stack_Push(vm, &BOOLEAN(res));

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

            case OP_GET_LOCAL:{
                uint64 address = VM_Read_Address(vm);
                if(address < 0 ){ //|| address >= vm->stack_end
                    VM_Exception("Invalid variable index.");
                }

                VM_Stack_Push(vm, &vm->bp[address]);

                break;
            }

            case OP_SET_LOCAL:{
                int64 index = VM_Read_Address(vm);
                RuntimeValue value = VM_Stack_Peek(vm, 0);
                vm->bp[index] = value;

                break;
            }

            case OP_SCOPE_EXIT:{
                uint64 count = VM_Read_Address(vm);

                if(count > 0){
                    // Move the result above the vars that is getting popped
                    *(vm->sp - 1 - count) = VM_Stack_Peek(vm, 0);

                    // Pop back to before scope
                    vm->sp -= count;
                }
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