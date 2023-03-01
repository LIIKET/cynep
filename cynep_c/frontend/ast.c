#pragma once

typedef enum   NodeType NodeType;
typedef struct AstNode AstNode;
typedef struct BlockStatement BlockStatement;
typedef struct Identifier Identifier;
typedef struct NumericLiteral NumericLiteral;
typedef struct StringLiteral StringLiteral;
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
typedef struct WhileStatement WhileStatement;
typedef struct FunctionDeclaration FunctionDeclaration;

enum NodeType 
{
    // Statements
    AST_BlockStatement,
    AST_IfStatement,
    AST_WhileStatement,
    AST_VariableDeclaration,
    AST_TypeDefinition,
    AST_PropertyDeclaration,
    AST_FunctionDeclaration,

    // Expression
    AST_AssignmentExpression,
    AST_BinaryExpression,
    AST_ComparisonExpression,
    AST_MemberExpression,
    AST_CallExpression,

    // Literals
    AST_NumericLiteral,
    AST_StringLiteral,
    AST_Identifier,
};

struct BlockStatement {
    List* body; // Statements
};

struct IfStatement {
    ComparisonExpression* test;
    AstNode* consequent; // Statements
    AstNode* alternate; // Statements
};

struct WhileStatement {
    ComparisonExpression* test;
    AstNode* body;
};

struct FunctionDeclaration {
    BufferString name;
    List* args; // List of identifiers, TODO: Replace with arg struct containing type info?
    BlockStatement* body;
};

struct Identifier {
    BufferString name;
};

struct NumericLiteral {
    int64 value;
};

struct StringLiteral {
    BufferString value;
};

struct VariableDeclaration {
    BufferString name;
    Expression* value;
};

struct PropertyDeclaration {
    BufferString name;
};

struct TypeDeclaration {
    BufferString name;
    List* properties; // PropertyDeclarations
    // int64 properties_count;
};

struct CallExpression {
    Expression* callee;
    List* args;
};

struct MemberExpression {
    Expression* object;
    Identifier* member;
};

struct AssignmentExpression {
    Expression* assignee;
    Expression* value;
};

struct BinaryExpression {
    char operator[2];
    Expression* left;
    Expression* right;
};

struct AstNode {
    union {
        BlockStatement block_statement;
        IfStatement ifStatement;
        Identifier identifier;
        NumericLiteral numeric_literal;
        StringLiteral string_literal;
        VariableDeclaration variable_declaration;
        PropertyDeclaration property_declaration;
        TypeDeclaration type_declaration;
        CallExpression call_expression;
        MemberExpression member_expression;
        AssignmentExpression assignment_expression;
        BinaryExpression binary_expression;
        BinaryExpression comparison_expression;
        WhileStatement while_statement;
        FunctionDeclaration function_declaration;
    }; 
    NodeType type;
};

struct Expression {
    AstNode statement;
};

//
// Initializers
//

BlockStatement* Create_BlockStatement(AstNode* node, List* body) {
    node->type = AST_BlockStatement;
    node->block_statement.body = list_create(body);

    return (BlockStatement*)node;
}

FunctionDeclaration* Create_FunctionDeclaration(AstNode* memory, BufferString name, List* args, BlockStatement* block) {
    memory->type = AST_FunctionDeclaration;
    memory->function_declaration.args = args;
    memory->function_declaration.body = block;
    memory->function_declaration.name = name;

    return (FunctionDeclaration*)memory;
}

VariableDeclaration* Create_VariableDeclaration(AstNode* memory, BufferString name, Expression* value) {
    memory->type = AST_VariableDeclaration;
    memory->variable_declaration.value = value;

    memory->variable_declaration.name.start = name.start;
    memory->variable_declaration.name.length = name.length;

    return (VariableDeclaration*)memory;
}

TypeDeclaration* Create_TypeDeclaration(AstNode* memory, List* member_list_memory, BufferString name) {
    memory->type = AST_TypeDefinition;
    memory->type_declaration.properties = list_create(member_list_memory);

    memory->type_declaration.name.start = name.start;
    memory->type_declaration.name.length = name.length;

    return (TypeDeclaration*)memory;
}

PropertyDeclaration* Create_PropertyDeclaration(AstNode* memory, BufferString name) {
    memory->type = AST_PropertyDeclaration;

    memory->property_declaration.name.start = name.start;
    memory->property_declaration.name.length = name.length;

    return (PropertyDeclaration*)memory;
}

