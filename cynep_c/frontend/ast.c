#pragma once

typedef enum   NodeType NodeType;
typedef struct Statement Statement;
typedef struct BlockStatement BlockStatement;
typedef struct Identifier Identifier;
typedef struct NumericLiteral NumericLiteral;
typedef struct VariableDeclaration VariableDeclaration;
typedef struct PropertyDeclaration PropertyDeclaration;
typedef struct TypeDeclaration TypeDeclaration;
typedef struct CallExpression CallExpression;
typedef struct MemberExpression MemberExpression;
typedef struct AssignmentExpression AssignmentExpression;
typedef struct BinaryExpression BinaryExpression;
typedef struct BinaryExpression ComparisonExpression;
typedef struct Expression Expression;
typedef struct IfStatement IfStatement;

enum NodeType 
{
    // Statements
    AST_BlockStatement,
    AST_IfStatement,
    AST_VariableDeclaration,
    AST_TypeDefinition,
    AST_PropertyDeclaration,

    // Expression
    AST_AssignmentExpression,
    AST_BinaryExpression,
    AST_ComparisonExpression,
    AST_MemberExpression,
    AST_CallExpression,

    // Literals
    AST_NumericLiteral,
    AST_Identifier,
};

struct BlockStatement 
{
    SSList* body; // Statements
};

struct IfStatement
{
    ComparisonExpression* test;
    Statement* consequent; // Statements
    Statement* alternate; // Statements
};

struct Identifier 
{
    BufferString name;
};

struct NumericLiteral 
{
    int64 value;
};

struct VariableDeclaration 
{
    BufferString name;
    Expression* value;
};

struct PropertyDeclaration 
{
    BufferString name;
};

struct TypeDeclaration
{
    BufferString name;
    SSList* properties; // PropertyDeclarations
    // int64 properties_count;
};

struct CallExpression 
{
    Expression* callee;
    Expression* args[2];
    int64 args_count;
};

struct MemberExpression 
{
    Statement* object;
    Identifier* property;
};

struct AssignmentExpression 
{
    Expression* assignee;
    Expression* value;
};

struct BinaryExpression 
{
    char operator[2];
    Expression* left;
    Expression* right;
};

struct Statement 
{
    union
    {
        BlockStatement block_statement;
        IfStatement ifStatement;
        Identifier identifier;
        NumericLiteral numeric_literal;
        VariableDeclaration variable_declaration;
        PropertyDeclaration property_declaration;
        TypeDeclaration type_declaration;
        CallExpression call_expression;
        MemberExpression member_expression;
        AssignmentExpression assignment_expression;
        BinaryExpression binary_expression;
        BinaryExpression comparison_expression;
    }; 
    NodeType type;
};

struct Expression {
    Statement statement;
};

//
// Initializers
//

BlockStatement* Create_BlockStatement(Statement* node, SSList* body)
{
    node->type = AST_BlockStatement;
    node->block_statement.body = SSList_Create(body);

    return (BlockStatement*)node;
}

VariableDeclaration* Create_VariableDeclaration(Statement* memory, BufferString name, Expression* value)
{
    memory->type = AST_VariableDeclaration;
    memory->variable_declaration.value = value;

    memory->variable_declaration.name.start = name.start;
    memory->variable_declaration.name.length = name.length;

    return (VariableDeclaration*)memory;
}

TypeDeclaration* Create_TypeDeclaration(Statement* memory, SSList* member_list_memory, BufferString name)
{
    memory->type = AST_TypeDefinition;
    memory->type_declaration.properties = SSList_Create(member_list_memory);

    memory->type_declaration.name.start = name.start;
    memory->type_declaration.name.length = name.length;

    return (TypeDeclaration*)memory;
}

PropertyDeclaration* Create_PropertyDeclaration(Statement* memory, BufferString name)
{
    memory->type = AST_PropertyDeclaration;

    memory->property_declaration.name.start = name.start;
    memory->property_declaration.name.length = name.length;

    return (PropertyDeclaration*)memory;
}

