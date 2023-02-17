#pragma once

void Gen(CodeObject* co, Statement* statement);
void Emit(CodeObject* co, uint8_t code);
CodeObject Compile(Statement* statement);
size_t Get_Offset(CodeObject* co);
void Write_Address_At_Offset(CodeObject* co, size_t offset, uint16_t value);
void Write_Byte_At_Offset(CodeObject* co, size_t offset, uint8_t value);
void Emit16(CodeObject* co, uint16_t value);

CodeObject Compile(Statement* statement){
    CodeObject co = AS_CODE(ALLOC_CODE("main", 4));

    // TODO: Need a growing array for this
    co.code = malloc(sizeof(uint8_t) * 100000000); // KRASCHAR OM FÖR MYCKET KOD
    co.constants = malloc(sizeof(RuntimeValue) * 100000000); // KRASCHAR OM FÖR MÅNGA KONSTANTER (värden i kod)

    // SSNode* current_node = statement->block_statement.body->first;
    // while (current_node != NULL)
    // {
    //     Gen(&co, statement);
    //     current_node = current_node->next;
    // }
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

            if(expression.operator[0] == '>'){
                Emit(co, OP_CMP);
                Emit(co, OP_CMP_GREATER);
            }
            else if(expression.operator[0] == '<'){
                Emit(co, OP_CMP);
                Emit(co, OP_CMP_LOWER);
            }
            else if(expression.operator[0] == '=' 
                 && expression.operator[0] == '='){
                Emit(co, OP_CMP);
                Emit(co, OP_CMP_EQUALS);
            }
            else if(expression.operator[0] == '>' 
                 && expression.operator[0] == '='){
                Emit(co, OP_CMP);
                Emit(co, OP_CMP_GREATER_EQUALS);
            }
            else if(expression.operator[0] == '<' 
                 && expression.operator[0] == '='){
                Emit(co, OP_CMP);
                Emit(co, OP_CMP_LOWER_EQUALS);
            }

            else if(expression.operator[0] == '!' 
                 && expression.operator[0] == '='){
                Emit(co, OP_CMP);
                Emit(co, OP_CMP_NOT_EQUALS);
            }
            break;
        }
        case AST_IfStatement: {
            IfStatement expression = *(IfStatement*)statement;

            Gen(co, (Statement*)expression.test); // Emit test
            Emit(co, OP_JMP_IF_FALSE); 

            Emit(co, 0); // Two bytes for alternate address
            Emit(co, 0);
            size_t else_jmp_address = Get_Offset(co) - 2;

            Gen(co, (Statement*)expression.consequent); // Emit consequent
            Emit(co, OP_JMP); 

            Emit(co, 0); // Two bytes for end address
            Emit(co, 0);
            size_t end_address = Get_Offset(co) - 2;

            // Patch else branch address
            size_t else_branch_address = Get_Offset(co);
            Write_Address_At_Offset(co, else_jmp_address, else_branch_address);

            if(expression.alternate != NULL){
                Gen(co, (Statement*)expression.alternate); // Emit alternate if present
            }
            // else{
            //     // TODO: NULL if no alternate branch
            //     co->constants[co->constants_last] = NUMBER(-999);
            //     Emit(co, OP_CONST);
            //     Emit(co, co->constants_last);
            //     co->constants_last++;
            // }

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
                }
            break;
        }
        case AST_NumericLiteral: {
            NumericLiteral expression = *(NumericLiteral*)statement;
            co->constants[co->constants_last] = NUMBER(expression.value);
            Emit(co, OP_CONST);
            Emit16(co, co->constants_last);


            //TEST
            // Emit(co, 0); // Two bytes for end address
            // Emit(co, 0);
            // size_t end_address = Get_Offset(co) - 2;
            // Write_Address_At_Offset(co, end_address, co->constants_last);

            co->constants_last++;
            break;
        }
        case AST_Identifier: {
            Identifier identifier = *(Identifier*)statement;
            if(strncmp (identifier.name.start,"true", identifier.name.length) == 0){
                co->constants[co->constants_last] = BOOLEAN(true);
                Emit(co, OP_CONST);
                Emit16(co, co->constants_last);
                
                //TEST
                // Emit(co, 0); // Two bytes for end address
                // Emit(co, 0);
                // size_t end_address = Get_Offset(co) - 2;
                // Write_Address_At_Offset(co, end_address, co->constants_last);

                co->constants_last++;
            }
            else if(strncmp (identifier.name.start,"false", identifier.name.length) == 0){
                co->constants[co->constants_last] = BOOLEAN(false);
                Emit(co, OP_CONST);
                Emit16(co, co->constants_last);
                //TEST
                // Emit(co, 0); // Two bytes for end address
                // Emit(co, 0);
                // size_t end_address = Get_Offset(co) - 2;
                // Write_Address_At_Offset(co, end_address, co->constants_last);

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



size_t Get_Offset(CodeObject* co){
    return co->code_last;
}

void Write_Byte_At_Offset(CodeObject* co, size_t offset, uint8_t value){
    co->code[offset] = value;
}

void Write_Address_At_Offset(CodeObject* co, size_t offset, uint16_t value){ // uint16 for two byte address
    Write_Byte_At_Offset(co, offset, (value >> 8) & 0xff);
    Write_Byte_At_Offset(co, offset + 1, (value) & 0xff);
}

void Emit(CodeObject* co, uint8_t code){
    co->code[co->code_last] = code;
    co->code_last++;
}

void Emit16(CodeObject* co, uint16_t value){
    uint8_t byte1 = (value >> 8) & 0xff;
    uint8_t byte2 = value & 0xff;
    co->code[co->code_last] = byte1;
    co->code_last++;
    co->code[co->code_last] = byte2;
    co->code_last++;
}