CallExpression* Create_CallExpression(AstNode* memory, Expression* callee, List* args) {
    memory->type = AST_CallExpression;
    memory->call_expression.callee = callee;
    memory->call_expression.args = args;

    return (CallExpression*)memory;
}

MemberExpression* Create_MemberExpression(AstNode* memory, Expression* object, Identifier* member) {
    memory->type = AST_MemberExpression;
    memory->member_expression.object = object;
    memory->member_expression.member = member;

    return (MemberExpression*)memory;
}

AssignmentExpression* Create_AssignmentExpression(AstNode* memory, Expression* assignee, Expression* value) {
    memory->type = AST_AssignmentExpression;
    memory->assignment_expression.assignee = assignee;
    memory->assignment_expression.value = value;

    return (AssignmentExpression*)memory;
}

BinaryExpression* Create_BinaryExpression(AstNode* memory, Expression* left, char* operatr, Expression* right) {
    memory->type = AST_BinaryExpression;
    memory->binary_expression.left = left;
    memory->binary_expression.right = right;

    strcpy(memory->binary_expression.operator, operatr);

    return (BinaryExpression*)memory;
}

ComparisonExpression* Create_ComparisonExpression(AstNode* memory, Expression* left, char* operatr, Expression* right) {
    memory->type = AST_ComparisonExpression;
    memory->comparison_expression.left = left;
    memory->comparison_expression.right = right;

    strcpy(memory->comparison_expression.operator, operatr);

    return (ComparisonExpression*)memory;
}

IfStatement* Create_IfStatement(AstNode* memory, ComparisonExpression* test, AstNode* consequent, AstNode* alternate) {
    memory->type = AST_IfStatement;
    memory->ifStatement.test = test;
    memory->ifStatement.consequent = consequent;
    memory->ifStatement.alternate = alternate;

    return (IfStatement*)memory;
}

WhileStatement* Create_WhileStatement(AstNode* memory, ComparisonExpression* test, AstNode* body) {
    memory->type = AST_WhileStatement;
    memory->while_statement.test = test;
    memory->while_statement.body = body;

    return (WhileStatement*)memory;
}

Identifier* Create_Identifier(AstNode* memory, BufferString name) {
    memory->type = AST_Identifier;
    memory->identifier.name.start = name.start;
    memory->identifier.name.length = name.length;

    return (Identifier*)memory;
}

NumericLiteral* Create_NumericLiteral(AstNode* memory, int64 value) {
    memory->type = AST_NumericLiteral;
    memory->numeric_literal.value = value;

    return (NumericLiteral*)memory;
}

StringLiteral* Create_StringLiteral(AstNode* memory, BufferString value) {
    memory->type = AST_StringLiteral;
    memory->string_literal.value = value;

    return (StringLiteral*)memory;
}

