#pragma once

CodeObject  Compile(Statement* statement, Global* global);
size_t      Get_Offset(CodeObject* co);
size_t      Numeric_Const_Index(CodeObject* co, float64 value);
void        Gen(CodeObject* co, Statement* statement, Global* global);
void        Emit(CodeObject* co, uint8_t code);
void        Write_Address_At_Offset(CodeObject* co, size_t offset, uint64_t value);
// void        Write_Byte_At_Offset(CodeObject* co, size_t offset, uint8_t value);
void        Emit64(CodeObject* co, uint64_t value);
int64       String_Const_Index(CodeObject* co, BufferString* string);

CodeObject Compile(Statement* statement, Global* global){
    int64 compile_begin = timestamp();
    CodeObject co = AS_CODE(Alloc_Code("main", 4));

    // TODO: Need a growing array for this
    co.code = malloc(sizeof(uint8_t) * 100000000); // TODO: DANGER! Handle memory! 100 000 000, Crashes if too many instructions
    co.constants = malloc(sizeof(RuntimeValue) * 100); // TODO: DANGER! Handle memory! 100 000 000, Crashes if too many constants

    // Setup null, true and false consts. Compiler assumes this index order!
    co.constants[co.constants_last++] = RUNTIME_NULL();
    co.constants[co.constants_last++] = BOOLEAN(true);
    co.constants[co.constants_last++] = BOOLEAN(false);


    Gen(&co, statement, global);

    Emit(&co, OP_HALT);

    int64 compile_end = timestamp();
    printf("Compiling: %d ms\n", compile_end/1000-compile_begin/1000);

    return co;
}

