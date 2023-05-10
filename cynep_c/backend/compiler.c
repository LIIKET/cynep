#pragma once

size_t      Get_Offset(FunctionObject* co);
size_t      Numeric_Const_Index(FunctionObject* co, float64 value);
size_t      locals_in_scope(FunctionObject* co);
int64_t     String_Const_Index(FunctionObject* co, char* string);
void        compile(AstNode* statement, Program* global);
void        generate(FunctionObject* co, AstNode* statement, Program* global);
void        emit_opcode(FunctionObject* co, uint8_t code);
void        Write_Address_At_Offset(FunctionObject* co, size_t offset, uint64_t value);
void        emit_64(FunctionObject* co, uint64_t value);
void        emit_return(FunctionObject* co, Program* global, uint64_t vars_declared_in_scope);
bool        is_global_scope(FunctionObject* co);

RuntimeValue Create_CodeObjectValue(char* name, size_t arity, Program* program){
    RuntimeValue value = Alloc_Function(name, arity);
    FunctionObject* co = &AS_FUNCTION(value);

    program_add_global(program, "null", NULL_VAL);
    program_add_global(program, "true", TRUE_VAL);
    program_add_global(program, "false", FALSE_VAL);

    if(strcmp(name, "main") == 0){
        program->main_function = co;
    }

    array_push(program->functions, co);

    return value;
}

void compile(AstNode* statement, Program* program)
{
    int64 compile_begin = timestamp();

    generate(NULL, statement, program);

    int64 compile_end = timestamp();
    printf("Compiling: %d ms\n", compile_end/1000-compile_begin/1000);
}

