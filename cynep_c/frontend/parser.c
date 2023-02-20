#pragma once

bool  End_Of_File();
Token Current();
Token Consume();
Token ConsumeExpect(TokenType type, char* error);

Statement*              Seed_Memory();
Statement*              Parse_Statement();
Expression*             Parse_Expression();
VariableDeclaration*    Parse_VariableDeclaration();
TypeDeclaration*        Parse_TypeDeclaration();
Expression*             Parse_AssignmentExpression();
Expression*             Parse_PrimaryExpression();

Expression* Parse_ComparisonExpression();
Expression*             Parse_AdditiveExpression();
Expression*             Parse_MultiplicativeExpression();
IfStatement* Parse_IfStatement();
BlockStatement* Parse_BlockStatement();

//
// Globals
//

Token* _tokens = NULL;
size_t _current_index = 0;

size_t nodes_max = 10000000; 
size_t nodes_count = 0;
Statement* nodes = NULL;

size_t list_nodes_max = 10000000; 
size_t list_nodes_count = 0;
SSNode* list_nodes = NULL;

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

Statement* Seed_Memory()
{   
    // TODO: This wont fly. Pointers are getting fucked up.
    // Also, think about storing offsets instead of pointers for serialization purposes
    if(nodes_count == nodes_max){
        nodes_max *= 2;
        nodes = (Statement*)realloc(nodes, sizeof(Statement) * nodes_max);
    }

    return &nodes[nodes_count++];
}

SSNode* Seed_SSNode_Memory(){
    return &list_nodes[nodes_count++];
}

//
//  Parsing
//

Statement* Build_SyntaxTree(Token* tokens)
{
    int64 t1 = timestamp();

    _tokens = tokens;

    nodes = (Statement*)malloc(sizeof(Statement) * nodes_max);
    list_nodes = (SSNode*)malloc(sizeof(SSNode) * list_nodes_max);

    Statement* block = (Statement*)Create_BlockStatement(Seed_Memory(), (SSList*)Seed_SSNode_Memory());

    while(!End_Of_File()){
        Statement* statement = Parse_Statement();
        SSNode* node = SSNode_Create(Seed_SSNode_Memory(),statement);
        SSList_Append(block->block_statement.body, node);
    }

    int64 t2 = timestamp();
    printf("Parsing: %d ms\n", t2/1000-t1/1000);

    // for (size_t i = 0; i < nodes_count; i++)
    // {
    //     printf("%s\n", nodes[i].name);
    // }

    return block;
}

Statement* Parse_Statement()
{
    switch (Current().type)
    {
        case Token_Let: {
            return (Statement*)Parse_VariableDeclaration();
        }
        case Token_Type: {
            return (Statement*)Parse_TypeDeclaration();
        }
        case Token_If: {
            return (Statement*)Parse_IfStatement();
        }
        default: {
            return (Statement*)Parse_Expression();
        }
    }
}

IfStatement* Parse_IfStatement()
{
    Token if_token = Consume();
    ConsumeExpect(Token_OpenParen, "If statement should be followed by an open parenthesis.");
    ComparisonExpression* test = (ComparisonExpression*)Parse_ComparisonExpression();
    ConsumeExpect(Token_CloseParen, "Missing close parenthesis in if statement.");
    
    Statement* consequtive;
    if(Current().type == Token_OpenBrace){
        consequtive = (Statement*)Parse_BlockStatement();
    }
    else{
        // TODO: Parse single expression. Fuck this for now.
    }

    Statement* alternate = NULL;
    if(Current().type == Token_Else){
        Consume();
        if(Current().type == Token_OpenBrace){
            alternate = (Statement*)Parse_BlockStatement();
        }
        else{
            // TODO: Parse single expression. Fuck this for now.
        }
    }

    return Create_IfStatement((Statement*)Seed_Memory(), test, consequtive, alternate);
}

BlockStatement* Parse_BlockStatement(){
    Consume(); // Open brace
    BlockStatement* block = Create_BlockStatement(Seed_Memory(), (SSList*)Seed_SSNode_Memory());

    while(Current().type != Token_CloseBrace){
        Statement* statement = Parse_Statement();
        SSNode* node = SSNode_Create(Seed_SSNode_Memory(),statement);
        SSList_Append(block->body, node);
    }
    ConsumeExpect(Token_CloseBrace, "Missing close brace in block.");

    return block;
}

