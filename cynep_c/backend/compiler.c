#pragma once

void Gen(CodeObject* co, Statement* statement);
void Emit(CodeObject* co, uint8_t code);
CodeObject Compile(Statement* statement);

CodeObject Compile(Statement* statement){
    CodeObject co = AS_CODE(ALLOC_CODE("main", 4));

    // TODO: Need a growing array for this
    co.code = malloc(sizeof(uint8_t) * 100);
    co.constants = malloc(sizeof(Value) * 100);


    Gen(&co, statement);
        Emit(&co, OP_HALT);

    return co;
}

void Gen(CodeObject* co, Statement* statement){
    switch (statement->type)
    {
        case AST_BinaryExpression:{
            BinaryExpression expression = *(BinaryExpression*)statement;
            
            switch (expression.operator[0])
            {
                case '+':{
                    Gen(co, (Statement*)expression.left);
                    Gen(co, (Statement*)expression.right);
                    Emit(co, OP_ADD);
                    break;
                }
                case '-':{
                    Gen(co, (Statement*)expression.left);
                    Gen(co, (Statement*)expression.right);
                    Emit(co, OP_SUB);
                    break;
                }
                case '*':{
                    Gen(co, (Statement*)expression.left);
                    Gen(co, (Statement*)expression.right);
                    Emit(co, OP_MUL);
                    break;
                }
                case '/':{
                    Gen(co, (Statement*)expression.left);
                    Gen(co, (Statement*)expression.right);
                    Emit(co, OP_DIV);
                    break;
                }
                default:
                    break;
            }

            break;
        }
        case AST_NumericLiteral: {
            NumericLiteral expression = *(NumericLiteral*)statement;
            co->constants[co->constants_last] = NUMBER(expression.value);
            Emit(co, OP_CONST);
            Emit(co, co->constants_last);
            co->constants_last++;
            break;
        }
    default:
        break;
    }
}



void Emit(CodeObject* co, uint8_t code){
    co->code[co->code_last] = code;
    co->code_last++;
}