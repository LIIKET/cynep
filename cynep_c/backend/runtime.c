#pragma once

typedef enum        ValueType ValueType;
typedef enum        ObjectType ObjectType;
typedef struct      VM VM;
typedef struct      Object Object;
typedef struct      StringObject StringObject;
typedef struct      FunctionObject FunctionObject;
typedef struct      GlobalVar GlobalVar;
typedef struct      Program Program;
typedef struct      LocalVar LocalVar;
typedef struct      TypeInstanceObject TypeInstanceObject;
typedef struct      MemberVar MemberVar;
typedef struct      NativeFunctionObject NativeFunctionObject;
typedef struct      TypeInfoObject TypeInfoObject;
typedef struct      MemberInfo MemberInfo;
typedef struct      Frame Frame;
typedef uint64_t    RuntimeValue;

RuntimeValue    vm_interp(VM* vm, Program* global);
uint8_t         VM_Peek_Byte(VM* vm);
uint64_t        VM_Read_Address(VM* vm);
void            VM_Stack_Push(VM* vm, RuntimeValue value);
void            VM_Exception(char* msg);
void            VM_DumpStack(VM* vm, uint8_t code);
char*           opcodeToString(uint8_t opcode);
int64_t         Member_GetIndex(TypeInfoObject* instance, char* name);


#pragma region TYPES

enum ObjectType {
    ObjectType_String,
    ObjectType_Code,
    ObjectType_NativeFunction,
    ObjectType_TypeInfo,
    ObjectType_TypeInstance
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
    FunctionObject** functions; // all functions //! Why is this an array of pointers? Fix?
    FunctionObject* main_function; // main function
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
    Frame callstack[512];
    Frame* csp; // call stack pointer
};



#pragma endregion

#pragma region RUNTIME_VALUE

#define QUIET_NAN ((uint64_t)0x7ffc000000000000)
#define SIGN_BIT  ((uint64_t)0x8000000000000000)
#define TAG_NULL  1
#define TAG_FALSE 2
#define TAG_TRUE  3

#define NUMBER_VAL(num) (numToValue(num))
#define FALSE_VAL ((RuntimeValue)(uint64_t)(QUIET_NAN | TAG_FALSE))
#define TRUE_VAL ((RuntimeValue)(uint64_t)(QUIET_NAN | TAG_TRUE))
#define NULL_VAL ((RuntimeValue)(uint64_t)(QUIET_NAN | TAG_NULL))
#define BOOL_VAL(val) ((val) ? TRUE_VAL : FALSE_VAL)
#define OBJ_VAL(obj) (RuntimeValue)(SIGN_BIT | QUIET_NAN | (uint64_t)(uintptr_t)(obj))

#define IS_NUMBER(value) (((value) & QUIET_NAN) != QUIET_NAN)
#define IS_NULL(value) ((value) == NULL_VAL)
#define IS_BOOL(value) (((value) | 1) == TRUE_VAL)
#define IS_OBJ(value) (((value) & (QUIET_NAN | SIGN_BIT)) == (QUIET_NAN | SIGN_BIT))

#define AS_C_BOOL(value) ((value) == TRUE_VAL)
#define AS_C_DOUBLE(value) valueToNum(value)
#define AS_C_OBJ(value) ((Object*)(uintptr_t)((value) & ~(SIGN_BIT | QUIET_NAN)))

#define AS_STRING(value) (*(StringObject*)AS_C_OBJ(value))
#define AS_NATIVE_FUNCTION(value) (*(NativeFunctionObject*)AS_C_OBJ(value))
#define AS_TYPEINFO(value) (*(TypeInfoObject*)AS_C_OBJ(value))
#define AS_TYPEINSTANCE(value) (*(TypeInstanceObject*)AS_C_OBJ(value))
#define AS_FUNCTION(value) (*(FunctionObject*)AS_C_OBJ(value))

static inline RuntimeValue numToValue(double num){
    RuntimeValue value;
    memcpy(&value, &num, sizeof(double));
    return value;
}

