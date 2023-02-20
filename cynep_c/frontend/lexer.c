#pragma once

typedef enum TokenType TokenType;
typedef struct Token Token; 
typedef struct BufferString BufferString; 

enum TokenType 
{
    // Literals
    Token_Number,
    Token_Identifier,

    // Keywords
    Token_Let,
    Token_Null,
    Token_Type,
    Token_If,
    Token_Else,

    // Operators
    Token_ComparisonOperator,
    Token_BinaryOperator,
    Token_Assignment,

    // Generic
    Token_Semicolon,
    Token_OpenParen,
    Token_CloseParen,
    Token_OpenBrace,
    Token_CloseBrace,
    Token_Comma,
    Token_Dot,
    Token_EOF
};

struct BufferString {
    char* start;
    size_t length;
};

struct Token 
{
    TokenType type;
    union
    {
        BufferString string_value;
        char operator_value[2];
        float64 number_value;
    };
};

void Token_Create(Token* token, TokenType type, char* start, size_t length) 
{
    token->type = type;
    token->string_value.length = length;
    token->string_value.start = start;
}

void Token_Number_Create(Token* token, TokenType type, float64 value) 
{
    token->type = type;
    token->number_value = value;
}

void Token_Operator_Create(Token* token, TokenType type, char operator[2]) 
{
    token->type = type;
    token->string_value.length = 0;
    token->string_value.start = NULL;
    strcpy(token->operator_value, operator);
}

bool Is_Skippable(char c) 
{
    return c == -85 || c == '\n' || c == '\t' || c == '\r' || isspace(c);
}

