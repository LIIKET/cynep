#pragma once

bool  End_Of_File();
Token Current();
Token Consume();
Token ConsumeExpect(TokenType type, char* error);

AstNode*              Parse_Statement(Arena* arena);
Expression*             Parse_Expression(Arena* arena);
VariableDeclaration*    Parse_VariableDeclaration(Arena* arena);
TypeDeclaration*        Parse_TypeDeclaration(Arena* arena);
Expression*             Parse_AssignmentExpression(Arena* arena);
Expression*             Parse_PrimaryExpression(Arena* arena);
Expression*             Parse_ComparisonExpression(Arena* arena);
Expression*             Parse_AdditiveExpression(Arena* arena);
Expression*             Parse_MultiplicativeExpression(Arena* arena);
IfStatement*            Parse_IfStatement(Arena* arena);
WhileStatement*         Parse_WhileStatement(Arena* arena);
BlockStatement*         Parse_BlockStatement(Arena* arena);

Expression* Parse_CallMemberExpression(Arena* arena);
CallExpression* Parse_CallExpression(Arena* arena, Expression* caller);
Expression* Parse_MemberExpression(Arena* arena);
List* Parse_Args(Arena* arena);
List* Parse_ArgumentsList(Arena* arena, List* args);
FunctionDeclaration* Parse_FunctionDeclaration(Arena* arena);

//
// Globals
//

Token* _tokens = NULL;
size_t _current_index = 0;

//
//  Helpers
//

bool End_Of_File(){
    return _tokens[_current_index].type == Token_EOF;
}

Token Current(){
    return _tokens[_current_index];
}

Token Consume(){
    Token token = _tokens[_current_index];
    _current_index++;
    return token;
}

