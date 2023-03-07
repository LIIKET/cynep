#pragma once

void  Compile(AstNode* statement, Global* global);
size_t      Get_Offset(CodeObject* co);
size_t      Numeric_Const_Index(CodeObject* co, float64 value);
void        Gen(CodeObject* co, AstNode* statement, Global* global);
void        Emit(CodeObject* co, uint8_t code);
void        Write_Address_At_Offset(CodeObject* co, size_t offset, uint64_t value);
// void     Write_Byte_At_Offset(CodeObject* co, size_t offset, uint8_t value);
void        Emit64(CodeObject* co, uint64_t value);
int64       String_Const_Index(CodeObject* co, char* string);
bool        Is_Global_Scope(CodeObject* co);

RuntimeValue Create_CodeObjectValue(char* name, size_t arity, Global* global){
    RuntimeValue value = Alloc_Code(name, arity);
    CodeObject* co = &AS_CODE(value);

    if(name[0] == 'm'){
        global->main_fn = co;
    }

    global->code_objects[global->code_objects_length] = co;
    global->code_objects_length++;

    return value;
}

void Compile(AstNode* statement, Global* global){

    global->code_objects= malloc(sizeof(void*) * 100); // TODO: DANGER! Handle memory! Crashes if too many functions

    int64 compile_begin = timestamp();

    Global_Add(global, "null", RUNTIME_NULL());
    Global_Add(global, "true", BOOLEAN(true));
    Global_Add(global, "false", BOOLEAN(false));

    Gen(NULL, statement, global);

    int64 compile_end = timestamp();
    printf("Compiling: %d ms\n", compile_end/1000-compile_begin/1000);
}

