#pragma once


typedef enum ValueType ValueType;
typedef enum ObjectType ObjectType;

typedef struct VM VM;
typedef struct RuntimeValue RuntimeValue;
typedef struct Object Object;
typedef struct StringObject StringObject;
typedef struct FunctionObject FunctionObject;
typedef struct GlobalVar GlobalVar;
typedef struct Program Program;
typedef struct LocalVar LocalVar;
typedef struct TypeInstanceObject TypeInstanceObject;
typedef struct MemberVar MemberVar;
typedef struct NativeFunctionObject NativeFunctionObject;
// typedef struct FunctionObject FunctionObject;
typedef struct TypeInfoObject TypeInfoObject;
typedef struct MemberInfo MemberInfo;
typedef struct Frame Frame;

RuntimeValue    vm_interp(VM* vm, Program* global);
RuntimeValue    VM_Stack_Peek(VM* vm, size_t offset);
RuntimeValue    VM_Stack_Pop(VM* vm);
uint8_t         VM_Read_Byte(VM* vm);
uint8_t         VM_Peek_Byte(VM* vm);
uint64_t        VM_Read_Address(VM* vm);
void            VM_Stack_Push(VM* vm, RuntimeValue* value);
void            VM_Exception(char* msg);
void            VM_DumpStack(VM* vm, uint8_t code);
char*           opcodeToString(uint8_t opcode);
int64_t         Member_GetIndex(TypeInfoObject* instance, char* name);


#pragma region TYPES

enum ValueType {
    ValueType_Null,
    ValueType_Number,
    ValueType_Boolean,
    ValueType_Object
};

enum ObjectType {
    ObjectType_String,
    ObjectType_Code,
    ObjectType_NativeFunction,
    ObjectType_TypeInfo,
    ObjectType_TypeInstance
};

struct RuntimeValue {
    ValueType type;
    union
    {
        float64 number;
        bool boolean;
        Object* object;
    };
};

struct Object {
    ObjectType objectType;
};

struct StringObject {
    Object object;
    char* string;
};

struct NativeFunctionObject {
    Object object; 
    void* func_ptr;
    char* name;
    size_t arity;
};

struct FunctionObject {
    Object object;
    char* name;
    size_t arity;
    uint8_t* code;
    RuntimeValue* constants;
    LocalVar* locals;

    int8_t scope_level; // Only for compiler state
};

struct TypeInfoObject {
    Object object;
    MemberInfo* members;
    size_t members_length;
    char* name;
};

struct MemberInfo {
    char* name;
};

struct TypeInstanceObject {
    Object object;
    MemberVar* members;
    size_t members_length;
    // char* name;
};

struct MemberVar {
    char* name;
    RuntimeValue value;
};

struct GlobalVar {
    char* name;
    RuntimeValue value;
};

struct LocalVar {
    char* name;
    int8_t scope_level;
    RuntimeValue value;
};

struct Program {
    GlobalVar* globals; // Array of global variables
    int64 globals_size;

    FunctionObject** code_objects; // all functions //! Why is this an array of pointers? Fix?
    size_t code_objects_length;
    FunctionObject* main_fn; // main function
};

struct Frame {
    uint8_t* ra; // return address
    RuntimeValue* bp; // base pointer of the caller
    FunctionObject* fn; // reference to the running function
};

struct VM {
    uint8_t* ip; // Instruction pointer
    Program* global;
    FunctionObject* fn; // Currently executing function
    RuntimeValue* sp; // Stack pointer
    RuntimeValue* bp; // Base pointer / Frame pointer
    RuntimeValue stack[512]; // Array of values
    Frame callStack[512];
    Frame* csp; // call stack pointer
};



#pragma endregion

#pragma region RUNTIME_VALUE

#define NUMBER(value) (RuntimeValue){.type = ValueType_Number, .number = value}
#define BOOLEAN(value) (RuntimeValue){.type = ValueType_Boolean, .boolean = value}
#define RUNTIME_NULL() (RuntimeValue){.type = ValueType_Null }

