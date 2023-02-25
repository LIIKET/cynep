#pragma once

bool  End_Of_File();
Token Current();
Token Consume();
Token ConsumeExpect(TokenType type, char* error);


Statement*              Parse_Statement(MemPool* pool);
Expression*             Parse_Expression(MemPool* pool);
VariableDeclaration*    Parse_VariableDeclaration(MemPool* pool);
TypeDeclaration*        Parse_TypeDeclaration(MemPool* pool);
Expression*             Parse_AssignmentExpression(MemPool* pool);
Expression*             Parse_PrimaryExpression(MemPool* pool);
Expression*             Parse_ComparisonExpression(MemPool* pool);
Expression*             Parse_AdditiveExpression(MemPool* pool);
Expression*             Parse_MultiplicativeExpression(MemPool* pool);
IfStatement*            Parse_IfStatement(MemPool* pool);
WhileStatement*         Parse_WhileStatement(MemPool* pool);
BlockStatement*         Parse_BlockStatement(MemPool* pool);

Expression* Parse_CallMemberExpression(MemPool* pool);
CallExpression* Parse_CallExpression(MemPool* pool, Expression* caller);
Expression* Parse_MemberExpression(MemPool* pool);
SSList* Parse_Args(MemPool* pool);
SSList* Parse_ArgumentsList(MemPool* pool, SSList* args);

//
// Globals
//

Token* _tokens = NULL;
size_t _current_index = 0;

//
//  Helpers
//

bool End_Of_File()
{
    return _tokens[_current_index].type == Token_EOF;
}

Token Current()
{
    return _tokens[_current_index];
}

Token Consume()
{
    Token token = _tokens[_current_index];
    _current_index++;
    return token;
}

Token ConsumeExpect(TokenType type, char* error)
{
    Token token = _tokens[_current_index];

    if(type != token.type){
        printf("Exprected %d, got %d. %s\n", type, token.type, error);
        exit(0);
    }

    _current_index++;
    return token;
}


//
//  Parsing
//

Statement* Build_SyntaxTree(Token* tokens)
{
    int64 t1 = timestamp();

    _tokens = tokens;

    MemPool* pool = MemPool_Make(sizeof(Statement) * 10000000);

    MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));
    MemPool_Record list_mem = MemPool_GetMem(pool, sizeof(SSList));

    BlockStatement* block = Create_BlockStatement(statement_mem.pointer, list_mem.pointer);

    while(!End_Of_File()){
        Statement* statement = Parse_Statement(pool);

        MemPool_Record list_node_mem = MemPool_GetMem(pool, sizeof(SSNode));

        SSNode* node = SSNode_Create(list_node_mem.pointer, statement);
        SSList_Append(block->body, node);
    }

    int64 t2 = timestamp();
    printf("Parsing: %d ms\n", t2/1000-t1/1000);

    // for (size_t i = 0; i < nodes_count; i++)
    // {
    //     printf("%s\n", nodes[i].name);
    // }

    return (Statement*)block;
}

Statement* Parse_Statement(MemPool* pool)
{
    switch (Current().type)
    {
        case Token_Let: {
            return (Statement*)Parse_VariableDeclaration(pool);
        }
        case Token_Type: {
            return (Statement*)Parse_TypeDeclaration(pool);
        }
        case Token_If: {
            return (Statement*)Parse_IfStatement(pool);
        }
        case Token_While: {
            return (Statement*)Parse_WhileStatement(pool);
        }
        default: {
            return (Statement*)Parse_Expression(pool);
        }
    }
}

IfStatement* Parse_IfStatement(MemPool* pool)
{
    Token if_token = Consume();
    ConsumeExpect(Token_OpenParen, "If statement should be followed by an open parenthesis.");
    ComparisonExpression* test = (ComparisonExpression*)Parse_ComparisonExpression(pool);
    ConsumeExpect(Token_CloseParen, "Missing close parenthesis in if statement.");
    
    Statement* consequtive;
    if(Current().type == Token_OpenBrace){
        consequtive = (Statement*)Parse_BlockStatement(pool);
    }
    else{
        // TODO: Parse single expression. Fuck this for now.
    }

    Statement* alternate = NULL;
    if(Current().type == Token_Else){
        Consume();
        if(Current().type == Token_OpenBrace){
            alternate = (Statement*)Parse_BlockStatement(pool);
        }
        else{
            // TODO: Parse single expression. Fuck this for now.
        }
    }

    MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));
    return Create_IfStatement(statement_mem.pointer, test, consequtive, alternate);
}

WhileStatement* Parse_WhileStatement(MemPool* pool)
{
    Token if_token = Consume();
    ConsumeExpect(Token_OpenParen, "While statement should be followed by an open parenthesis.");
    ComparisonExpression* test = (ComparisonExpression*)Parse_ComparisonExpression(pool);
    ConsumeExpect(Token_CloseParen, "Missing close parenthesis in while statement.");
    
    Statement* body;
    if(Current().type == Token_OpenBrace){
        body = (Statement*)Parse_BlockStatement(pool);
    }
    else{
        // TODO: Parse single expression. Fuck this for now.
    }

    MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));
    return Create_WhileStatement(statement_mem.pointer, test, body);
}