void generate(FunctionObject* co, AstNode* statement, Program* program){
    switch (statement->type)
    {
        case AST_BinaryExpression:{
            BinaryExpression expression = *(BinaryExpression*)statement;

            generate(co, (AstNode*)expression.left, program);
            generate(co, (AstNode*)expression.right, program);

            if(expression.operator[0] == '+'){
                emit_opcode(co, OP_ADD);
            }
            else if(expression.operator[0] == '-'){
                emit_opcode(co, OP_SUB);
            }
            else if(expression.operator[0] == '*'){
                emit_opcode(co, OP_MUL);
            }
            else if(expression.operator[0] == '/'){
                emit_opcode(co, OP_DIV);
            }
            break;
        }
        
        case AST_ReturnStatement: {
            ReturnStatement expression = *(ReturnStatement*)statement;

            generate(co, (AstNode*)expression.value, program);

            // Since we quit the whole function we exit scope with all locals in function
            // TODO: This should probably be all locals in current scope or lower?
            uint64_t vars_declared_in_scope_count = array_length(co->locals);
            
            // if((vars_declared_in_scope_count > 0 || co->arity > 0) && global->main_fn != co){
            //     Emit(co, OP_SCOPE_EXIT);
            //     Emit64(co, vars_declared_in_scope_count);
            // }

            // If we are returning a value we need to pop the expression result as well
            uint64_t pop_expr_result = expression.value ? 1 : 0; 

            emit_return(co, program, vars_declared_in_scope_count + pop_expr_result);

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

            generate(co, (AstNode*)expression.object, program);

            // Läs tillbaka senast emittade och kolla typ?
            // ! IMPORTANT! Read back 64 bit. 
            uint8_t address = co->code[array_length(co->code) - 8]; // Read back address
            uint8_t code = co->code[array_length(co->code) - 9]; // läs tillbaka opcode

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

            int64 typeinfo_index = Global_GetIndex(program, "player");
            GlobalVar typeinfovar = Global_Get(program, typeinfo_index);
            TypeInfoObject type_info = AS_TYPEINFO(typeinfovar.value);

            int64 member_index = Member_GetIndex(&type_info, member.name);

            emit_opcode(co, OP_GET_MEMBER);
            emit_64(co, member_index);

            break;
        }
        
        case AST_ComparisonExpression: {
            BinaryExpression expression = *(BinaryExpression*)statement;

            generate(co, (AstNode*)expression.left, program);
            generate(co, (AstNode*)expression.right, program);

            if(expression.operator[0] == '>'
            && expression.operator[1] == NULL_CHAR){
                emit_opcode(co, OP_CMP);
                emit_opcode(co, OP_CMP_GT);
            }
            else if(expression.operator[0] == '<'
            && expression.operator[1] == NULL_CHAR){
                emit_opcode(co, OP_CMP);
                emit_opcode(co, OP_CMP_LT);
            }
            else if(expression.operator[0] == '=' 
                 && expression.operator[1] == '='){
                emit_opcode(co, OP_CMP);
                emit_opcode(co, OP_CMP_EQ);
            }
            else if(expression.operator[0] == '>' 
                 && expression.operator[1] == '='){
                emit_opcode(co, OP_CMP);
                emit_opcode(co, OP_CMP_GE);
            }
            else if(expression.operator[0] == '<' 
                 && expression.operator[1] == '='){
                emit_opcode(co, OP_CMP);
                emit_opcode(co, OP_CMP_LE);
            }

            else if(expression.operator[0] == '!' 
                 && expression.operator[1] == '='){
                emit_opcode(co, OP_CMP);
                emit_opcode(co, OP_CMP_NE);
            }
            break;
        }

        case AST_TypeDefinition: {
            TypeDeclaration typeDeclaration = *(TypeDeclaration*)statement;

            RuntimeValue typeInfoValue = Alloc_TypeInfo(&typeDeclaration);
            TypeInfoObject* typeInfo = (TypeInfoObject*)AS_C_OBJ(typeInfoValue);

            MemberInfo asd = typeInfo->members[0];

            program_add_global(program, typeInfo->name, typeInfoValue);

            break;
        }

        case AST_IfStatement: {
            IfStatement expression = *(IfStatement*)statement;

            generate(co, (AstNode*)expression.test, program); // Emit test
            emit_opcode(co, OP_JMP_IF_FALSE); 

            emit_64(co, 0); // 64 bit for alternate address
            size_t else_jmp_address = Get_Offset(co) - 8; // 8 bytes for 64 bit

            generate(co, (AstNode*)expression.consequent, program); // Emit consequent

            emit_opcode(co, OP_JMP); 
            emit_64(co, 0); // 64 bit for end address
            size_t end_address = Get_Offset(co) - 8; // 8 bytes for 64 bit

            // Patch else branch address
            size_t else_branch_address = Get_Offset(co);
            Write_Address_At_Offset(co, else_jmp_address, else_branch_address);

            if(expression.alternate != NULL){
                // Emit alternate if present
                generate(co, (AstNode*)expression.alternate, program); 
            }

            // Patch end of "if" address
            size_t end_branch_address = Get_Offset(co);
            Write_Address_At_Offset(co, end_address, end_branch_address);

            break;
        }

        case AST_WhileStatement: {
            WhileStatement expression = *(WhileStatement*)statement;

            size_t loop_start_address = Get_Offset(co);

            generate(co, (AstNode*)expression.test, program); // Emit test

            emit_opcode(co, OP_JMP_IF_FALSE); 
            size_t loop_end_jmp_address = Get_Offset(co);
            emit_64(co, 0);

            generate(co, (AstNode*)expression.body, program); // Emit block

            emit_opcode(co, OP_JMP); 
            size_t end_address = Get_Offset(co);
            emit_64(co, loop_start_address);

            // Patch end
            size_t end_branch_address = Get_Offset(co);
            Write_Address_At_Offset(co, loop_end_jmp_address, end_branch_address);

            break;
        }

        case AST_FunctionDeclaration: {
            FunctionDeclaration functionDeclaration = *(FunctionDeclaration*)statement;

            char* name = functionDeclaration.name;
            size_t arity = functionDeclaration.args->count;

            RuntimeValue coValue = Create_CodeObjectValue(name, arity, program);
            FunctionObject* new_co = &AS_FUNCTION(coValue);
            
            program_add_global(program, new_co->name, coValue);

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
            generate(new_co, functionBody, program);

            // Here goes cleanup if we add functions without block as body

            // This is for implicit return
            // ! Do not emit return if previous instruction was explicit return
            // TODO: Pass number of top level var declarations here (maybe not? They should be popped by scope exit)
            emit_return(new_co, program, 0);

            break;
        }

        case AST_BlockStatement:{
            BlockStatement blockStatement = *(BlockStatement*)statement;

            // Scope begin
            uint8_t saved_scope = 0;
            if(!is_global_scope(co)){
                saved_scope = co->scope_level;
                co->scope_level++;
            }

            ListNode* current_node = blockStatement.body->first;
            while (current_node != NULL)
            {
                AstNode* current_astNode = ((AstNode*)current_node->value);

                generate(co, current_node->value, program);

                if(current_astNode->type == AST_AssignmentExpression){
                    emit_opcode(co, OP_POP);
                }

                current_node = current_node->next;
            }

            // Scope exit
            if(!is_global_scope(co)){

                // We need to pop all vars declared in this scope from the stack.
                // TODO: Do not emit this if previous instruction was an explicit return
                uint64 vars_declared_in_scope_count = locals_in_scope(co);

                array_popn(co->locals, vars_declared_in_scope_count);
                
                
                if(vars_declared_in_scope_count > 0 || co->arity > 0){
                    emit_opcode(co, OP_SCOPE_EXIT);
                    emit_64(co, vars_declared_in_scope_count);
                }

                co->scope_level = saved_scope;
            }

            break;
        }

        case AST_NumericLiteral: {
            NumericLiteral expression = *(NumericLiteral*)statement;

            size_t index = Numeric_Const_Index(co, expression.value);

            emit_opcode(co, OP_CONST);
            emit_64(co, index);

            break;
        }

        case AST_StringLiteral: {
            StringLiteral expression = *(StringLiteral*)statement;

            size_t index = String_Const_Index(co, expression.value);

            emit_opcode(co, OP_CONST);
            emit_64(co, index);

            break;
        }

        case AST_Identifier: {
            Identifier identifier = *(Identifier*)statement;

            // Handle scoped variables
            int64 local_index = Local_GetIndex(co, identifier.name);
            if(local_index != -1){
                emit_opcode(co, OP_GET_LOCAL);
                emit_64(co, local_index);
            }
            else{
                // No local, try global
                int64 global_index = Global_GetIndex(program, identifier.name);

                if(global_index == -1){
                    printf("\033[0;31mCompiler: Reference error \033[0m\n");
                    exit(0);
                }

                emit_opcode(co, OP_GET_GLOBAL);
                emit_64(co, global_index);
            }
            break;
        }

        case AST_VariableDeclaration: {
            VariableDeclaration variableDeclaration = *(VariableDeclaration*)statement;

            if(is_global_scope(co))
            { 
                program_define_global(program, variableDeclaration.name); // This should return index directly
                // TODO: We need to set global value here. Needs to be numericliteral or stringliteral.
            }
            else{
                if(variableDeclaration.value != NULL){
                    generate(co, (AstNode*)variableDeclaration.value, program);
                }
                else{
                    // Emit null if no value
                    int64 global_index = Global_GetIndex(program, "null");
                    emit_opcode(co, OP_GET_GLOBAL);
                    emit_64(co, global_index);
                }

                Local_Define(co, variableDeclaration.name); // This should return index directly
                int64 index = Local_GetIndex(co, variableDeclaration.name);

                emit_opcode(co, OP_SET_LOCAL);
                emit_64(co, index);
            }

            break;
        }

        case AST_AssignmentExpression: {
            AssignmentExpression assignmentExpression = *(AssignmentExpression*)statement; 
            Identifier* identifier = (Identifier*)assignmentExpression.assignee; // TODO: Handle member expressions

            // Emit value
            generate(co, (AstNode*)assignmentExpression.value, program);

            // 1. Locals
            int64 local_index = Local_GetIndex(co, identifier->name);
            if(local_index != -1){
                emit_opcode(co, OP_SET_LOCAL);
                emit_64(co, local_index);
            }
            else{
                // 2. Globals
                int64 global_index = Global_GetIndex(program, identifier->name);

                if(global_index == -1){
                    printf("\033[0;31mCompiler: Reference error \033[0m\n");
                    exit(0);
                }

                emit_opcode(co, OP_SET_GLOBAL);
                emit_64(co, global_index);
            }

            break;
        }

        case AST_CallExpression:{
            CallExpression callExpression = *(CallExpression*)statement; 


            // uint8_t address = co->code[co->code_last - 8]; // Read back function global address

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
                generate(co, (AstNode*)cursor->value, program);
                cursor = cursor->next;
            }

            // Emit function
            generate(co, (AstNode*)callExpression.callee, program);

            emit_opcode(co, OP_CALL);
            emit_64(co, callExpression.args->count);
            
            break;
        }

        default: {
            printf("\033[0;31mCompiler error: Unknown AST node \033[0m\n");
            exit(0);
            break;
        }

    }
}