CallExpression* Create_CallExpression(Statement* memory, Statement* calee)
{
    memory->type = AST_CallExpression;

    return (CallExpression*)memory;
}

MemberExpression* Create_MemberExpression(Statement* memory, Statement* object, Identifier* property)
{
    memory->type = AST_MemberExpression;
    memory->member_expression.object = object;
    memory->member_expression.property = property;

    return (MemberExpression*)memory;
}

AssignmentExpression* Create_AssignmentExpression(Statement* memory, Expression* assignee, Expression* value)
{
    memory->type = AST_AssignmentExpression;
    memory->assignment_expression.assignee = assignee;
    memory->assignment_expression.value = value;

    return (AssignmentExpression*)memory;
}

BinaryExpression* Create_BinaryExpression(Statement* memory, Expression* left, char* operatr, Expression* right)
{
    memory->type = AST_BinaryExpression;
    memory->binary_expression.left = left;
    memory->binary_expression.right = right;

    strcpy(memory->binary_expression.operator, operatr);

    return (BinaryExpression*)memory;
}

ComparisonExpression* Create_ComparisonExpression(Statement* memory, Expression* left, char* operatr, Expression* right)
{
    memory->type = AST_ComparisonExpression;
    memory->comparison_expression.left = left;
    memory->comparison_expression.right = right;

    strcpy(memory->comparison_expression.operator, operatr);

    return (ComparisonExpression*)memory;
}

IfStatement* Create_IfStatement(Statement* memory, ComparisonExpression* test, Statement* consequent)
{
    memory->type = AST_IfStatement;
    memory->ifStatement.test = test;
    memory->ifStatement.consequent = consequent;
    memory->ifStatement.alternate = NULL;

    return (IfStatement*)memory;
}

Identifier* Create_Identifier(Statement* memory, BufferString name)
{
    memory->type = AST_Identifier;
    memory->identifier.name.start = name.start;
    memory->identifier.name.length = name.length;

    return (Identifier*)memory;
}

NumericLiteral* Create_NumericLiteral(Statement* memory, int64 value)
{
    memory->type = AST_NumericLiteral;
    memory->numeric_literal.value = value;

    return (NumericLiteral*)memory;
}

//
// Visualizing
//
SSList* Get_Children(Statement* expression)
{
    SSList* list = malloc(sizeof(SSList));
    list->first = NULL;
    list->last = NULL;

    switch (expression->type)
    {
        case AST_BlockStatement:
        {
            Statement* node = expression;
            return node->block_statement.body;

            break;
        }
        case AST_VariableDeclaration:
        {
            VariableDeclaration* node = (VariableDeclaration*)expression;
            
            SSNode* newNode = SSNode_Create(malloc(sizeof(SSNode)), node->value);
            SSList_Append(list, newNode);

            break;
        }
        case AST_TypeDefinition:
        {
            TypeDeclaration* node = (TypeDeclaration*)expression;
            return node->properties;

            break;
        }
        case AST_PropertyDeclaration:
        {
            break;
        }
        case AST_AssignmentExpression:
        {
            AssignmentExpression* node = (AssignmentExpression*)expression;

            SSNode* left_node = SSNode_Create(malloc(sizeof(SSNode)), node->assignee);
            SSList_Append(list, left_node);

            SSNode* right_node = SSNode_Create(malloc(sizeof(SSNode)), node->value);
            SSList_Append(list, right_node);

            break;
        }
        case AST_BinaryExpression:
        {
            BinaryExpression* node = (BinaryExpression*)expression;

            SSNode* left_node = SSNode_Create(malloc(sizeof(SSNode)), node->left);
            SSList_Append(list, left_node);

            SSNode* right_node = SSNode_Create(malloc(sizeof(SSNode)), node->right);
            SSList_Append(list, right_node);

            break;
        }
        case AST_ComparisonExpression:
        {
            BinaryExpression* node = (BinaryExpression*)expression;

            SSNode* left_node = SSNode_Create(malloc(sizeof(SSNode)), node->left);
            SSList_Append(list, left_node);

            SSNode* right_node = SSNode_Create(malloc(sizeof(SSNode)), node->right);
            SSList_Append(list, right_node);

            break;
        }
        case AST_IfStatement:{
            IfStatement* node = (IfStatement*)expression;

            SSNode* test_node = SSNode_Create(malloc(sizeof(SSNode)), node->test);
            SSList_Append(list, test_node);

            SSNode* left_node = SSNode_Create(malloc(sizeof(SSNode)), node->consequent);
            SSList_Append(list, left_node);

            if(node->alternate != NULL){
                SSNode* right_node = SSNode_Create(malloc(sizeof(SSNode)), node->alternate);
                SSList_Append(list, right_node);
            }



            break;
        }
        case AST_MemberExpression:
        {
            break;
        }
        case AST_CallExpression:
        {
            break;
        }
        case AST_NumericLiteral:
        {
            // Cannot have children. Don't add anything.
            break;
        }
        case AST_Identifier:
        {
            // Cannot have children. Don't add anything.
            break;
        }
    }

    return list;
}