void Gen(CodeObject* co, AstNode* statement, Global* global){
    switch (statement->type)
    {
        case AST_BinaryExpression:{
            BinaryExpression expression = *(BinaryExpression*)statement;

            Gen(co, (AstNode*)expression.left, global);
            Gen(co, (AstNode*)expression.right, global);

            if(expression.operator[0] == '+'){
                Emit(co, OP_ADD);
            }
            else if(expression.operator[0] == '-'){
                Emit(co, OP_SUB);
            }
            else if(expression.operator[0] == '*'){
                Emit(co, OP_MUL);
            }
            else if(expression.operator[0] == '/'){
                Emit(co, OP_DIV);
            }
            break;
        }

        case AST_MemberExpression: {
            MemberExpression expression = *(MemberExpression*)statement;

            // Emit object first
            // Borde köra Gen på object sen
            // Identifier object = *(Identifier*)expression.object;
            // int64 local_index = Local_GetIndexBufferString(co, &object.name);

            // Emit(co, OP_GET_LOCAL);
            // Emit64(co, local_index);

            Gen(co, (AstNode*)expression.object, global);

            // Läs tillbaka senast emittade och kolla typ?
            // TODO: IMPORTANT! Read back 64 bit. 
            uint8_t address = co->code[co->code_last - 8]; // Read back address
            uint8_t code = co->code[co->code_last - 9]; // läs tillbaka opcode

            if(code == OP_GET_LOCAL){
                LocalVar assd = Local_Get(co, address); // Check type and use when getting typeinfo
                // Maybe it should have a typeinfo pointer on it
                int asd = 0;
            }
            else{
                printf("\033[0;31mCompiler: Implement global in member expression \033[0m\n");
                exit(0);
            }

            // Then member, borde alltid vara identifier?
            Identifier member = *(Identifier*)expression.member;

            int64 typeinfo_index = Global_GetIndex(global, "player");
            GlobalVar typeinfovar = Global_Get(global, typeinfo_index);
            TypeInfoObject type_info = AS_TYPEINFO(typeinfovar.value);

            int64 member_index = Member_GetIndex(&type_info, member.name);

            Emit(co, OP_GET_MEMBER);
            Emit64(co, member_index);

            break;
        }
        
        case AST_ComparisonExpression: {
            BinaryExpression expression = *(BinaryExpression*)statement;

            Gen(co, (AstNode*)expression.left, global);
            Gen(co, (AstNode*)expression.right, global);

            if(expression.operator[0] == '>'
            && expression.operator[1] == NULL_CHAR){
                Emit(co, OP_CMP);
                Emit(co, OP_CMP_GT);
            }
            else if(expression.operator[0] == '<'
            && expression.operator[1] == NULL_CHAR){
                Emit(co, OP_CMP);
                Emit(co, OP_CMP_LT);
            }
            else if(expression.operator[0] == '=' 
                 && expression.operator[1] == '='){
                Emit(co, OP_CMP);
                Emit(co, OP_CMP_EQ);
            }
            else if(expression.operator[0] == '>' 
                 && expression.operator[1] == '='){
                Emit(co, OP_CMP);
                Emit(co, OP_CMP_GE);
            }
            else if(expression.operator[0] == '<' 
                 && expression.operator[1] == '='){
                Emit(co, OP_CMP);
                Emit(co, OP_CMP_LE);
            }

            else if(expression.operator[0] == '!' 
                 && expression.operator[1] == '='){
                Emit(co, OP_CMP);
                Emit(co, OP_CMP_NE);
            }
            break;
        }

        case AST_TypeDefinition: {
            TypeDeclaration typeDeclaration = *(TypeDeclaration*)statement;

            RuntimeValue typeInfoValue = Alloc_TypeInfo(&typeDeclaration);
            TypeInfoObject* typeInfo = (TypeInfoObject*)typeInfoValue.object;

            MemberInfo asd = typeInfo->members[0];

            Global_Add(global, typeInfo->name, typeInfoValue);

            break;
        }

        case AST_IfStatement: {
            IfStatement expression = *(IfStatement*)statement;

            Gen(co, (AstNode*)expression.test, global); // Emit test
            Emit(co, OP_JMP_IF_FALSE); 

            Emit64(co, 0); // 64 bit for alternate address
            size_t else_jmp_address = Get_Offset(co) - 8; // 8 bytes for 64 bit

            Gen(co, (AstNode*)expression.consequent, global); // Emit consequent

            Emit(co, OP_JMP); 
            Emit64(co, 0); // 64 bit for end address
            size_t end_address = Get_Offset(co) - 8; // 8 bytes for 64 bit

            // Patch else branch address
            size_t else_branch_address = Get_Offset(co);
            Write_Address_At_Offset(co, else_jmp_address, else_branch_address);

            if(expression.alternate != NULL){
                // Emit alternate if present
                Gen(co, (AstNode*)expression.alternate, global); 
            }

            // Patch end of "if" address
            size_t end_branch_address = Get_Offset(co);
            Write_Address_At_Offset(co, end_address, end_branch_address);

            break;
        }

        case AST_WhileStatement: {
            WhileStatement expression = *(WhileStatement*)statement;

            size_t loop_start_address = Get_Offset(co);

            Gen(co, (AstNode*)expression.test, global); // Emit test

            Emit(co, OP_JMP_IF_FALSE); 
            size_t loop_end_jmp_address = Get_Offset(co);
            Emit64(co, 0);

            Gen(co, (AstNode*)expression.body, global); // Emit block

            Emit(co, OP_JMP); 
            size_t end_address = Get_Offset(co);
            Emit64(co, loop_start_address);

            // Patch end
            size_t end_branch_address = Get_Offset(co);
            Write_Address_At_Offset(co, loop_end_jmp_address, end_branch_address);

            break;
        }

        case AST_FunctionDeclaration: {
            FunctionDeclaration functionDeclaration = *(FunctionDeclaration*)statement;

            char* name = functionDeclaration.name;
            size_t arity = functionDeclaration.args->count;

            RuntimeValue coValue = Create_CodeObjectValue(name, arity, global);
            CodeObject* new_co = &AS_CODE(coValue);
            
            Global_Add(global, new_co->name, coValue);

            new_co->scope_level = 1;

            ListNode* current_node = functionDeclaration.args->first;
            while (current_node != NULL)
            {
                Identifier* identifier = ((Identifier*)current_node->value);
                Local_Define(new_co, identifier->name);
                current_node = current_node->next;
            }
            new_co->scope_level = 0;

            // Generate body
            AstNode* functionBody = (AstNode*)functionDeclaration.body;
            Gen(new_co, functionBody, global);

            // Here goes cleanup if we add functions without block as body

            if(new_co == global->main_fn){
                Emit(new_co, OP_HALT);
            }
            else{
                Emit(new_co, OP_RETURN);
            }

            break;
        }

        case AST_BlockStatement:{
            BlockStatement blockStatement = *(BlockStatement*)statement;

            // Scope begin
            if(!Is_Global_Scope(co)){
                co->scope_level++;
            }

            ListNode* current_node = blockStatement.body->first;
            while (current_node != NULL)
            {
                AstNode* current_astNode = ((AstNode*)current_node->value);

                Gen(co, current_node->value, global);

                if(current_astNode->type == AST_AssignmentExpression){
                    Emit(co, OP_POP);
                }

                current_node = current_node->next;
            }

            // Scope exit
            if(!Is_Global_Scope(co)){

                // We need to pop all vars declared in this scope from the stack.
                uint64 vars_declared_in_scope_count = 0;
                while (co->locals_size > 0 && co->locals[co->locals_size - 1].scope_level == co->scope_level)
                {
                    vars_declared_in_scope_count++;
                    co->locals_size--;
                }
                
                if(vars_declared_in_scope_count > 0 || co->arity > 0){
                    Emit(co, OP_SCOPE_EXIT);
                    Emit64(co, vars_declared_in_scope_count);
                }

                co->scope_level--;
            }

            break;
        }

        case AST_NumericLiteral: {
            NumericLiteral expression = *(NumericLiteral*)statement;

            size_t index = Numeric_Const_Index(co, expression.value);

            Emit(co, OP_CONST);
            Emit64(co, index);

            break;
        }

        case AST_StringLiteral: {
            StringLiteral expression = *(StringLiteral*)statement;

            size_t index = String_Const_Index(co, expression.value);

            Emit(co, OP_CONST);
            Emit64(co, index);

            break;
        }

        case AST_Identifier: {
            Identifier identifier = *(Identifier*)statement;

            // Handle scoped variables
            int64 local_index = Local_GetIndex(co, identifier.name);
            if(local_index != -1){
                Emit(co, OP_GET_LOCAL);
                Emit64(co, local_index);
            }
            else{
                // No local, try global
                int64 global_index = Global_GetIndex(global, identifier.name);

                if(global_index == -1){
                    printf("\033[0;31mCompiler: Reference error \033[0m\n");
                    exit(0);
                }

                Emit(co, OP_GET_GLOBAL);
                Emit64(co, global_index);
            }
            break;
        }

        case AST_VariableDeclaration: {
            VariableDeclaration variableDeclaration = *(VariableDeclaration*)statement;

            if(Is_Global_Scope(co))
            { 
                Global_Define(global, variableDeclaration.name); // This should return index directly
                // int64 index = Global_GetIndexBufferString(global, &variableDeclaration.name);

                // Emit(co, OP_SET_GLOBAL);
                // Emit64(co, index);
            }
            else{
                if(variableDeclaration.value != NULL){
                    Gen(co, (AstNode*)variableDeclaration.value, global);
                }
                else{
                    // Emit null if no value
                    int64 global_index = Global_GetIndex(global, "null");
                    Emit(co, OP_GET_GLOBAL);
                    Emit64(co, global_index);
                }

                Local_Define(co, variableDeclaration.name); // This should return index directly
                int64 index = Local_GetIndex(co, variableDeclaration.name);

                Emit(co, OP_SET_LOCAL);
                Emit64(co, index);
            }

            break;
        }

        case AST_AssignmentExpression: {
            AssignmentExpression assignmentExpression = *(AssignmentExpression*)statement; 
            Identifier* identifier = (Identifier*)assignmentExpression.assignee; // TODO: Handle expressions

            // Emit value
            Gen(co, (AstNode*)assignmentExpression.value, global);

            // 1. Locals
            int64 local_index = Local_GetIndex(co, identifier->name);
            if(local_index != -1){
                Emit(co, OP_SET_LOCAL);
                Emit64(co, local_index);
            }
            else{
                // 2. Globals
                int64 global_index = Global_GetIndex(global, identifier->name);

                if(global_index == -1){
                    printf("\033[0;31mCompiler: Reference error \033[0m\n");
                    exit(0);
                }

                Emit(co, OP_SET_GLOBAL);
                Emit64(co, global_index);
            }

            break;
        }

        case AST_CallExpression:{
            CallExpression callExpression = *(CallExpression*)statement; 



            // TODO: IMPORTANT! Read back 64 bit. 
            uint8_t address = co->code[co->code_last - 8]; // Read back function global address

            // TODO: IMPORTANT! Valitade arity for native and user defined functions. Example (native):
            /*
            NativeFunctionObject native_fn = AS_NATIVE_FUNCTION(Global_Get(global, address).value);

            if(native_fn.arity != callExpression.args->count){
                printf("\033[0;31mCompiler: Reference error. Arity mismatch. \033[0m\n");
                exit(0);
            }
            */

            ListNode* cursor = callExpression.args->first;
            while (cursor != NULL)
            {
                Gen(co, (AstNode*)cursor->value, global);
                cursor = cursor->next;
            }

            // Emit function
            Gen(co, (AstNode*)callExpression.callee, global);

            Emit(co, OP_CALL);
            Emit64(co, callExpression.args->count);
            
            break;
        }

        default: {
            printf("\033[0;31mCompiler error: Unknown AST node \033[0m\n");
            exit(0);
            break;
        }

    }
}