//
// Visualizing
//
List* Get_Children(AstNode* expression) {
    List* list = malloc(sizeof(List));
    list->first = NULL;
    list->last = NULL;

    switch (expression->type) {
        case AST_BlockStatement: 
        {
            AstNode* node = expression;
            return node->block_statement.body;

            break;
        }
        case AST_VariableDeclaration:
        {
            VariableDeclaration* node = (VariableDeclaration*)expression;
            
            if(node->value != NULL){
                ListNode* newNode = listNode_create(malloc(sizeof(ListNode)), node->value);
                list_append(list, newNode);
            }

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

            ListNode* left_node = listNode_create(malloc(sizeof(ListNode)), node->assignee);
            list_append(list, left_node);

            ListNode* right_node = listNode_create(malloc(sizeof(ListNode)), node->value);
            list_append(list, right_node);

            break;
        }
        case AST_CallExpression:
        {
            CallExpression* node = (CallExpression*)expression;

            ListNode* left_node = listNode_create(malloc(sizeof(ListNode)), node->callee);
            list_append(list, left_node);

            if(node->args->first != NULL){
                ListNode* cursor = node->args->first;
                while(cursor != NULL){
                    ListNode* arg_node = listNode_create(malloc(sizeof(ListNode)), cursor->value);
                    list_append(list, arg_node);
                    cursor = cursor->next;
                }
            }

            break;
        }
        case AST_FunctionDeclaration:
        {
            FunctionDeclaration* node = (FunctionDeclaration*)expression;

            if(node->args->first != NULL){
                ListNode* cursor = node->args->first;
                while(cursor != NULL){
                    ListNode* arg_node = listNode_create(malloc(sizeof(ListNode)), cursor->value);
                    list_append(list, arg_node);
                    cursor = cursor->next;
                }
            }

            ListNode* left_node = listNode_create(malloc(sizeof(ListNode)), node->body);
            list_append(list, left_node);
            
            break;
        }
        case AST_BinaryExpression:
        {
            BinaryExpression* node = (BinaryExpression*)expression;

            ListNode* left_node = listNode_create(malloc(sizeof(ListNode)), node->left);
            list_append(list, left_node);

            ListNode* right_node = listNode_create(malloc(sizeof(ListNode)), node->right);
            list_append(list, right_node);

            break;
        }
        
        case AST_ComparisonExpression:
        {
            BinaryExpression* node = (BinaryExpression*)expression;

            ListNode* left_node = listNode_create(malloc(sizeof(ListNode)), node->left);
            list_append(list, left_node);

            ListNode* right_node = listNode_create(malloc(sizeof(ListNode)), node->right);
            list_append(list, right_node);

            break;
        }
        case AST_IfStatement:{
            IfStatement* node = (IfStatement*)expression;

            ListNode* test_node = listNode_create(malloc(sizeof(ListNode)), node->test);
            list_append(list, test_node);

            ListNode* left_node = listNode_create(malloc(sizeof(ListNode)), node->consequent);
            list_append(list, left_node);

            if(node->alternate != NULL){
                ListNode* right_node = listNode_create(malloc(sizeof(ListNode)), node->alternate);
                list_append(list, right_node);
            }
            
            break;
        }
        case AST_WhileStatement:{
            WhileStatement* node = (WhileStatement*)expression;

            ListNode* test_node = listNode_create(malloc(sizeof(ListNode)), node->test);
            list_append(list, test_node);

            ListNode* left_node = listNode_create(malloc(sizeof(ListNode)), node->body);
            list_append(list, left_node);


            
            break;
        }
        case AST_MemberExpression:
        {
            MemberExpression* node = (MemberExpression*)expression;

            ListNode* right_node = listNode_create(malloc(sizeof(ListNode)), node->object);
            list_append(list, right_node);

            ListNode* left_node = listNode_create(malloc(sizeof(ListNode)), node->member);
            list_append(list, left_node);

            break;
        }
        case AST_NumericLiteral:
        {
            // Cannot have children. Don't add anything.
            break;
        }
        case AST_StringLiteral:
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

void astNode_print(AstNode* node)
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
        case AST_WhileStatement:
        {
            printf("WhileStatement");   
            break;    
        }
        case AST_FunctionDeclaration:
        {
            FunctionDeclaration* item = (FunctionDeclaration*)node;
            printf("FunctionDeclaration: %.*s", item->name.length, item->name.start);
            break;    
        }
        case AST_NumericLiteral:
        {
            NumericLiteral* item = (NumericLiteral*)node;
            printf("NumericLiteral: %i", item->value);
            break;    
        }
        case AST_StringLiteral:
        {
            StringLiteral* item = (StringLiteral*)node;
            printf("StringLiteral: %.*s", item->value.length, item->value.start);
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

void ast_print(AstNode* node, char* indent, bool isLast){
    char* marker;

    if(isLast)
        marker = "└───";
    else
        marker = "├───";

    List* children = Get_Children(node);

    printf("%s", indent);
    printf("%s", marker);
    astNode_print(node);
    printf("\n");

    char *new_indent = (char*)malloc(strlen(indent) + 8);
    strcpy(new_indent, indent);

    if(isLast)
        strcat(new_indent, "    ");     
    else
        strcat(new_indent, "│   ");

    AstNode* lastChild;

    if(children->first == NULL)
        lastChild = NULL;
    else
        lastChild = (AstNode*)children->last->value;

    ListNode* cursor = children->first;
    while(cursor != NULL){
        ast_print(cursor->value, new_indent, cursor->next == NULL);
        cursor = cursor->next;
    }

    // Cleanup
    free(new_indent);
}

void ast_print_begin(AstNode* program){
    printf("\n---------------- ABSTRACT SYNTAX TREE ----------------\n\n");

    ast_print(program, "", true);
}