void Gen(CodeObject* co, Statement* statement, Global* global){
    switch (statement->type)
    {
        case AST_BinaryExpression:{
            BinaryExpression expression = *(BinaryExpression*)statement;

            Gen(co, (Statement*)expression.left, global);
            Gen(co, (Statement*)expression.right, global);

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
        
        case AST_ComparisonExpression: {
            BinaryExpression expression = *(BinaryExpression*)statement;

            Gen(co, (Statement*)expression.left, global);
            Gen(co, (Statement*)expression.right, global);

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

        case AST_IfStatement: {
            IfStatement expression = *(IfStatement*)statement;

            Gen(co, (Statement*)expression.test, global); // Emit test
            Emit(co, OP_JMP_IF_FALSE); 

            Emit64(co, 0); // 64 bit for alternate address
            size_t else_jmp_address = Get_Offset(co) - 8; // 8 bytes for 64 bit

            Gen(co, (Statement*)expression.consequent, global); // Emit consequent
            Emit(co, OP_JMP); 

            Emit64(co, 0); // 64 bit for end address
            size_t end_address = Get_Offset(co) - 8; // 8 bytes for 64 bit

            // Patch else branch address
            size_t else_branch_address = Get_Offset(co);
            Write_Address_At_Offset(co, else_jmp_address, else_branch_address);

            if(expression.alternate != NULL){
                // Emit alternate if present
                Gen(co, (Statement*)expression.alternate, global); 
            }
            else{
                // TODO: NULL if no alternate branch
                // Emit(co, OP_CONST);
                // size_t index = Numeric_Const_Index(co, -999);
                // Emit64(co, index);
            }

            // Patch end of "if" address
            size_t end_branch_address = Get_Offset(co);
            Write_Address_At_Offset(co, end_address, end_branch_address);

            break;
        }

        case AST_WhileStatement: {
            WhileStatement expression = *(WhileStatement*)statement;

            size_t loop_start_address = Get_Offset(co);

            Gen(co, (Statement*)expression.test, global); // Emit test

            Emit(co, OP_JMP_IF_FALSE); 
            size_t loop_end_jmp_address = Get_Offset(co);
            Emit64(co, 0);

            Gen(co, (Statement*)expression.body, global); // Emit block

            Emit(co, OP_JMP); 
            size_t end_address = Get_Offset(co);
            Emit64(co, loop_start_address);

            // Patch end
            size_t end_branch_address = Get_Offset(co);
            Write_Address_At_Offset(co, loop_end_jmp_address, end_branch_address);

            break;
        }

        case AST_FunctionDeclaration: {
            // TODO: IMPLEMENT
            break;
        }

        case AST_BlockStatement:{
            BlockStatement blockStatement = *(BlockStatement*)statement;

            // Scope begin
            co->scope_level++;

            SSNode* current_node = blockStatement.body->first;
            while (current_node != NULL)
            {
                Statement* test = ((Statement*)current_node->value);

                Gen(co, current_node->value, global);

                // Pop if not last last (last is return value)
                bool is_local_declaration = 
                    ((Statement*)current_node->value)->type == AST_VariableDeclaration && co->scope_level > 0;

                if(!is_local_declaration){ // current_node->next != NULL && 
                    Emit(co, OP_POP);
                }

                current_node = current_node->next;
            }

            // Scope exit
            if(co->scope_level > 0){
                uint64 vars_to_pop = 0;
                while (co->locals_size > 0 && co->locals[co->locals_size - 1].scope_level == co->scope_level)
                {
                    vars_to_pop++;
                    co->locals_size--;
                }
                
             
                Emit(co, OP_SCOPE_EXIT);
                Emit64(co, vars_to_pop);
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

            size_t index = String_Const_Index(co, &expression.value);

            Emit(co, OP_CONST);
            Emit64(co, index);

            break;
        }

        case AST_Identifier: {
            Identifier identifier = *(Identifier*)statement;

            if(strncmp(identifier.name.start,"true", identifier.name.length) == 0){
                Emit(co, OP_CONST);
                Emit64(co, 1);
            }
            else if(strncmp(identifier.name.start,"false", identifier.name.length) == 0){
                Emit(co, OP_CONST);
                Emit64(co, 2);
            }
            else if(strncmp(identifier.name.start,"null", identifier.name.length) == 0){
                Emit(co, OP_CONST);
                Emit64(co, 0);
            }
            else{

                // Handle scoped variables
                int64 local_index = Local_GetIndexBufferString(co, &identifier.name);
                if(local_index != -1){
                    Emit(co, OP_GET_LOCAL);
                    Emit64(co, local_index);
                }
                else{
                    int64 global_index = Global_GetIndexBufferString(global, &identifier.name);

                    if(global_index == -1){
                        printf("\033[0;31mCompiler: Reference error \033[0m\n");
                        exit(0);
                    }

                    Emit(co, OP_GET_GLOBAL);
                    Emit64(co, global_index);
                }
            }
            break;
        }

        case AST_VariableDeclaration: {
            VariableDeclaration variableDeclaration = *(VariableDeclaration*)statement;
            if(variableDeclaration.value != NULL){
                Gen(co, (Statement*)variableDeclaration.value, global);
            }
            else{
                // Emit null if no value
                Emit(co, OP_CONST);
                Emit64(co, 0);
            }

            if(co->scope_level == 1) // If global scope
            { 
                Global_Define(global, &variableDeclaration.name); // This should return index directly
                int64 index = Global_GetIndexBufferString(global, &variableDeclaration.name);

                Emit(co, OP_SET_GLOBAL);
                Emit64(co, index);
            }
            else // Handle scoped variables
            {
                Local_Define(co, &variableDeclaration.name); // This should return index directly
                int64 index = Local_GetIndexBufferString(co, &variableDeclaration.name);

                Emit(co, OP_SET_LOCAL);
                Emit64(co, index);
            }

            break;
        }

        case AST_AssignmentExpression: {
            AssignmentExpression assignmentExpression = *(AssignmentExpression*)statement; 
            Identifier* identifier = (Identifier*)assignmentExpression.assignee; // TODO: Handle expressions

            // Emit value
            Gen(co, (Statement*)assignmentExpression.value, global);

            // 1. Locals
            int64 local_index = Local_GetIndexBufferString(co, &identifier->name);
            if(local_index != -1){
                Emit(co, OP_SET_LOCAL);
                Emit64(co, local_index);
            }
            else{
                // 2. Globals
                int64 global_index = Global_GetIndexBufferString(global, &identifier->name);

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

            // Emit function
            Gen(co, (Statement*)callExpression.callee, global);

            // Valitade arity
            
            // TODO: IMMPORTANT! Read back 64 bit. 
            uint8_t address = co->code[co->code_last - 8]; // Read back function global address
            NativeFunctionObject native_fn = AS_NATIVE_FUNCTION(Global_Get(global, address).value);

            if(native_fn.arity != callExpression.args->count){
                printf("\033[0;31mCompiler: Reference error. Arity mismatch. \033[0m\n");
                exit(0);
            }

            //RuntimeValue asd = co->code[co->code_last - 1];

            SSNode* cursor = callExpression.args->first;

            while (cursor != NULL)
            {
                Gen(co, (Statement*)cursor->value, global);
                cursor = cursor->next;
            }

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

int64 String_Const_Index(CodeObject* co, BufferString* string){
    for (size_t i = 0; i < co->constants_last + 1; i++)
    {
        if(co->constants[i].type != ValueType_Object){
            continue;
        }

        StringObject* strObj = (StringObject*)co->constants[i].object;
        uint64 current_length = strlen(strObj->string);

        if(current_length == string->length && strncmp(string->start, strObj->string, string->length) == 0){
            return i;
        }
    }

    co->constants[co->constants_last] = Alloc_String_From_BufferString(string);

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
        case OP_SCOPE_EXIT: return "SCOPE_EXIT:";
        default: return "NOT IMPLEMENTED";
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

void Disassemble(CodeObject* co, Global* global){
    printf("\n------------------ MAIN DISASSEMBLY ------------------\n\n");

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
            // printf("(%s)", Global_Get(global, args).name);
            offset += 8;
        }

        if(opcode == OP_SET_LOCAL){
            printf("%-7u", args);
            // printf("(%s)", Global_Get(global, args).name);
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

#pragma endregion