bool Is_Global_Scope(CodeObject* co){
    return co == NULL;
}

size_t Numeric_Const_Index(CodeObject* co, float64 value){
    for (size_t i = 0; i < co->constants_last + 1; i++)
    {
        if(co->constants[i].type != ValueType_Number){
            continue;
        }
        if(co->constants[i].number == value){
            return i;
        }
    }

    co->constants[co->constants_last] = NUMBER(value);

    return co->constants_last++; // Increments after return
}

int64 String_Const_Index(CodeObject* co, char* string){
    for (size_t i = 0; i < co->constants_last + 1; i++)
    {
        if(co->constants[i].type != ValueType_Object){
            continue;
        }

        StringObject* strObj = (StringObject*)co->constants[i].object;
        // uint64 current_length = strlen(strObj->string);

        if(strcmp(string, strObj->string) == 0){
            return i;
        }
    }

    co->constants[co->constants_last] = Alloc_String(string);
    return co->constants_last++; // Increments after return
}

size_t Get_Offset(CodeObject* co){
    return co->code_last;
}

// void Write_Byte_At_Offset(CodeObject* co, size_t offset, uint8_t value){
//     co->code[offset] = value;
// }

// Writes 64 bit
void Write_Address_At_Offset(CodeObject* co, size_t offset, uint64_t value){
    memcpy(&co->code[offset], &value, sizeof( uint64_t ));
}