#define AS_STRING(value) (*(StringObject*)value.object)
// #define AS_CODE(value) (*(FunctionObject*)value.object)
#define AS_NATIVE_FUNCTION(value) (*(NativeFunctionObject*)value.object)
#define AS_TYPEINFO(value) (*(TypeInfoObject*)value.object)
#define AS_TYPEINSTANCE(value) (*(TypeInstanceObject*)value.object)
#define AS_FUNCTION(value) (*(FunctionObject*)value.object)
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
        FunctionObject code = AS_FUNCTION(value);
        return code.name;
    }
    if(value.type == ValueType_Object && value.object->objectType == ObjectType_String){
        StringObject str = AS_STRING(value);
        char* buf = malloc(sizeof(char) * buff_size);
        strcpy(buf, str.string);
        return buf;
    }
    if(value.type == ValueType_Object && value.object->objectType == ObjectType_NativeFunction){
        return "(Native function)";
    }
    if(value.type == ValueType_Object && value.object->objectType == ObjectType_TypeInfo){
        TypeInfoObject typeInfo = AS_TYPEINFO(value);
        return typeInfo.name;
    }
    if(value.type == ValueType_Object && value.object->objectType == ObjectType_TypeInstance){
        TypeInstanceObject typeInfo = AS_TYPEINSTANCE(value);
        // char* buf = malloc(sizeof(char) * buff_size);
        // gcvt(typeInfo.members[0].value.number, 6, buf);
        // return buf;
        return "(Type instance)";
    }

    return "VM: ToString not implemented";
}

RuntimeValue Alloc_String(char* value){
    RuntimeValue result;
    result.type = ValueType_Object;

    StringObject* stringObject = malloc(sizeof(StringObject));

    stringObject->object.objectType = ObjectType_String;
    stringObject->string = malloc(strlen(value) * sizeof(char) + 1);
    result.object = (Object*)stringObject;
    
    strcpy(stringObject->string, value);

    return result;
}

