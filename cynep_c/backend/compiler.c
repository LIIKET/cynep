#pragma once

CodeObject Compile(Statement* statement);
size_t Get_Offset(CodeObject* co);
void Gen(CodeObject* co, Statement* statement);
void Emit(CodeObject* co, uint8_t code);
void Write_Address_At_Offset(CodeObject* co, size_t offset, uint64_t value);
void Write_Byte_At_Offset(CodeObject* co, size_t offset, uint8_t value);
void Emit64(CodeObject* co, uint64_t value);
size_t Numeric_Const_Index(CodeObject* co, float64 value);

CodeObject Compile(Statement* statement){
    CodeObject co = AS_CODE(ALLOC_CODE("main", 4));

    // TODO: Need a growing array for this
    co.code = malloc(sizeof(uint8_t) * 100000000); // 100 000 000, Crashes if too many instructions
    co.constants = malloc(sizeof(RuntimeValue) * 100000000); // 100 000 000, Crashes if too many constants

    Gen(&co, statement);
    
    Emit(&co, OP_HALT);

    return co;
}

void Gen(CodeObject* co, Statement* statement){
    switch (statement->type)
    {
        case AST_BinaryExpression:{
            BinaryExpression expression = *(BinaryExpression*)statement;

            Gen(co, (Statement*)expression.left);
            Gen(co, (Statement*)expression.right);

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

            Gen(co, (Statement*)expression.left);
            Gen(co, (Statement*)expression.right);

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

            Gen(co, (Statement*)expression.test); // Emit test
            Emit(co, OP_JMP_IF_FALSE); 

            Emit64(co, 0); // 64 bytes for alternate address
            size_t else_jmp_address = Get_Offset(co) - 8; // 8 bytes for 64 bit

            Gen(co, (Statement*)expression.consequent); // Emit consequent
            Emit(co, OP_JMP); 

            Emit64(co, 0); // 64 bytes for end address
            size_t end_address = Get_Offset(co) - 8; // 8 bytes for 64 bit

            // Patch else branch address
            size_t else_branch_address = Get_Offset(co);
            Write_Address_At_Offset(co, else_jmp_address, else_branch_address);

            if(expression.alternate != NULL){
                // Emit alternate if present
                Gen(co, (Statement*)expression.alternate); 
            }
            else{
                //TODO: NULL if no alternate branch
                // Emit(co, OP_CONST);
                // size_t index = Numeric_Const_Index(co, -999);
                // Emit64(co, index);
            }

            // Patch end of "if" address
            size_t end_branch_address = Get_Offset(co);
            Write_Address_At_Offset(co, end_address, end_branch_address);

            break;
        }
        case AST_BlockStatement:{
            BlockStatement st = *(BlockStatement*)statement;

            SSNode* current_node = statement->block_statement.body->first;
            while (current_node != NULL)
            {
                Gen(co, current_node->value);
                current_node = current_node->next;

                // Pop if last
                // if(current_node == NULL){
                //     Emit(co, OP_POP);
                // }
            }
            break;
        }
        case AST_NumericLiteral: {
            NumericLiteral expression = *(NumericLiteral*)statement;

            // co->constants[co->constants_last] = NUMBER(expression.value);
            // Emit(co, OP_CONST);
            // Emit64(co, co->constants_last);
            // co->constants_last++;

            //co->constants[co->constants_last] = NUMBER(expression.value);
            Emit(co, OP_CONST);
            size_t index = Numeric_Const_Index(co, expression.value);
            Emit64(co, index);

            break;
        }
        case AST_Identifier: {
            Identifier identifier = *(Identifier*)statement;
            if(strncmp(identifier.name.start,"true", identifier.name.length) == 0){
                co->constants[co->constants_last] = BOOLEAN(true);
                Emit(co, OP_CONST);
                Emit64(co, co->constants_last);

                co->constants_last++;
            }
            else if(strncmp(identifier.name.start,"false", identifier.name.length) == 0){
                co->constants[co->constants_last] = BOOLEAN(false);
                Emit(co, OP_CONST);
                Emit64(co, co->constants_last);

                co->constants_last++;
            }
            else{
                // TODO: Handle variables
            }
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
        if(co->constants[i].type != ValuteType_Number){
            continue;
        }
        if(co->constants[i].number == value){
            return i;
        }
    }

    co->constants[co->constants_last] = NUMBER(value);

    return co->constants_last++; // Increments after return
}

size_t Get_Offset(CodeObject* co){
    return co->code_last;
}

void Write_Byte_At_Offset(CodeObject* co, size_t offset, uint8_t value){
    co->code[offset] = value;
}

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

void Disassemble(CodeObject* co){
    size_t offset = 0;
    while(offset < co->code_last){
        uint8_t opcode = co->code[offset];
        uint64_t args = co->code[offset + 1];
        uint8_t small_args = co->code[offset + 1];

        char* opcode_string = opcodeToString(opcode);

        printf("0x%04X", offset);
        printf("%-10s", "\t");
        printf("%-20s", opcode_string);

        if(opcode == OP_CONST){
            printf("0x%04X", args);
            printf(" (%s)", RuntimeValueToString(co->constants[args]));
            offset += 8;
        }

        if(opcode == OP_CMP){
            printf("0x%01X", small_args);
            printf("%-4s", " ");
            printf("(%s)", cmpCodeToString(small_args));    
            offset += 1;
        }

        if(opcode == OP_JMP){
            printf("0x%04X", args);
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