Token ConsumeExpect(TokenType type, char* error){
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

AstNode* Build_SyntaxTree(Token* tokens)
{
    int64 t1 = timestamp();

    _tokens = tokens;

    Arena* arena = arena_create(500 * sizeof(AstNode));

    BlockStatement* block = Create_BlockStatement(arena_alloc(arena, sizeof(AstNode)), arena_alloc(arena, sizeof(List)));

    while(!End_Of_File()){
        AstNode* statement = Parse_Statement(arena);

        ListNode* node = listNode_create(arena_alloc(arena, sizeof(ListNode)), statement);
        list_append(block->body, node);
    }

    int64 t2 = timestamp();
    printf("Parsing: %d ms\n", t2/1000-t1/1000);

    // for (size_t i = 0; i < nodes_count; i++)
    // {
    //     printf("%s\n", nodes[i].name);
    // }

    return (AstNode*)block;
}

AstNode* Parse_Statement(Arena* arena)
{
    switch (Current().type)
    {
        case Token_Let: {
            return (AstNode*)Parse_VariableDeclaration(arena);
        }
        case Token_Type: {
            return (AstNode*)Parse_TypeDeclaration(arena);
        }
        case Token_Func: {
            return (AstNode*)Parse_FunctionDeclaration(arena);
        }
        case Token_If: {
            return (AstNode*)Parse_IfStatement(arena);
        }
        case Token_While: {
            return (AstNode*)Parse_WhileStatement(arena);
        }
        default: {
            return (AstNode*)Parse_Expression(arena);
        }
    }
}

FunctionDeclaration* Parse_FunctionDeclaration(Arena* arena)
{
    Token func_token = Consume();
    Token func_name = ConsumeExpect(Token_Identifier, "Function should be followed by an identifier");

    List* args = list_create(arena_alloc(arena, sizeof(List)));

    ConsumeExpect(Token_OpenParen, "Function declaration should be followed by an open parenthesis.");

    if(Current().type == Token_Identifier){
        do {
            if(Current().type == Token_Comma){
                Consume();
            }

            Token identifierTok = ConsumeExpect(Token_Identifier, "func argument should be an identifier.");
            Identifier* identifier = Create_Identifier(arena_alloc(arena, sizeof(AstNode)), identifierTok.string);
            ListNode* node = listNode_create(arena_alloc(arena, sizeof(AstNode)), identifier);
            list_append(args, node);
        } while((Current().type == Token_Comma));
    }

    ConsumeExpect(Token_CloseParen, "Missing close parenthesis in function declaration.");
    
    BlockStatement* body;
    if(Current().type == Token_OpenBrace){
        body = Parse_BlockStatement(arena);
    }

    return Create_FunctionDeclaration(arena_alloc(arena, sizeof(AstNode)), func_name.string, args, body);
}

IfStatement* Parse_IfStatement(Arena* arena)
{
    Token if_token = Consume();
    ConsumeExpect(Token_OpenParen, "If statement should be followed by an open parenthesis.");
    ComparisonExpression* test = (ComparisonExpression*)Parse_ComparisonExpression(arena);
    ConsumeExpect(Token_CloseParen, "Missing close parenthesis in if statement.");
    
    AstNode* consequtive;
    if(Current().type == Token_OpenBrace){
        consequtive = (AstNode*)Parse_BlockStatement(arena);
    }
    else{
        // TODO: Parse single expression. Fuck this for now.
    }

    AstNode* alternate = NULL;
    if(Current().type == Token_Else){
        Consume();
        if(Current().type == Token_OpenBrace){
            alternate = (AstNode*)Parse_BlockStatement(arena);
        }
        else{
            // TODO: Parse single expression. Fuck this for now.
        }
    }

    return Create_IfStatement(arena_alloc(arena, sizeof(AstNode)), test, consequtive, alternate);
}

WhileStatement* Parse_WhileStatement(Arena* arena)
{
    Token if_token = Consume();
    ConsumeExpect(Token_OpenParen, "While statement should be followed by an open parenthesis.");
    ComparisonExpression* test = (ComparisonExpression*)Parse_ComparisonExpression(arena);
    ConsumeExpect(Token_CloseParen, "Missing close parenthesis in while statement.");
    
    AstNode* body;
    if(Current().type == Token_OpenBrace){
        body = (AstNode*)Parse_BlockStatement(arena);
    }
    else{
        // TODO: Parse single expression. Fuck this for now.
    }

    return Create_WhileStatement(arena_alloc(arena, sizeof(AstNode)), test, body);
}

BlockStatement* Parse_BlockStatement(Arena* arena){
    Consume(); // Open brace

    BlockStatement* block = Create_BlockStatement(arena_alloc(arena, sizeof(AstNode)), arena_alloc(arena, sizeof(List)));

    while(Current().type != Token_CloseBrace){
        AstNode* statement = Parse_Statement(arena);
        ListNode* node = listNode_create(arena_alloc(arena, sizeof(ListNode)), statement);

        list_append(block->body, node);
    }

    ConsumeExpect(Token_CloseBrace, "Missing close brace in block.");

    return block;
}

VariableDeclaration* Parse_VariableDeclaration(Arena* arena)
{
    // var {identifier} : {type} = {expression};
    // var {identifier};
    Token let_token = Consume();
    Token identifier = ConsumeExpect(Token_Identifier, "Let keyword should be followed by an identifier.");

    if(Current().type == Token_Semicolon){
        Consume();

        return Create_VariableDeclaration(arena_alloc(arena, sizeof(AstNode)), identifier.string, NULL);
    }
    else{
        ConsumeExpect(Token_Assignment, "Identifier in var declaration should be followed by an equals token.");
        Expression* expression = Parse_Expression(arena);
        
        ConsumeExpect(Token_Semicolon, "Variable declaration must end with semicolon.");
        
        return Create_VariableDeclaration(arena_alloc(arena, sizeof(AstNode)), identifier.string, expression);
    }
}

TypeDeclaration* Parse_TypeDeclaration(Arena* arena)
{
    Token type_token = Consume();
    Token identifier = ConsumeExpect(Token_Identifier, "Type keyword should be followed by an identifier.");
    ConsumeExpect(Token_Assignment, "Error in type declaration");
    ConsumeExpect(Token_OpenBrace, "Error in type declaration");

    TypeDeclaration* type_declaration = Create_TypeDeclaration(arena_alloc(arena, sizeof(AstNode)), arena_alloc(arena, sizeof(List)), identifier.string);

    while(!End_Of_File() && Current().type != Token_CloseBrace){
        Token property_identifier = ConsumeExpect(Token_Identifier, "Error in type declaration");
        ConsumeExpect(Token_Semicolon, "Error in type declaration");

        PropertyDeclaration* property_declaration = Create_PropertyDeclaration(arena_alloc(arena, sizeof(AstNode)), property_identifier.string);

        ListNode* node = listNode_create(arena_alloc(arena, sizeof(AstNode)), property_declaration);
        list_append(type_declaration->properties, node);
    }

    ConsumeExpect(Token_CloseBrace, "Error in type declaration");

    return type_declaration;
}

Expression* Parse_Expression(Arena* arena)
{
    return (Expression*)Parse_AssignmentExpression(arena);
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

Expression* Parse_AssignmentExpression(Arena* arena)
{
    Expression* left = Parse_ComparisonExpression(arena);

    if(Current().type == Token_Assignment)
    {
        Consume();
        Expression* value = Parse_ComparisonExpression(arena);
        ConsumeExpect(Token_Semicolon, "Variable assignment must end with semicolon.");

        return (Expression*)Create_AssignmentExpression(arena_alloc(arena, sizeof(AstNode)), left, value);
    }

    return left;
}

Expression* Parse_ComparisonExpression(Arena* arena)
{
    Expression* left = Parse_AdditiveExpression(arena);

    while (0 == strncmp(Current().operator_value,"==", 2)
        || 0 == strncmp(Current().operator_value,"!=", 2)  
        || 0 == strncmp(Current().operator_value,">=", 2) 
        || 0 == strncmp(Current().operator_value,"<=", 2)
        || 0 == strncmp(Current().operator_value,">" , 2)
        || 0 == strncmp(Current().operator_value,"<" , 2))
    {
        char* operator = Consume().operator_value;
        Expression* right = Parse_AdditiveExpression(arena);

        left = (Expression*)Create_ComparisonExpression(arena_alloc(arena, sizeof(AstNode)), left, operator, right);

        // TODO: Should be possible to do type checking here
    }

    return left;
}

Expression* Parse_AdditiveExpression(Arena* arena)
{
    Expression* left = Parse_MultiplicativeExpression(arena);

    while (0 == strcmp(Current().operator_value,"+") || 0 == strcmp(Current().operator_value,"-"))
    {
        char* operator = Consume().operator_value;
        Expression* right = Parse_MultiplicativeExpression(arena);

        left = (Expression*)Create_BinaryExpression(arena_alloc(arena, sizeof(AstNode)), left, operator, right);

        // TODO: Should be possible to do type checking here
    }

    return left;
}

Expression* Parse_MultiplicativeExpression(Arena* arena)
{
    Expression* left = Parse_CallMemberExpression(arena);

    while (0 == strcmp(Current().operator_value,"*") 
        || 0 == strcmp(Current().operator_value,"/") 
        || 0 == strcmp(Current().operator_value,"%"))
    {
        char* operator = Consume().operator_value;
        Expression* right = Parse_CallMemberExpression(arena);

        left = (Expression*)Create_BinaryExpression(arena_alloc(arena, sizeof(AstNode)), left, operator, right);
    }

    return left;
}

//
//  CallExpression
//

Expression* Parse_CallMemberExpression(Arena* arena){
    Expression* member = Parse_MemberExpression(arena);

    if (Current().type == Token_OpenParen)
    {
        return (Expression*)Parse_CallExpression(arena, member);
    }

    return member; 
}

CallExpression* Parse_CallExpression(Arena* arena, Expression* caller){

    CallExpression* call_expr = Create_CallExpression(arena_alloc(arena, sizeof(AstNode)), caller, Parse_Args(arena));

    if(Current().type == Token_OpenParen){
        call_expr = Parse_CallExpression(arena, caller);
    }

    return call_expr;
}

List* Parse_Args(Arena* arena)
{
    ConsumeExpect(Token_OpenParen, "Args must start with open paren.");
    List* args = list_create(arena_alloc(arena, sizeof(List)));  

    if(Current().type == Token_CloseParen)
    {
        int a = 0;
    }
    else
    {
        args = Parse_ArgumentsList(arena, args);
    }

    ConsumeExpect(Token_CloseParen, "Args must end with close paren.");

    return args;
}

List* Parse_ArgumentsList(Arena* arena, List* args)
{
    Expression* expr = Parse_AssignmentExpression(arena);
    ListNode* node = listNode_create(arena_alloc(arena, sizeof(ListNode)), expr);
    list_append(args, node);

    while (Current().type == Token_Comma)
    {
        Consume();

        Expression* expr = Parse_AssignmentExpression(arena);
        ListNode* node = listNode_create(arena_alloc(arena, sizeof(ListNode)), expr);
        list_append(args, node);
    }

    return args;
}

Expression* Parse_MemberExpression(Arena* arena)
{
    Expression* obj = Parse_PrimaryExpression(arena);

    while (Current().type == Token_Dot)
    {
        Consume(); // Eat dot
        Expression* member = Parse_PrimaryExpression(arena);

        if (member->statement.type != AST_Identifier)
        {
            printf("Dot operator requires identifier on right hand");
            exit(0);
        }

        obj = (Expression*)Create_MemberExpression(arena_alloc(arena, sizeof(AstNode)), obj, (Identifier*)member);
    }

    return obj;
}

//
//  End CallExpression
//

Expression* Parse_PrimaryExpression(Arena* arena)
{
    Token token = Current();

    switch (token.type) { 
        case Token_Identifier:
            {
                return (Expression*)Create_Identifier(arena_alloc(arena, sizeof(AstNode)), Consume().string);
            }
        case Token_Number:
            {
                return (Expression*)Create_NumericLiteral(arena_alloc(arena, sizeof(AstNode)), Consume().number_value); //atoi(Consume().value)
            }
        case Token_String:
            {     
                return (Expression*)Create_StringLiteral(arena_alloc(arena, sizeof(AstNode)), Consume().string); //atoi(Consume().value)
            }
        case Token_OpenParen:
            {
                Consume(); // Throw away open paren
                Expression* expression = Parse_Expression(arena);
                ConsumeExpect(Token_CloseParen, "Error in primary expression"); // Throw away close paren

                return expression;
            }
        default:
            {
                printf("Unexpected token in parser.");
                 printf("Unexpected token in parser: \"%s\"\n", Current().string);
                exit(0);
            }
    }
}