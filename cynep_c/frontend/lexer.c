#pragma once

typedef enum   TokenType  TokenType;
typedef struct Token      Token; 
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
    BufferString token_str;
    char operator[2];
};



void create_token(Token* token, TokenType type, char* start, size_t length) 
{
    token->type = type;
    token->token_str.length = length;
    token->token_str.start = start;
}

void create_token_operator(Token* token, TokenType type, char operator[2]) 
{
    token->type = type;
    token->token_str.length = NULL;
    token->token_str.start = NULL;
    strcpy(token->operator, operator);
}

bool is_skippable(char c) 
{
    return c == -85 || c == '\n' || c == '\t' || c == '\r' || isspace(c);
}

Token* lexer_tokenize(char* str)
{
    size_t iterator = 0;

    int64 t1 = timestamp();

    // TODO: Should predict a good size based on characters in document
    // Maybe just allocate as if every character is a token?
    // Kind of wasteful but then we would never have to reallocate.
    size_t tokens_max = 10000000; 
    size_t tokens_count = 0;

    // size_t strings_cursor = 0;

    Token* tokens = (Token*)malloc(sizeof(Token) * tokens_max);
    // char* strings = (char*)malloc(sizeof(char) * tokens_max);

    char* current;
    char* lookahead;
    char current_as_string[2];
    char current_and_lookahead_as_string[3];

    while(str[iterator] != NULL_CHAR)
    {
        current = &str[iterator];
        lookahead = &str[iterator + 1];

        current_as_string[0] = *current;
        current_as_string[1] = NULL_CHAR;

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
                create_token_operator(&tokens[tokens_count++], Token_BinaryOperator, current_as_string);  
                break;
            }    
            case '(':
            {
                create_token_operator(&tokens[tokens_count++], Token_OpenParen, current_as_string);  
                break;
            }
            case ')':
            {
                create_token_operator(&tokens[tokens_count++], Token_CloseParen, current_as_string);  
                break;
            }   
            case '{':
            {
                create_token_operator(&tokens[tokens_count++], Token_OpenBrace, current_as_string);  
                break;
            }
            case '}':
            {
                create_token_operator(&tokens[tokens_count++], Token_CloseBrace, current_as_string);  
                break;
            }
            case ',':
            {
                create_token_operator(&tokens[tokens_count++], Token_Comma, current_as_string);  
                break;
            }
            case ';':
            {
                create_token_operator(&tokens[tokens_count++], Token_Semicolon, current_as_string);  
                break;
            }
            case '.':
            {
                create_token_operator(&tokens[tokens_count++], Token_Dot, current_as_string);  
                break;
            }
            case '!':
            {   
                if(*lookahead == '=')
                {
                    create_token_operator(&tokens[tokens_count++], Token_ComparisonOperator, current_and_lookahead_as_string);  
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
                    create_token_operator(&tokens[tokens_count++], Token_ComparisonOperator, current_and_lookahead_as_string);  
                    iterator++;
                }
                else{
                    create_token_operator(&tokens[tokens_count++], Token_ComparisonOperator, current_as_string); 
                }
                
                break;
            }
            case '<': 
            {   
                if(*lookahead == '=')
                {
                    create_token_operator(&tokens[tokens_count++], Token_ComparisonOperator, current_and_lookahead_as_string);  
                    iterator++;
                }
                else{
                    create_token_operator(&tokens[tokens_count++], Token_ComparisonOperator, current_as_string); 
                }
                
                break;
            }
            case '=':
            {   
                if(*lookahead == '=')
                {
                    create_token_operator(&tokens[tokens_count++], Token_ComparisonOperator, current_and_lookahead_as_string);  
                    iterator++;
                }
                else{
                    create_token_operator(&tokens[tokens_count++], Token_Assignment, current_as_string); 
                }
                
                break;
            }
            default: 
            {
                // Handle multicharacter tokens here

                if (isdigit(*current))
                {
                    // Create number token
                    //char number[50] = "";
                    size_t start_pos = iterator;
                    size_t length = 0;
                    while (isdigit(str[start_pos + length]))
                    {
                        //strncat(number, &str[iterator], 1);
                        length++;
                    }
                    iterator += length - 1;
                    

                    create_token(&tokens[tokens_count++], Token_Number, current, length); 
                }
                else if (isalpha(*current))
                {
                    //char identifier[50] = "";

                    size_t start_pos = iterator;
                    size_t length = 0;
                    while (isalpha(str[start_pos + length]))
                    {
                        //strncat(identifier, &str[start_pos + length], 1);
                        length++;
                    }
                    iterator += length - 1;

                    // Check for reserved keywords
                    // strncmp(identifier, var, length); 
                    if(strncmp(current, "type", length) == 0)
                    {
                        create_token(&tokens[tokens_count++], Token_Type, current, length); 
                    }
                    else if(strncmp(current, "var", length) == 0)
                    {
                        create_token(&tokens[tokens_count++], Token_Let, current, length); 
                    }
                    else
                    {
                        create_token(&tokens[tokens_count++], Token_Identifier, current, length); 
                    }
                }
                else if (is_skippable(*current))
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

    create_token(&tokens[tokens_count++], Token_EOF, NULL, 0);  

    // for (size_t i = 0; i < tokens_count; i++)
    // {
    //     printf("%s", tokens[i].operator);
    //     printf("%i %.*s\n", tokens[i].type, (int)tokens[i].token_str.length, tokens[i].token_str.start);
    // }
    
    int64 t2 = timestamp();
    printf("Tokenization: %d ms\n", t2/1000-t1/1000);

    return tokens;
}