BlockStatement* Parse_BlockStatement(MemPool* pool){
    Consume(); // Open brace

    MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));
    MemPool_Record list_mem = MemPool_GetMem(pool, sizeof(SSList));

    BlockStatement* block = Create_BlockStatement(statement_mem.pointer, list_mem.pointer);

    while(Current().type != Token_CloseBrace){
        Statement* statement = Parse_Statement(pool);

        MemPool_Record list_node_mem = MemPool_GetMem(pool, sizeof(SSNode));
        SSNode* node = SSNode_Create(list_node_mem.pointer, statement);

        SSList_Append(block->body, node);
    }

    ConsumeExpect(Token_CloseBrace, "Missing close brace in block.");

    return block;
}

VariableDeclaration* Parse_VariableDeclaration(MemPool* pool)
{
    // var {identifier} : {type} = {expression};
    // var {identifier};
    Token let_token = Consume();
    Token identifier = ConsumeExpect(Token_Identifier, "Let keyword should be followed by an identifier.");

    if(Current().type == Token_Semicolon){
        Consume();

        MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));

        return Create_VariableDeclaration(statement_mem.pointer, identifier.string_value, NULL);
    }
    else{
        ConsumeExpect(Token_Assignment, "Identifier in var declaration should be followed by an equals token.");
        Expression* expression = Parse_Expression(pool);
        
        ConsumeExpect(Token_Semicolon, "Variable declaration must end with semicolon.");

        MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));
        
        return Create_VariableDeclaration(statement_mem.pointer, identifier.string_value, expression);
    }
}

TypeDeclaration* Parse_TypeDeclaration(MemPool* pool)
{
    Token type_token = Consume();
    Token identifier = ConsumeExpect(Token_Identifier, "Type keyword should be followed by an identifier.");
    ConsumeExpect(Token_Assignment, "Error in type declaration");
    ConsumeExpect(Token_OpenBrace, "Error in type declaration");

    MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));
    MemPool_Record list_mem = MemPool_GetMem(pool, sizeof(SSList));

    TypeDeclaration* type_declaration = Create_TypeDeclaration(statement_mem.pointer, list_mem.pointer, identifier.string_value);

    while(!End_Of_File() && Current().type != Token_CloseBrace){
        Token property_identifier = ConsumeExpect(Token_Identifier, "Error in type declaration");
        ConsumeExpect(Token_Semicolon, "Error in type declaration");

        MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));
        MemPool_Record list_node_mem = MemPool_GetMem(pool, sizeof(SSNode));

        PropertyDeclaration* property_declaration = Create_PropertyDeclaration(statement_mem.pointer, property_identifier.string_value);

        SSNode* node = SSNode_Create(list_node_mem.pointer, property_declaration);
        SSList_Append(type_declaration->properties, node);
    }

    ConsumeExpect(Token_CloseBrace, "Error in type declaration");

    return type_declaration;
}

Expression* Parse_Expression(MemPool* pool)
{
    return (Expression*)Parse_AssignmentExpression(pool);
}


// Orders of prescidence (highest last, just like execution order)

// AssignmentExpression         [X]
// Object
// LogicalExpression (and, or)                
// ComparisonExpression         [X]
// AdditiveExpression           [X]
// MiltiplicativeExpression     [X]
// UnaryExpression
// FunctionCall                 [/]
// MemberExpression             [X]
// PrimaryExpression            [X]

Expression* Parse_AssignmentExpression(MemPool* pool)
{
    Expression* left = Parse_ComparisonExpression(pool);

    if(Current().type == Token_Assignment)
    {
        Consume();
        Expression* value = Parse_ComparisonExpression(pool);
        ConsumeExpect(Token_Semicolon, "Variable assignment must end with semicolon.");

        MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));

        return (Expression*)Create_AssignmentExpression(statement_mem.pointer, left, value);
    }

    return left;
}

Expression* Parse_ComparisonExpression(MemPool* pool)
{
    Expression* left = Parse_AdditiveExpression(pool);

    while (0 == strncmp(Current().operator_value,"==", 2)
        || 0 == strncmp(Current().operator_value,"!=", 2)  
        || 0 == strncmp(Current().operator_value,">=", 2) 
        || 0 == strncmp(Current().operator_value,"<=", 2)
        || 0 == strncmp(Current().operator_value,">" , 2)
        || 0 == strncmp(Current().operator_value,"<" , 2))
    {
        char* operator = Consume().operator_value;
        Expression* right = Parse_AdditiveExpression(pool);

        MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));

        left = (Expression*)Create_ComparisonExpression(statement_mem.pointer, left, operator, right);

        // TODO: Should be possible to do type checking here
    }

    return left;
}