Token* lexer_tokenize(SourceFile* file)
{
    char* file_buffer = file->buffer;
    int64 t1 = timestamp();

    // TODO: Should predict a good size based on characters in document
    // Maybe just allocate as if every character is a token?
    // Kind of wasteful but then we would never have to reallocate.
    size_t tokens_max = 10000000; 
    size_t tokens_count = 0;

    size_t iterator = 0;

    Token* tokens = (Token*)malloc(sizeof(Token) * tokens_max);

    while(file_buffer[iterator] != NULL_CHAR)
    {
        char* current = &file_buffer[iterator];
        char* lookahead = &file_buffer[iterator + 1];

        char current_as_string[2];
        current_as_string[0] = *current;
        current_as_string[1] = NULL_CHAR;

        char current_and_lookahead_as_string[3];
        current_and_lookahead_as_string[0] = *current;
        current_and_lookahead_as_string[1] = *lookahead;
        current_and_lookahead_as_string[2] = NULL_CHAR;

        switch (*current)
        {
            case '+':
            case '-':
            case '*':
            case '/':
            case '%':
            {
                Token_Operator_Create(&tokens[tokens_count++], Token_BinaryOperator, current_as_string);  
                break;
            }    
            case '(':
            {
                Token_Operator_Create(&tokens[tokens_count++], Token_OpenParen, current_as_string);  
                break;
            }
            case ')':
            {
                Token_Operator_Create(&tokens[tokens_count++], Token_CloseParen, current_as_string);  
                break;
            }   
            case '{':
            {
                Token_Operator_Create(&tokens[tokens_count++], Token_OpenBrace, current_as_string);  
                break;
            }
            case '}':
            {
                Token_Operator_Create(&tokens[tokens_count++], Token_CloseBrace, current_as_string);  
                break;
            }
            case ',':
            {
                Token_Operator_Create(&tokens[tokens_count++], Token_Comma, current_as_string);  
                break;
            }
            case ';':
            {
                Token_Operator_Create(&tokens[tokens_count++], Token_Semicolon, current_as_string);  
                break;
            }
            case '.':
            {
                Token_Operator_Create(&tokens[tokens_count++], Token_Dot, current_as_string);  
                break;
            }
            case '!':
            {   
                if(*lookahead == '=')
                {
                    Token_Operator_Create(&tokens[tokens_count++], Token_ComparisonOperator, current_and_lookahead_as_string);  
                    iterator++;
                }
                else{
                    // Some kind of negation operator
                }
                
                break;
            }
            case '>':
            {   
                if(*lookahead == '=')
                {
                    Token_Operator_Create(&tokens[tokens_count++], Token_ComparisonOperator, current_and_lookahead_as_string);  
                    iterator++;
                }
                else{
                    Token_Operator_Create(&tokens[tokens_count++], Token_ComparisonOperator, current_as_string); 
                }
                
                break;
            }
            case '<': 
            {   
                if(*lookahead == '=')
                {
                    Token_Operator_Create(&tokens[tokens_count++], Token_ComparisonOperator, current_and_lookahead_as_string);  
                    iterator++;
                }
                else{
                    Token_Operator_Create(&tokens[tokens_count++], Token_ComparisonOperator, current_as_string); 
                }
                
                break;
            }
            case '=':
            {   
                if(*lookahead == '=')
                {
                    Token_Operator_Create(&tokens[tokens_count++], Token_ComparisonOperator, current_and_lookahead_as_string);  
                    iterator++;
                }
                else{
                    Token_Operator_Create(&tokens[tokens_count++], Token_Assignment, current_as_string); 
                }
                
                break;
            }
            default: 
            {
                // Handle multicharacter tokens here

                if (isdigit(*current))
                {
                    // Create number token
                    char number[50] = "";
                    size_t start_pos = iterator;
                    size_t length = 0;
                    while (isdigit(file_buffer[start_pos + length]))
                    {
                        strncat(number, &file_buffer[start_pos + length], 1);
                        length++;
                    }
                    iterator += length - 1;

                    float64 parsed_number = atoi(number); // atof for float. Slow as fuck. Just parse int and cast to float?

                    Token_Number_Create(&tokens[tokens_count++], Token_Number, parsed_number); 
                }
                else if (isalpha(*current))
                {
                    size_t start_pos = iterator;
                    size_t length = 0;
                    for (; isalpha(file_buffer[start_pos + length]); length++) {} // Count the length of the identifier
                    iterator += length - 1; // Reduce by one cause we also increment at the end of loop

                    // Check for reserved keywords
                    if(strncmp(current, "type", length) == 0)
                    {
                        Token_Create(&tokens[tokens_count++], Token_Type, current, length); 
                    }
                    else if(strncmp(current, "var", length) == 0)
                    {
                        Token_Create(&tokens[tokens_count++], Token_Let, current, length); 
                    }
                    else if(strncmp(current, "if", length) == 0)
                    {
                        Token_Create(&tokens[tokens_count++], Token_If, current, length); 
                    }
                    else if(strncmp(current, "else", length) == 0)
                    {
                        Token_Create(&tokens[tokens_count++], Token_Else, current, length); 
                    }
                    else
                    {
                        Token_Create(&tokens[tokens_count++], Token_Identifier, current, length); 
                    }
                }
                else if (Is_Skippable(*current))
                {
                    break;
                }
                else
                {   
                    printf("Lexer error. Unrecognized character in source: \"%c\".\n", current);
                    exit(0);
                }
            }
        }

        // if(tokens_count == tokens_max)
        // {
        //     tokens_max *= 2;
        //     tokens = (Token*)realloc(tokens, sizeof(Token) * tokens_max);
        // }

        iterator++;
    }  

    Token_Create(&tokens[tokens_count++], Token_EOF, NULL, 0);  

    // for (size_t i = 0; i < tokens_count; i++)
    // {
    //     printf("%s", tokens[i].operator);
    //     printf("%i %.*s\n", tokens[i].type, (int)tokens[i].token_str.length, tokens[i].token_str.start);
    // }
    
    int64 t2 = timestamp();
    printf("Tokenization: %d ms\n", t2/1000-t1/1000);

    return tokens;
}