void Print_Node(Statement* node)
{
    switch (node->type)
    {
        case AST_BlockStatement:
        {
            printf("BlockStatement");
            break;
        }
        case AST_VariableDeclaration:
        {
            printf("VariableDeclaration: %.*s", node->variable_declaration.name.length, node->variable_declaration.name.start);
            break;
        }
        case AST_TypeDefinition:
        {
            printf("TypeDeclaration: %.*s", node->type_declaration.name.length, node->type_declaration.name.start);
            break;            
        }
        case AST_PropertyDeclaration:
        {
            printf("PropertyDeclaration: %.*s", node->property_declaration.name.length, node->property_declaration.name.start);
            break;
        }
        case AST_AssignmentExpression:
        {
            printf("AssignmentExpression");
            break;
        }
        case AST_BinaryExpression:
        {
            printf("BinaryExpression: %s", node->binary_expression.operator);
            break;  
        }
        case AST_ComparisonExpression:
        {
            printf("ComparisonExpression");
            break;    
        }
        case AST_MemberExpression:
        {
            printf("MemberExpression");    
            break;     
        }
        case AST_IfStatement:
        {
            printf("IfStatement");    
            break;     
        }
        case AST_CallExpression:
        {
            printf("CallExpression");   
            break;    
        }
        case AST_NumericLiteral:
        {
            NumericLiteral* item = (NumericLiteral*)node;
            printf("NumericLiteral: %i", item->value);
            break;    
        }
        case AST_Identifier:
        {
            printf("Identifier: %.*s", node->identifier.name.length, node->identifier.name.start); 
            break;  
        }
        default:
        {
            printf("Node printing error!");
            break;  
        }
    }
}

void PrettyPrint(Statement* node, char* indent, bool isLast){
    char* marker;

    if(isLast)
        marker = "└───";
    else
        marker = "├───";

    SSList* children = Get_Children(node);

    printf("%s", indent);
    printf("%s", marker);
    Print_Node(node);
    printf("\n");

    char *new_indent = (char*)malloc(strlen(indent) + 8);
    strcpy(new_indent, indent);

    if(isLast)
    {
        strcat(new_indent, "    ");     

        // if(children->first != NULL){
        //     printf("%s│\n", new_indent);
        // }
    }
    else
        strcat(new_indent, "│   ");

    Statement* lastChild;

    if(children->first == NULL)
        lastChild = NULL;
    else
        lastChild = (Statement*)children->last->value;

    SSNode* cursor = children->first;
    while(cursor != NULL){
        PrettyPrint(cursor->value, new_indent, cursor->next == NULL);
        cursor = cursor->next;
    }

    // Cleanup
    SSList_Free(children);
    free(new_indent);
}