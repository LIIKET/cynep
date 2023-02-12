    #pragma once

    typedef enum   NodeType NodeType;
    typedef struct Statement Statement;
    typedef struct Program Program;
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

    enum NodeType 
    {
        // Statements
        AST_Program,
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

    struct Program 
    {
        SSList* body;
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
        PropertyDeclaration* properties[50];
        int64 properties_count;
    };

    struct CallExpression 
    {
        Expression* callee;
        Expression* args[50];
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
        char operatr[50];
        Expression* left;
        Expression* right;
    };

    struct Statement 
    {
        union
        {
            Program program;
            Identifier identifier;
            NumericLiteral numericLiteral;
            VariableDeclaration variableDeclaration;
            PropertyDeclaration propertyDeclaration;
            TypeDeclaration typeDeclaration;
            CallExpression callExpression;
            MemberExpression memberExpression;
            AssignmentExpression assignmentExpression;
            BinaryExpression binaryExpression;
            BinaryExpression comparisonExpression;
        }; 
        NodeType type;
    };

    struct Expression {
        Statement statement;
    };
    //
    // Initializers
    //

    Statement* Create_Program()
    {
        Statement* node = (Statement*)malloc(sizeof(Statement));
        node->type = AST_Program;
        node->program.body = SSList_Create(malloc(sizeof(SSList)));

        return node;
    }

    VariableDeclaration* Create_VariableDeclaration(Statement* memory, BufferString name, Expression* value)
    {
        memory->type = AST_VariableDeclaration;
        memory->variableDeclaration.value = value;

        memory->variableDeclaration.name.start = name.start;
        memory->variableDeclaration.name.length = name.length;

        // strcpy(memory->variableDeclaration.name, name);

        return (VariableDeclaration*)memory;
    }

    TypeDeclaration* Create_TypeDeclaration(Statement* memory, BufferString name)
    {
        memory->type = AST_TypeDefinition;
        memory->typeDeclaration.properties_count = 0;

        memory->typeDeclaration.name.start = name.start;
        memory->typeDeclaration.name.length = name.length;
        // strcpy(memory->typeDeclaration.name, name);

        return (TypeDeclaration*)memory;
    }

    PropertyDeclaration* Create_PropertyDeclaration(Statement* memory, BufferString name)
    {
        memory->type = AST_PropertyDeclaration;

        memory->propertyDeclaration.name.start = name.start;
        memory->propertyDeclaration.name.length = name.length;
        // strcpy(memory->propertyDeclaration.name, name);

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
        memory->memberExpression.object = object;
        memory->memberExpression.property = property;

        return (MemberExpression*)memory;
    }

    AssignmentExpression* Create_AssignmentExpression(Statement* memory, Expression* assignee, Expression* value)
    {
        memory->type = AST_AssignmentExpression;
        memory->assignmentExpression.assignee = assignee;
        memory->assignmentExpression.value = value;

        return (AssignmentExpression*)memory;
    }

    BinaryExpression* Create_BinaryExpression(Statement* memory, Expression* left, char* operatr, Expression* right)
    {
        memory->type = AST_BinaryExpression;
        memory->binaryExpression.left = left;
        memory->binaryExpression.right = right;

        strcpy(memory->binaryExpression.operatr, operatr);

        return (BinaryExpression*)memory;
    }

    ComparisonExpression* Create_ComparisonExpression(Statement* memory, Expression* left, char* operatr, Expression* right)
    {
        memory->type = AST_ComparisonExpression;
        memory->comparisonExpression.left = left;
        memory->comparisonExpression.right = right;

        strcpy(memory->comparisonExpression.operatr, operatr);

        return (ComparisonExpression*)memory;
    }

    Identifier* Create_Identifier(Statement* memory, BufferString name)
    {
        memory->type = AST_Identifier;
        memory->identifier.name.start = name.start;
        memory->identifier.name.length = name.length;
        // strcpy(memory->identifier.name, name);

        return (Identifier*)memory;
    }

    NumericLiteral* Create_NumericLiteral(Statement* memory, int64 value)
    {
        memory->type = AST_NumericLiteral;
        memory->numericLiteral.value = value;

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
            case AST_Program:
            {
                Statement* node = expression;
                return node->program.body;

                break;
            }
            case AST_VariableDeclaration:
            {
                Statement* node = expression;

                SSNode* newNode = malloc(sizeof(SSNode));
                newNode->next = NULL;
                newNode->prev = NULL;
                newNode->value = node->variableDeclaration.value;
                SSList_Append(list, newNode);

                break;
            }
            case AST_TypeDefinition:
            {
                break;
            }
            case AST_PropertyDeclaration:
            {
                break;
            }
            case AST_AssignmentExpression:
            {
                AssignmentExpression* node = (AssignmentExpression*)expression;

                SSNode* newNode = malloc(sizeof(SSNode));
                newNode->next = NULL;
                newNode->prev = NULL;
                newNode->value = node->assignee;
                SSList_Append(list, newNode);

                SSNode* newNode2 = malloc(sizeof(SSNode));
                newNode2->next = NULL;
                newNode2->prev = NULL;
                newNode2->value = node->value;
                SSList_Append(list, newNode2);

                break;
            }
            case AST_BinaryExpression:
            {
                BinaryExpression* node = (BinaryExpression*)expression;

                SSNode* newNode = malloc(sizeof(SSNode));
                newNode->next = NULL;
                newNode->prev = NULL;
                newNode->value = node->left;
                SSList_Append(list, newNode);

                SSNode* newNode2 = malloc(sizeof(SSNode));
                newNode2->next = NULL;
                newNode2->prev = NULL;
                newNode2->value = node->right;
                SSList_Append(list, newNode2);

                break;
            }
            case AST_ComparisonExpression:
            {
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
                break;
            }
            case AST_Identifier:
            {
                break;
            }
        }

        return list;
    }

    void Print_Node(Statement* node)
    {
        switch (node->type)
        {
            case AST_Program:
            {
                printf("Program");
                break;
            }
            case AST_VariableDeclaration:
            {
                printf("VariableDeclaration: %.*s", node->variableDeclaration.name.length, node->variableDeclaration.name.start);
                break;
            }
            case AST_TypeDefinition:
            {
                printf("TypeDeclaration: %.*s", node->typeDeclaration.name.length, node->typeDeclaration.name.start);
                break;            
            }
            case AST_PropertyDeclaration:
            {
                printf("PropertyDeclaration: %.*s", node->propertyDeclaration.name.length, node->propertyDeclaration.name.start);
                break;
            }
            case AST_AssignmentExpression:
            {
                printf("AssignmentExpression");
                break;
            }
            case AST_BinaryExpression:
            {
                printf("BinaryExpression: %s", node->binaryExpression.operatr);
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
                printf("Identifier");    
                break;  
            }
            default:
            {
                printf("Node printing error!");
                break;  
            }
        }
    }