void Emit(CodeObject* co, uint8_t code){
    co->code[co->code_last] = code;
    co->code_last++;
}

void Emit64(CodeObject* co, uint64_t value){
    memcpy(&co->code[co->code_last + 0], &value, sizeof( uint64_t ));
    co->code_last += 8;
}

#pragma region DISASSEMBLER

char* opcodeToString(uint8_t opcode){
    switch (opcode)
    {
        case OP_HALT: return "HALT";
        case OP_CONST: return "CONST";
        case OP_ADD: return "ADD";
        case OP_SUB: return "SUB";
        case OP_MUL: return "MUL";
        case OP_DIV: return "DIV";
        case OP_CMP: return "CMP";
        case OP_JMP_IF_FALSE: return "JMP_IF_FALSE";
        case OP_JMP: return "JMP";
        case OP_POP: return "POP";
        case OP_CALL: return "CALL";
        case OP_GET_GLOBAL: return "GET_GLOBAL";
        case OP_SET_GLOBAL: return "SET_GLOBAL";
        case OP_GET_LOCAL: return "GET_LOCAL";
        case OP_SET_LOCAL: return "SET_LOCAL";
        case OP_SCOPE_EXIT: return "SCOPE_EXIT";
        case OP_RETURN: return "RETURN";
        case OP_GET_MEMBER: return "GET_MEMBER";
        default: {
            return "NOT IMPLEMENTED";
        }
    }
}