Expression* Parse_AdditiveExpression(MemPool* pool)
{
    Expression* left = Parse_MultiplicativeExpression(pool);

    while (0 == strcmp(Current().operator_value,"+") || 0 == strcmp(Current().operator_value,"-"))
    {
        char* operator = Consume().operator_value;
        Expression* right = Parse_MultiplicativeExpression(pool);

        MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));

        left = (Expression*)Create_BinaryExpression(statement_mem.pointer, left, operator, right);

        // TODO: Should be possible to do type checking here
    }

    return left;
}

Expression* Parse_MultiplicativeExpression(MemPool* pool)
{
    Expression* left = Parse_CallMemberExpression(pool);

    while (0 == strcmp(Current().operator_value,"*") 
        || 0 == strcmp(Current().operator_value,"/") 
        || 0 == strcmp(Current().operator_value,"%"))
    {
        char* operator = Consume().operator_value;
        Expression* right = Parse_CallMemberExpression(pool);

        MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));

        left = (Expression*)Create_BinaryExpression(statement_mem.pointer, left, operator, right);
    }

    return left;
}

//
//  CallExpression
//

Expression* Parse_CallMemberExpression(MemPool* pool){
    Expression* member = Parse_MemberExpression(pool);

    if (Current().type == Token_OpenParen)
    {
        return (Expression*)Parse_CallExpression(pool, member);
    }

    return member; 
}

CallExpression* Parse_CallExpression(MemPool* pool, Expression* caller){

    MemPool_Record memory = MemPool_GetMem(pool, sizeof(Statement));
    CallExpression* call_expr = Create_CallExpression(memory.pointer, caller, Parse_Args(pool));

    if(Current().type == Token_OpenParen){
        call_expr = Parse_CallExpression(pool, caller);
    }

    return call_expr;
}

SSList* Parse_Args(MemPool* pool)
{
    ConsumeExpect(Token_OpenParen, "Args must start with open paren.");
    MemPool_Record memory = MemPool_GetMem(pool, sizeof(SSList));
    SSList* args = SSList_Create(memory.pointer);  

    if(Current().type == Token_CloseParen)
    {
        int a = 0;
    }
    else
    {
        args = Parse_ArgumentsList(pool, args);
    }

    ConsumeExpect(Token_CloseParen, "Args must end with close paren.");

    return args;
}

SSList* Parse_ArgumentsList(MemPool* pool, SSList* args)
{
    MemPool_Record node_mem = MemPool_GetMem(pool, sizeof(SSNode));
    Expression* expr = Parse_AssignmentExpression(pool);
    SSNode* node = SSNode_Create(node_mem.pointer, expr);
    SSList_Append(args, node);

    while (Current().type == Token_Comma)
    {
        Consume();

        MemPool_Record node_mem = MemPool_GetMem(pool, sizeof(SSNode));
        Expression* expr = Parse_AssignmentExpression(pool);
        SSNode* node = SSNode_Create(node_mem.pointer, expr);
        SSList_Append(args, node);
    }

    return args;
}

Expression* Parse_MemberExpression(MemPool* pool)
{
    Expression* obj = Parse_PrimaryExpression(pool);

    while (Current().type == Token_Dot)
    {
        Consume(); // Eat dot
        Expression* member = Parse_PrimaryExpression(pool);

        if (member->statement.type != AST_Identifier)
        {
            printf("Dot operator requires identifier on right hand");
            exit(0);
        }

        MemPool_Record mem = MemPool_GetMem(pool, sizeof(Statement));
        obj = (Expression*)Create_MemberExpression(mem.pointer, obj, (Identifier*)member);
    }

    return obj;
}

//
//  End CallExpression
//

Expression* Parse_PrimaryExpression(MemPool* pool)
{
    Token token = Current();

    switch (token.type) { 
        case Token_Identifier:
            {
                MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));
                return (Expression*)Create_Identifier(statement_mem.pointer, Consume().string_value);
            }
        case Token_Number:
            {
                MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));
                return (Expression*)Create_NumericLiteral(statement_mem.pointer, Consume().number_value); //atoi(Consume().value)
            }
        case Token_String:
            {     
                MemPool_Record statement_mem = MemPool_GetMem(pool, sizeof(Statement));
                return (Expression*)Create_StringLiteral(statement_mem.pointer, Consume().string_value); //atoi(Consume().value)
            }
        case Token_OpenParen:
            {
                Consume(); // Throw away open paren
                Expression* expression = Parse_Expression(pool);
                ConsumeExpect(Token_CloseParen, "Error in primary expression"); // Throw away close paren

                return expression;
            }
        default:
            {
                printf("Unexpected token in parser.");
                 printf("Unexpected token in parser: \"%s\"\n", Current().string_value);
                exit(0);
            }
    }
}