VariableDeclaration* Parse_VariableDeclaration()
{
    // var {identifier} : {type} = {expression};
    // var {identifier};
    Token let_token = Consume();
    Token identifier = ConsumeExpect(Token_Identifier, "Let keyword should be followed by an identifier.");

    if(Current().type == Token_Semicolon){
        Consume();
        return Create_VariableDeclaration((Statement*)Seed_Memory(), identifier.string_value, NULL);
    }
    else{
        ConsumeExpect(Token_Assignment, "Identifier in var declaration should be followed by an equals token.");
        Expression* expression = Parse_Expression();
        ConsumeExpect(Token_Semicolon, "Variable declaration must end with semicolon.");
        return Create_VariableDeclaration((Statement*)Seed_Memory(), identifier.string_value, expression);
    }
}

TypeDeclaration* Parse_TypeDeclaration()
{
    Token type_token = Consume();
    Token identifier = ConsumeExpect(Token_Identifier, "Type keyword should be followed by an identifier.");
    ConsumeExpect(Token_Assignment, "Error in type declaration");
    ConsumeExpect(Token_OpenBrace, "Error in type declaration");

    TypeDeclaration* type_declaration = Create_TypeDeclaration(Seed_Memory(), (SSList*)Seed_SSNode_Memory() ,identifier.string_value);

    while(!End_Of_File() && Current().type != Token_CloseBrace){
        Token property_identifier = ConsumeExpect(Token_Identifier, "Error in type declaration");
        ConsumeExpect(Token_Semicolon, "Error in type declaration");
        PropertyDeclaration* property_declaration = Create_PropertyDeclaration(Seed_Memory(), property_identifier.string_value);

        SSNode* node = SSNode_Create(Seed_SSNode_Memory(),property_declaration);
        SSList_Append(type_declaration->properties, node);
    }

    ConsumeExpect(Token_CloseBrace, "Error in type declaration");

    return type_declaration;
}

Expression* Parse_Expression()
{
    return (Expression*)Parse_AssignmentExpression();
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

Expression* Parse_AssignmentExpression()
{
    Expression* left = Parse_ComparisonExpression();

    if(Current().type == Token_Assignment)
    {
        Consume();
        Expression* value = Parse_ComparisonExpression();
        ConsumeExpect(Token_Semicolon, "Variable assignment must end with semicolon.");
        return (Expression*)Create_AssignmentExpression(Seed_Memory(), left, value);
    }

    return left;
}

Expression* Parse_ComparisonExpression()
{
    Expression* left = Parse_AdditiveExpression();

    while (0 == strcmp(Current().operator_value,"==")
        || 0 == strcmp(Current().operator_value,"!=")  
        || 0 == strcmp(Current().operator_value,">" )
        || 0 == strcmp(Current().operator_value,">=") 
        || 0 == strcmp(Current().operator_value,"<" )
        || 0 == strcmp(Current().operator_value,"<="))
    {
        char* operator = Consume().operator_value;
        Expression* right = Parse_AdditiveExpression();
        left = (Expression*)Create_ComparisonExpression(Seed_Memory(), left, operator, right);

        // TODO: Should be possible to do type checking here
    }

    return left;
}

Expression* Parse_AdditiveExpression()
{
    Expression* left = Parse_MultiplicativeExpression();

    while (0 == strcmp(Current().operator_value,"+") || 0 == strcmp(Current().operator_value,"-"))
    {
        char* operator = Consume().operator_value;
        Expression* right = Parse_MultiplicativeExpression();
        left = (Expression*)Create_BinaryExpression(Seed_Memory(), left, operator, right);

        // TODO: Should be possible to do type checking here
    }

    return left;
}

Expression* Parse_MultiplicativeExpression()
{
    Expression* left = Parse_PrimaryExpression();

    while (0 == strcmp(Current().operator_value,"*") 
        || 0 == strcmp(Current().operator_value,"/") 
        || 0 == strcmp(Current().operator_value,"%"))
    {
        char* operator = Consume().operator_value;
        Expression* right = Parse_PrimaryExpression();
        left = (Expression*)Create_BinaryExpression(Seed_Memory(), left, operator, right);
    }

    return left;
}

Expression* Parse_PrimaryExpression()
{
    Token token = Current();

    switch (token.type) { 
        case Token_Identifier:
            {
                return (Expression*)Create_Identifier(Seed_Memory(), Consume().string_value);
            }
        case Token_Number:
            {
                return (Expression*)Create_NumericLiteral(Seed_Memory(), Consume().number_value); //atoi(Consume().value)
            }
        case Token_OpenParen:
            {
                Consume(); // Throw away open paren
                Expression* expression = Parse_Expression();
                ConsumeExpect(Token_CloseParen, "Error in primary expression"); // Throw away close paren

                return expression;
            }
        default:
            {
                //printf("Unexpected token in parser: \"%s\"\n", Current().value);
                exit(0);
            }
    }
}