bool is_global_scope(FunctionObject* co){
    return co == NULL;
}

size_t locals_in_scope(FunctionObject* co){
    size_t vars_declared_in_scope = 0;

    for (int i = array_length(co->locals) - 1; i >= 0; i--)
    {
        if(co->locals[i].scope_level == co->scope_level){
            vars_declared_in_scope++;
        }
        else{
            break;
        }
    }

    return vars_declared_in_scope;
}

void emit_return(FunctionObject* co, Program* global, uint64_t vars_declared_in_scope){
    if(co == global->main_function){
        emit_opcode(co, OP_HALT);
    }
    else{
        emit_opcode(co, OP_RETURN);
        emit_64(co, vars_declared_in_scope);
    }
}

size_t Numeric_Const_Index(FunctionObject* co, double value){
    for (size_t i = 0; i < array_length(co->constants); i++)
    {
        if(!IS_NUMBER(co->constants[i])){
            continue;
        }
        if(AS_C_DOUBLE(co->constants[i]) == value){
            return i;
        }
    }

    array_push(co->constants, NUMBER_VAL(value));

    return array_length(co->constants) - 1; 
}

int64 String_Const_Index(FunctionObject* co, char* string){
    for (size_t i = 0; i < array_length(co->constants); i++)
    {
        if(!IS_OBJ(co->constants[i])){
            continue;
        }

        StringObject* strObj = (StringObject*)AS_C_OBJ(co->constants[i]);

        if(strcmp(string, strObj->string) == 0){
            return i;
        }
    }

    array_push(co->constants, Alloc_String(string));

    return array_length(co->constants) - 1;
}

size_t Get_Offset(FunctionObject* co){
    return array_length(co->code);
}

// void Write_Byte_At_Offset(CodeObject* co, size_t offset, uint8_t value){
//     co->code[offset] = value;
// }

// Writes 64 bit
void Write_Address_At_Offset(FunctionObject* co, size_t offset, uint64_t value){
    memcpy(&co->code[offset], &value, sizeof( uint64_t ));
}

void emit_opcode(FunctionObject* co, uint8_t code){
    array_push(co->code, code);
}

void emit_64(FunctionObject* co, uint64_t value){
    uint8_t* loc = arraddnptr(co->code, 8);
    memcpy(loc, &value, sizeof( uint64_t ));
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

void Disassemble(Program* global){

    for (size_t i = 0; i < array_length(global->functions); i++)
    {
        FunctionObject* co = global->functions[i];

    printf("\n------------------ %s DISASSEMBLY ------------------\n\n", co->name);

size_t offset = 0;
    while(offset < array_length(co->code)){
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

        if(opcode == OP_RETURN){
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