char* cmpCodeToString(uint8_t cmpcode){
    switch (cmpcode)
    {
        case OP_CMP_GT: return "GT";
        case OP_CMP_LT: return "LT";
        case OP_CMP_EQ: return "EQ";
        case OP_CMP_GE: return "GE";
        case OP_CMP_LE: return "LE";
        case OP_CMP_NE: return "NE";
        default: return "NOT IMPLEMENTED";
    }
}

void Disassemble(Global* global){


    for (size_t i = 0; i < global->code_objects_length; i++)
    {
        CodeObject* co = global->code_objects[i];

    printf("\n------------------ %s DISASSEMBLY ------------------\n\n", co->name);

size_t offset = 0;
    while(offset < co->code_last){
        uint8_t opcode = co->code[offset];
        uint64_t args;

        memcpy(&args, &co->code[offset + 1], sizeof(uint64_t));

        uint8_t small_args = co->code[offset + 1];

        char* opcode_string = opcodeToString(opcode);

        printf("0x%04X", offset);
        printf("%-10s", "\t");
        printf("%-20s", opcode_string);

        if(opcode == OP_CONST){
            printf("0x%04X", args);
            printf(" (%s)", RuntimeValue_ToString(co->constants[args]));
            offset += 8;
        }

        if(opcode == OP_GET_MEMBER){
            printf("0x%04X", args);
            printf(" (%s)", RuntimeValue_ToString(co->constants[args]));
            offset += 8;
        }

        if(opcode == OP_CMP){
            printf("0x%01X", small_args);
            printf("%-4s", " ");
            printf("(%s)", cmpCodeToString(small_args));    
            offset += 1;
        }

        if(opcode == OP_GET_GLOBAL){
            printf("%-7u", args);
            printf("(%s)", Global_Get(global, args).name);
            offset += 8;
        }

        if(opcode == OP_SET_GLOBAL){
            printf("%-7u", args);
            printf("(%s)", Global_Get(global, args).name);
            offset += 8;
        }

        if(opcode == OP_GET_LOCAL){
            printf("%-7u", args);
            printf("(%s)", Local_Get(co, args).name);
            offset += 8;
        }

        if(opcode == OP_SET_LOCAL){
            printf("%-7u", args);
            printf("(%s)", Local_Get(co, args).name);
            offset += 8;
        }

        if(opcode == OP_SCOPE_EXIT){
            printf("%-7u", args);
            // printf("(%s)", Global_Get(global, args).name);
            offset += 8;
        }

        if(opcode == OP_JMP){
            printf("0x%04X", args);
            offset += 8;
        }

        if(opcode == OP_CALL){
            printf("%-7u", args);
            offset += 8;
        }

        if(opcode == OP_JMP_IF_FALSE){
            printf("0x%04X", args);
            offset += 8;
        }

        offset++;
        printf("\n");
    }

    printf("\n");


    }
    

    
}

#pragma endregion