RuntimeValue Alloc_NativeFunction(void* func, char* name, size_t arity){
    RuntimeValue result;
    result.type = ValueType_Object;

    NativeFunctionObject* nativeFunctionObject = malloc(sizeof(NativeFunctionObject));

    nativeFunctionObject->object.objectType = ObjectType_NativeFunction;
    nativeFunctionObject->arity = arity;
    nativeFunctionObject->func_ptr = func;
    nativeFunctionObject->name = name;

    result.object = (Object*)nativeFunctionObject;
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

RuntimeValue Alloc_Code(char* name, size_t arity){
    RuntimeValue result;
    result.type = ValueType_Object;

    FunctionObject* co = malloc(sizeof(FunctionObject));
    co->object.objectType = ObjectType_Code;
    co->name = name;
    co->code = NULL;
    co->constants = NULL;
    co->locals = NULL;
    co->scope_level = 0;
    co->arity = arity;

    result.object = (Object*)co;
    
    return result;
}

RuntimeValue Alloc_TypeInfo(TypeDeclaration* typeDeclaration){
    RuntimeValue result;
    result.type = ValueType_Object;

    TypeInfoObject* co = malloc(sizeof(TypeInfoObject));
    co->object.objectType = ObjectType_TypeInfo;
    co->members = malloc(typeDeclaration->properties->count * sizeof(MemberInfo));
    co->members_length = typeDeclaration->properties->count;
    co->name= typeDeclaration->name;

    ListNode* cursor = typeDeclaration->properties->first;
    int i = 0;
    while(cursor != NULL){
        PropertyDeclaration* prop = (PropertyDeclaration*)cursor->value;
        MemberInfo* member = &co->members[i];

        member->name = prop->name;

        i++;
        cursor = cursor->next;
    }

    result.object = (Object*)co;
    
    return result;
}

RuntimeValue Alloc_TypeInstance(TypeInfoObject* typeInfo){
    RuntimeValue result;
    result.type = ValueType_Object;

    TypeInstanceObject* co = malloc(sizeof(TypeInstanceObject));

    co->object.objectType = ObjectType_TypeInstance;
    co->members = malloc(typeInfo->members_length * sizeof(MemberVar));
    co->members_length = typeInfo->members_length;

    for (size_t i = 0; i < typeInfo->members_length; i++)
    {
        co->members[i].name = typeInfo->members[i].name;
        co->members[i].value = NUMBER(789);
    }
    
    result.object = (Object*)co;
    
    return result;
}

#pragma endregion

#pragma region GLOBAL_OBJECT

GlobalVar Global_Get(Program* global, int64 index){
    return global->globals[index];
}

void Global_Set(Program* global, int64 index, RuntimeValue* value){
    if(index >= global->globals_size){
        printf("\033[0;31mVM Error. Global index %n does not exist.\033[0m\n", index);
        exit(0);
    }

    global->globals[index].value = *value;
}

int64 Global_GetIndex(Program* global, char* name){
    if(global->globals_size > 1){
        for(int64 i = global->globals_size - 1; i >= 0; i--){
            if(strcmp(global->globals[i].name, name) == 0){
                return i;
            }
        }
    }

    return -1;
}

int64 Member_GetIndex(TypeInfoObject* instance, char* name){
    if(instance->members_length > 1){
        for(int64 i = instance->members_length - 1; i >= 0; i--){
            if(strcmp(instance->members[i].name, name) == 0){
                return i;
            }
        }
    }

    return -1;
}

MemberVar Member_Get(TypeInstanceObject* co, int64 index){
    return co->members[index];
}

void program_define_global(Program* global, char* name)
{
    int64 index = Global_GetIndex(global, name);

    if(index != -1){
        return;
    }

    GlobalVar* var = &global->globals[global->globals_size++];
    var->name = name;

    var->value = NUMBER(0); // TODO: Set to null
}

void program_add_global(Program* global, char* name, RuntimeValue value)
{
    if(Global_GetIndex(global, name) != -1){
        return;
    }

    GlobalVar* var = &global->globals[global->globals_size];
    var->name = name;
    var->value = value;

    global->globals_size++;
}

void program_add_native_function(Program* global, char* name, void* func_ptr, size_t arity)
{
    if(Global_GetIndex(global, name) != -1){
        return;
    }

    RuntimeValue function = Alloc_NativeFunction(func_ptr, name, arity);
   
    GlobalVar* var = &global->globals[global->globals_size];
    var->name = name;
    var->value = function;

    global->globals_size++;
}

Program* make_program(){
    Program* global = malloc(sizeof(Program));
    global->globals = malloc(sizeof(GlobalVar) * 100); // TODO: DANGER! Handle memory when adding
    global->globals_size = 0;

    global->code_objects_length = 0;

    return global;
}

#pragma endregion

#pragma region LOCALS

int64 Local_GetIndex(FunctionObject* co, char* name){
    // We iterate backwards to grab the one with the closest scope level first. (It should be added closer to the end)
    if(array_length(co->locals) > 0)
    {
        for(int64 i = array_length(co->locals) - 1; i >= 0; i--)
        {
            if(strcmp(name, co->locals[i].name) == 0 )
            {
                return i;
            }
        }
    }

    return -1;
}

void Local_Define(FunctionObject* co, char* name){
    if(strcmp("si", name) == 0){
        int asd = 0;
    }

    int64 index = Local_GetIndex(co, name);

    if(index != -1)
    {
        return;
    }

    LocalVar var;
    var.scope_level = co->scope_level;
    var.name = name;
    var.value = RUNTIME_NULL();

    array_push(co->locals, var);
}

LocalVar Local_Get(FunctionObject* co, int64 index){
    return co->locals[index];
}

// void Local_Add(CodeObject* co, BufferString* name){
//     if(Local_GetIndexBufferString(co, name) != -1){
//         return;
//     }

//     LocalVar* var = &co->locals[co->locals_size];
//     var->scope_level = co->scope_level;
//     var->name = name;
//     co->locals_size++;
// }

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
#define OP_SET_GLOBAL       11
#define OP_GET_LOCAL        12
#define OP_SET_LOCAL        13
#define OP_SCOPE_EXIT       14
#define OP_CALL             15
#define OP_RETURN           16
#define OP_GET_MEMBER       17

#define OP_CMP_GT           0x01
#define OP_CMP_LT           0x02
#define OP_CMP_EQ           0x03
#define OP_CMP_GE           0x04
#define OP_CMP_LE           0x05
#define OP_CMP_NE           0x06

#define STACK_LIMIT 512

RuntimeValue vm_exec(VM* vm, Program* global)
{
    vm->global = global;
    
    FunctionObject* co = global->main_fn;
    vm->fn = co;
    vm->ip = &co->code[0];
    vm->sp = &vm->stack[0];
    vm->bp = &vm->stack[0];

    vm->csp = vm->callStack;

    return vm_interp(vm, global);
}

RuntimeValue vm_interp(VM* vm, Program* global)
{
    int64 t1 = timestamp();

    static void* dispatch_table[] = {
    &&DO_OP_HALT, &&DO_OP_CONST, &&DO_OP_ADD, &&DO_OP_SUB,
    &&DO_OP_MUL, &&DO_OP_DIV, &&DO_OP_CMP, &&DO_OP_JMP_IF_FALSE, &&DO_OP_JMP, &&DO_OP_POP, &&DO_OP_GET_GLOBAL,
    &&DO_OP_SET_GLOBAL, &&DO_OP_GET_LOCAL, &&DO_OP_SET_LOCAL, &&DO_OP_SCOPE_EXIT, &&DO_OP_CALL, &&DO_OP_RETURN, &&DO_OP_GET_MEMBER};

    // TODO: Put this shit in dispatch macro
    // Introspect stack for debugging
    // VM_DumpStack(vm, opcode);

    #define DISPATCH()                          \
    do {                                        \
        goto *dispatch_table[VM_Read_Byte(vm)]; \
    } while (false)                             \

    #define BINARY_OP(operation)                \
    do {                                        \
        float64 op2 = VM_Stack_Pop(vm).number;  \
        float64 op1 = VM_Stack_Pop(vm).number;  \
        float64 result = op1 operation op2;     \
        VM_Stack_Push(vm, &NUMBER(result));     \
    } while (false)                             \

    DISPATCH();

    DO_OP_HALT: {
        int64 t2 = timestamp();
        printf("Execution time: %d ms\n", t2/1000-t1/1000);
        return VM_Stack_Pop(vm);
    }

    DO_OP_CONST: {
        uint64_t constIndex = VM_Read_Address(vm);
        RuntimeValue constant = vm->fn->constants[constIndex];
        VM_Stack_Push(vm, &constant);

        DISPATCH();
    }

    DO_OP_ADD: {
        RuntimeValue op2 = VM_Stack_Pop(vm);
        RuntimeValue op1 = VM_Stack_Pop(vm);

        if(op1.type == ValueType_Number && op2.type == ValueType_Number)
        {
            float64 result = op1.number + op2.number;

            VM_Stack_Push(vm, &NUMBER(result));

            DISPATCH();
        }
        else if(op1.type == ValueType_Object && op1.object->objectType == ObjectType_String
             && op2.type == ValueType_Object && op2.object->objectType == ObjectType_String)
        { 
            StringObject str1 = AS_STRING(op1);
            StringObject str2 = AS_STRING(op2);
            RuntimeValue value = Alloc_String_Combine(&str1, &str2);

            VM_Stack_Push(vm, &value);

            DISPATCH();
        }

        VM_Exception("Illegal add operation.");
    }

    DO_OP_SUB: {
        BINARY_OP(-);
        DISPATCH();
    }

    DO_OP_MUL: {
        BINARY_OP(*);
        DISPATCH();
    }

    DO_OP_DIV: {
        BINARY_OP(/);
        DISPATCH();
    }

    DO_OP_CMP: {
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
        else 
        {
            VM_Exception("Illegal comparison.");
        }

        VM_Stack_Push(vm, &BOOLEAN(res));

        DISPATCH();
    }

    DO_OP_JMP_IF_FALSE: {
        bool condition = VM_Stack_Pop(vm).boolean;
        uint64_t address = VM_Read_Address(vm);

        if(!condition){
            vm->ip = &vm->fn->code[address];
        }
    
        DISPATCH();
    }

    DO_OP_JMP: {
        uint64_t address = VM_Read_Address(vm);
        vm->ip = &vm->fn->code[address];

        DISPATCH();
    }

    DO_OP_POP: {
        VM_Stack_Pop(vm);

        DISPATCH();
    }

    DO_OP_GET_GLOBAL: {
        int64 address = VM_Read_Address(vm);
        RuntimeValue value = Global_Get(global, address).value;
        VM_Stack_Push(vm, &value);

        DISPATCH();
    }

    DO_OP_SET_GLOBAL: {
        int64 index = VM_Read_Address(vm);
        RuntimeValue value = VM_Stack_Peek(vm, 0);
        Global_Set(global, index, &value);

        DISPATCH();
    }

    DO_OP_GET_LOCAL: {
        uint64 address = VM_Read_Address(vm);
        if(address < 0 ){ 
            VM_Exception("Invalid variable index.");
        }

        VM_Stack_Push(vm, &vm->bp[address]);

        DISPATCH();
    }

    DO_OP_SET_LOCAL: {
        int64 index = VM_Read_Address(vm);
        RuntimeValue value = VM_Stack_Peek(vm, 0);
        vm->bp[index] = value;

        DISPATCH();
    }

    DO_OP_SCOPE_EXIT: {
        uint64 count = VM_Read_Address(vm);

        if(count > 0)
        {
            // Move the result above the vars that is getting popped
            *(vm->sp - 1 - count) = VM_Stack_Peek(vm, 0);

            // Pop back to before scope
            vm->sp -= count;
        }

        DISPATCH();
    }

    DO_OP_CALL: {
        uint64_t arg_count = VM_Read_Address(vm);
        RuntimeValue fnValue = VM_Stack_Pop(vm);

        if(fnValue.type == ValueType_Object && fnValue.object->objectType == ObjectType_NativeFunction){
            NativeFunctionObject fn = AS_NATIVE_FUNCTION(fnValue);

            RuntimeValue args[arg_count];

            for (size_t i = 0; i < arg_count; i++)
            {
                args[i] = VM_Stack_Pop(vm);
            }

            RuntimeValue (*fun_ptr)() = fn.func_ptr; // Function pointer
            RuntimeValue res = (*fun_ptr)(arg_count, &args); // Invoke
            
            VM_Stack_Push(vm, &res); // Push the result
        }
        else
        {
            FunctionObject* fn = (FunctionObject*)(fnValue.object);

            // Save execution context, restored on OP_RETURN
            Frame fr = {
                .bp = vm->bp,
                .fn = vm->fn,
                .ra = vm->ip
            };
            *vm->csp = fr;
            vm->csp++;

            // Set function on vm
            vm->fn = fn;

            // set base pointer (frame) to the call
            vm->bp = vm->sp - arg_count;

            // Jump to the function code
            vm->ip = &fn->code[0];
        }

        DISPATCH();
    }

    DO_OP_RETURN: {
        // Restore frame
        vm->csp--;
        vm->ip = vm->csp->ra;
        vm->bp = vm->csp->bp;
        vm->fn = vm->csp->fn;

        DISPATCH();
    }

    DO_OP_GET_MEMBER: {
        RuntimeValue instanceVal = VM_Stack_Pop(vm);
        TypeInstanceObject instance = AS_TYPEINSTANCE(instanceVal);
        int64 memberIndex = VM_Read_Address(vm);

        RuntimeValue memberValue = Member_Get(&instance, memberIndex).value;
        VM_Stack_Push(vm, &memberValue);

        DISPATCH();
    }
}

#pragma endregion

#pragma region VIRTUAL_MACHINE_HELPER

// Reads 64 bit address.
uint64_t VM_Read_Address(VM* vm){ 
    uint64_t result;
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
    if(vm->sp == &vm->stack[512]){
        VM_Exception("Stack Overflow");
    }
    *vm->sp = *value;
    vm->sp++;
}

RuntimeValue VM_Stack_Pop(VM* vm){
    if(vm->sp == vm->stack){
        VM_Exception("Stack Empty");
    }
    vm->sp--;
    return *vm->sp;
}

RuntimeValue VM_Stack_Peek(VM* vm, size_t offset){
    if(vm->sp < vm->stack){
        VM_Exception("Stack Empty");
    }
    return *(vm->sp -1 - offset);
}

void VM_Exception(char* msg){
    printf("\033[0;31mVM: %s \033[0m\n", msg);
    exit(0);
}

void VM_DumpStack(VM* vm, uint8_t code){

    char* op  = opcodeToString(code);
    printf("------------------ STACK AT %-10s -------------\n", op);

    if(vm->sp == vm->stack){
        printf("(empty)\n");
    }
    else if(vm->sp < vm->stack){
           printf("(less)\n");     
    }
    else{
        RuntimeValue* csp = vm->sp - 1;
        while(csp >= vm->stack){
            char* asd = RuntimeValue_ToString(*csp--);

            printf("%s\n", asd);
        }
    }

    printf("\n");
}

#pragma endregion