static inline double valueToNum(RuntimeValue val){
    double num;
    memcpy(&num, &val, sizeof(double));
    return num;
}

char* RuntimeValue_ToString(RuntimeValue value){
    size_t buff_size = 100; 
    
    if(IS_NUMBER(value)){
        char* buf = malloc(sizeof(char) * buff_size);
        gcvt(AS_C_DOUBLE(value), 6, buf);
        return buf;
    }
    if(IS_BOOL(value)){
        if(AS_C_BOOL(value) == true)
            return "true";
        else
            return "false";
    }
    if(IS_NULL(value)){
        return "null";
    }
    if(IS_OBJ(value) && AS_C_OBJ(value)->objectType == ObjectType_Code){
        FunctionObject code = AS_FUNCTION(value);
        return code.name;
    }
    if(IS_OBJ(value) && AS_C_OBJ(value)->objectType == ObjectType_String){
        StringObject str = AS_STRING(value);
        char* buf = malloc(sizeof(char) * buff_size);
        strcpy(buf, str.string);
        return buf;
    }
    if(IS_OBJ(value) && AS_C_OBJ(value)->objectType == ObjectType_NativeFunction){
        return "(Native function)";
    }
    if(IS_OBJ(value) && AS_C_OBJ(value)->objectType == ObjectType_TypeInfo){
        TypeInfoObject typeInfo = AS_TYPEINFO(value);
        return typeInfo.name;
    }
    if(IS_OBJ(value) && AS_C_OBJ(value)->objectType == ObjectType_TypeInstance){
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

    StringObject* stringObject = malloc(sizeof(StringObject));

    stringObject->object.objectType = ObjectType_String;
    stringObject->string = malloc(strlen(value) * sizeof(char) + 1);

    result = OBJ_VAL(stringObject);
    
    strcpy(stringObject->string, value);

    return result;
}

RuntimeValue Alloc_NativeFunction(void* func, char* name, size_t arity){
    RuntimeValue result;

    NativeFunctionObject* nativeFunctionObject = malloc(sizeof(NativeFunctionObject));

    nativeFunctionObject->object.objectType = ObjectType_NativeFunction;
    nativeFunctionObject->arity = arity;
    nativeFunctionObject->func_ptr = func;
    nativeFunctionObject->name = name;

    result = OBJ_VAL(nativeFunctionObject);
    return result;
}

RuntimeValue Alloc_String_Combine(StringObject* one, StringObject* two){
    RuntimeValue result;

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

    result = OBJ_VAL(stringObject);
    
    return result;
}

RuntimeValue Alloc_Function(char* name, size_t arity){
    RuntimeValue result;


    FunctionObject* co = malloc(sizeof(FunctionObject));
    co->object.objectType = ObjectType_Code;
    co->name = name;
    co->code = NULL;
    co->constants = NULL;
    co->locals = NULL;
    co->scope_level = 0;
    co->arity = arity;

    arrsetcap(co->code, 1);
    arrsetcap(co->constants, 1);
    arrsetcap(co->locals, 1);

    result = OBJ_VAL(co);
    
    return result;
}

RuntimeValue Alloc_TypeInfo(TypeDeclaration* typeDeclaration){
    RuntimeValue result;

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

    result = OBJ_VAL(co);
    
    return result;
}

RuntimeValue Alloc_TypeInstance(TypeInfoObject* typeInfo){
    RuntimeValue result;

    TypeInstanceObject* co = malloc(sizeof(TypeInstanceObject));

    co->object.objectType = ObjectType_TypeInstance;
    co->members = malloc(typeInfo->members_length * sizeof(MemberVar));
    co->members_length = typeInfo->members_length;

    for (size_t i = 0; i < typeInfo->members_length; i++)
    {
        co->members[i].name = typeInfo->members[i].name;
        co->members[i].value = NUMBER_VAL(789);
    }
    
    result = OBJ_VAL(co);
    
    return result;
}

#pragma endregion

#pragma region GLOBAL_OBJECT

GlobalVar Global_Get(Program* global, int64 index){
    return global->globals[index];
}

void Global_Set(Program* global, int64 index, RuntimeValue* value){
    if(index >= array_length(global->globals)){
        printf("\033[0;31mVM Error. Global index %n does not exist.\033[0m\n", index);
        exit(0);
    }

    global->globals[index].value = *value;
}

int64 Global_GetIndex(Program* global, char* name){
    if(array_length(global->globals) > 1){
        for(int64 i = array_length(global->globals) - 1; i >= 0; i--){
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

    GlobalVar var;
    var.name = name;
    var.value = NUMBER_VAL(0); // TODO: Set to null

    array_push(global->globals, var);
}

void program_add_global(Program* global, char* name, RuntimeValue value)
{
    if(Global_GetIndex(global, name) != -1){
        return;
    }

    GlobalVar var;
    var.name = name;
    var.value = value;

    array_push(global->globals, var);
}

void program_add_native_function(Program* global, char* name, void* func_ptr, size_t arity)
{
    if(Global_GetIndex(global, name) != -1){
        return;
    }

    RuntimeValue function = Alloc_NativeFunction(func_ptr, name, arity);
   
    GlobalVar var;
    var.name = name;
    var.value = function;

    array_push(global->globals, var);
}

Program* make_program(){
    Program* global = malloc(sizeof(Program));
    global->globals = NULL;
    global->functions = NULL;

    return global;
}

#pragma endregion

#pragma region LOCALS

int64_t Local_GetIndex(FunctionObject* func, char* name){
    // We iterate backwards to grab the one with the closest scope level first. (It should be added closer to the end)
    if(array_length(func->locals) > 0)
    {
        for(int64_t i = array_length(func->locals) - 1; i >= 0; i--)
        {
            if(strcmp(name, func->locals[i].name) == 0 )
            {
                return i;
            }
        }
    }

    return -1;
}

void Local_Define(FunctionObject* func, char* name){

    // TODO: den här ska hämta med strikt scope
    // int64 index = Local_GetIndex(co, name);

    // if(index != -1)
    // {
    //     return;
    // }

    LocalVar var;
    var.scope_level = func->scope_level;
    var.name = name;
    var.value = NULL_VAL;

    array_push(func->locals, var);

    return;
}

LocalVar Local_Get(FunctionObject* func, int64 index){
    return func->locals[index];
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
#define OP_SET_GLOBAL       0x0B
#define OP_GET_LOCAL        0x0C
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
    
    FunctionObject* co = global->main_function;
    vm->fn = co;
    vm->ip = &co->code[0];
    vm->sp = &vm->stack[0];
    vm->bp = &vm->stack[0];
    vm->csp = vm->callstack;

    return vm_interp(vm, global);
}

RuntimeValue vm_interp(register VM* vm, Program* global)
{
    int64 t1 = timestamp();

    register uint8_t* ip = vm->ip;
    register RuntimeValue* sp = vm->sp;

    #define POP() (*(--sp))
    #define READ_BYTE() (*ip++)
    #define PEEK() (*(sp - 1))
    #define PUSH(value) (*sp++ = value)
    #define READ_ADDRESS(out) (*(uint64_t*)memcpy(&out, ip, sizeof(uint64_t))); ip += 8

    static void* dispatch_table[] = {
    &&DO_OP_HALT, &&DO_OP_CONST, &&DO_OP_ADD, &&DO_OP_SUB,
    &&DO_OP_MUL, &&DO_OP_DIV, &&DO_OP_CMP, &&DO_OP_JMP_IF_FALSE, &&DO_OP_JMP, &&DO_OP_POP, &&DO_OP_GET_GLOBAL,
    &&DO_OP_SET_GLOBAL, &&DO_OP_GET_LOCAL, &&DO_OP_SET_LOCAL, &&DO_OP_SCOPE_EXIT, &&DO_OP_CALL, &&DO_OP_RETURN, &&DO_OP_GET_MEMBER};

    uint8_t opcode;

    #define DISPATCH()                                   \
    do {                                                 \
        /* Introspect stack for debugging */             \
         /*VM_DumpStack(vm, opcode);*/                   \
        goto *dispatch_table[opcode = READ_BYTE()]; \
    } while (false)                                      \

    #define BINARY_OP(operation)                         \
    do {                                                 \
        double op2 = AS_C_DOUBLE(POP());                 \
        double op1 = AS_C_DOUBLE(POP());                 \
        double result = op1 operation op2;               \
        PUSH(NUMBER_VAL(result));                        \
    } while (false)                                      \

    DISPATCH();

    DO_OP_HALT: {
        int64 t2 = timestamp();
        printf("Execution time: %d ms\n", t2/1000-t1/1000);
        return POP();
    }

    DO_OP_CONST: {
        uint64_t constIndex = READ_ADDRESS(constIndex);
        RuntimeValue constant = vm->fn->constants[constIndex];
        PUSH(constant);

        DISPATCH();
    }

    DO_OP_ADD: {
        RuntimeValue op2 = POP();
        RuntimeValue op1 = POP();

        if(IS_NUMBER(op1) && IS_NUMBER(op2))
        {
            double result = AS_C_DOUBLE(op1) + AS_C_DOUBLE(op2);
            RuntimeValue runtime_result = NUMBER_VAL(result);

            PUSH(runtime_result);

            DISPATCH();
        }
        else if(IS_OBJ(op1) && AS_C_OBJ(op1)->objectType == ObjectType_String
             && IS_OBJ(op2) && AS_C_OBJ(op2)->objectType == ObjectType_String)
        { 
            StringObject str1 = AS_STRING(op1);
            StringObject str2 = AS_STRING(op2);
            RuntimeValue value = Alloc_String_Combine(&str1, &str2);

            PUSH(value);

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
        uint8_t cmp_type = READ_BYTE();

        RuntimeValue op2 = POP();
        RuntimeValue op1 = POP();
        bool res;

        if(IS_NUMBER(op2) && IS_NUMBER(op1))
        {
            switch (cmp_type)
            {
                case OP_CMP_GT:
                    res = AS_C_DOUBLE(op1) > AS_C_DOUBLE(op2);
                    break;
                case OP_CMP_LT:
                    res = AS_C_DOUBLE(op1) < AS_C_DOUBLE(op2);
                    break;
                case OP_CMP_EQ:
                    res = AS_C_DOUBLE(op1) == AS_C_DOUBLE(op2);
                    break;
                case OP_CMP_GE:
                    res = AS_C_DOUBLE(op1) >= AS_C_DOUBLE(op2);
                    break;
                case OP_CMP_LE:
                    res = AS_C_DOUBLE(op1) <= AS_C_DOUBLE(op2);
                    break;
                case OP_CMP_NE:
                    res = AS_C_DOUBLE(op1) != AS_C_DOUBLE(op2);
                    break;
                default:
                    VM_Exception("Illegal comparison.");
            }  
        }
        else if(IS_BOOL(op2) && IS_BOOL(op1))
        {
            switch (cmp_type)
            {
                case OP_CMP_EQ:
                    res = AS_C_BOOL(op1) == AS_C_BOOL(op2);
                    break;
                case OP_CMP_NE:
                    res = AS_C_BOOL(op1) != AS_C_BOOL(op2);
                    break;
                default:
                    VM_Exception("Illegal comparison.");
            }  
        }
        else if(IS_OBJ(op1) && AS_C_OBJ(op1)->objectType == ObjectType_String
             && IS_OBJ(op2) && AS_C_OBJ(op2)->objectType == ObjectType_String)
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
        else if(IS_NULL(op2) || IS_NULL(op1))
        {
            switch (cmp_type)
            {
                case OP_CMP_EQ:
                    res = IS_NULL(op2) && IS_NULL(op1);
                    break;
                case OP_CMP_NE:
                    res = !IS_NULL(op2) || !IS_NULL(op1);
                    break;
                default:
                    VM_Exception("Illegal comparison.");
            }  
        }
        else 
        {
            VM_Exception("Illegal comparison.");
        }

        RuntimeValue runtime_result = BOOL_VAL(res);
        PUSH(runtime_result);

        DISPATCH();
    }

    DO_OP_JMP_IF_FALSE: {
        bool condition = AS_C_BOOL(POP());
        uint64_t address = READ_ADDRESS(address);

        if(!condition){
            ip = &vm->fn->code[address];
        }
    
        DISPATCH();
    }

    DO_OP_JMP: {
        uint64_t address = READ_ADDRESS(address);
        ip = &vm->fn->code[address];

        DISPATCH();
    }

    DO_OP_POP: {
        POP();

        DISPATCH();
    }

    DO_OP_GET_GLOBAL: {
        int64 address = READ_ADDRESS(address);
        RuntimeValue value = Global_Get(global, address).value;
        PUSH(value);

        DISPATCH();
    }

    DO_OP_SET_GLOBAL: {
        int64 index = READ_ADDRESS(index);
        RuntimeValue value = PEEK();
        Global_Set(global, index, &value);

        DISPATCH();
    }

    DO_OP_GET_LOCAL: {
        uint64 address = READ_ADDRESS(address);
        // if(address < 0 ){ 
        //     VM_Exception("Invalid variable index.");
        // }

        PUSH(vm->bp[address]);

        DISPATCH();
    }

    DO_OP_SET_LOCAL: {
        int64 index = READ_ADDRESS(index);
        RuntimeValue value = PEEK();
        vm->bp[index] = value;

        DISPATCH();
    }

    DO_OP_SCOPE_EXIT: {
        uint64 count = READ_ADDRESS(count);

        if(count > 0)
        {
            // Move the result above the vars that is getting popped
            // TODO: Borde bara vara så här för ett block som returerar

            //*(vm->sp - count) = VM_Stack_Peek(vm, 0);

            // Pop back to before scope
            sp -= (count); //  - 1
        }

        DISPATCH();
    }

    DO_OP_CALL: {
        uint64_t arg_count = READ_ADDRESS(arg_count);
        RuntimeValue fnValue = POP();


        if(IS_OBJ(fnValue) && AS_C_OBJ(fnValue)->objectType == ObjectType_NativeFunction){
            NativeFunctionObject fn = AS_NATIVE_FUNCTION(fnValue);

            RuntimeValue args[arg_count];

            for (size_t i = 0; i < arg_count; i++)
            {
                args[i] = POP();
            }

            RuntimeValue (*fun_ptr)() = fn.func_ptr; // Function pointer
            RuntimeValue res = (*fun_ptr)(arg_count, &args); // Invoke
            
            PUSH(res); // Push the result
        }
        else
        {
            FunctionObject* fn = (FunctionObject*)(AS_C_OBJ(fnValue));

            // Save execution context, restored on OP_RETURN
            Frame fr = {
                .bp = vm->bp,
                .fn = vm->fn,
                .ra = ip
            };
            *vm->csp = fr;
            vm->csp++;

            // Set function on vm
            vm->fn = fn;

            // set base pointer (frame) to the call
            vm->bp = sp - arg_count;

            // Jump to the function code
            ip = &fn->code[0];
        }

        DISPATCH();
    }

    DO_OP_RETURN: {
        uint64 count = READ_ADDRESS(count);

        if(count > 0)
        {
            // Move the result above the vars that is getting popped
            // TODO: Borde bara vara så här för ett block som returerar

            *(sp - count) = PEEK();

            // Pop back to before scope
            sp -= (count - 1); //  
        }

        // Restore frame
        vm->csp--;
        ip = vm->csp->ra;
        vm->bp = vm->csp->bp;
        vm->fn = vm->csp->fn;

        DISPATCH();
    }

    DO_OP_GET_MEMBER: {
        RuntimeValue instanceVal = POP();
        TypeInstanceObject instance = AS_TYPEINSTANCE(instanceVal);
        int64 memberIndex = READ_ADDRESS(memberIndex);

        RuntimeValue memberValue = Member_Get(&instance, memberIndex).value;
        PUSH(memberValue);

        DISPATCH();
    }
}

#pragma endregion

#pragma region VIRTUAL_MACHINE_HELPER


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