#pragma once

typedef enum TokenType TokenType;
typedef struct Token Token; 
typedef struct BufferString BufferString; 
typedef enum ParseState ParseState;

enum TokenType 
{
    // Literals
    Token_Number,
    Token_String,
    Token_Identifier,

    // Keywords
    Token_Let,
    Token_Null,
    Token_Type,
    Token_If,
    Token_Else,
    Token_While,
    Token_Func,

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

enum ParseState {
    ParseState_Start,
    ParseState_Symbol
};

struct BufferString {
    char* start;
    size_t length;
};

struct Token {
    TokenType type;
    union
    {
        BufferString string_value;
        char operator_value[2];
        float64 number_value;
    };
};

unsigned long hash_function(char* str)
{
    unsigned long i = 0;

    for (int j = 0; str[j]; j++)
        i += str[j];

    return i % 50000;
}

Token Token_Create(TokenType type, char* start, size_t length) {
    Token token;
    token.type = type;
    token.string_value.length = length;
    token.string_value.start = start;

    return token;
}

Token Token_Number_Create(TokenType type, float64 value) {
    Token token;
    token.type = type;
    token.number_value = value;

    return token;
}

Token Token_Operator_Create(TokenType type, char operator[2]) {
    Token token;
    token.type = type;
    token.string_value.length = 0;
    token.string_value.start = NULL;
    strcpy(token.operator_value, operator);

    return token;
}

bool Is_Skippable(char c) {
    return isspace(c);
}

int token_count = 0;

Token* NextTokenMem(MemPool* pool) {
    token_count++;
    return MemPool_GetMem(pool, sizeof(Token)).pointer;
}

Token* lexer_tokenize(TextFile* file) {
    char* file_buffer = file->buffer;
    int64 t1 = timestamp();

    // Just allocate as if every character is a token?
    // Kind of wasteful but then we would never have to reallocate.
    size_t tokens_max = file->length / 2;

    MemPool* pool = MemPool_Make(sizeof(Token) * tokens_max);
    size_t iterator = 0;



    ParseState state = ParseState_Start;

    char* buff_start = 0;
    size_t buff_length = 0;

    while(file_buffer[iterator] != NULL_CHAR) {
        char* current = &file_buffer[iterator];
        char* lookahead = &file_buffer[iterator + 1];

        char current_as_string[2];
        current_as_string[0] = *current;
        current_as_string[1] = NULL_CHAR;

        char current_and_lookahead_as_string[3];
        current_and_lookahead_as_string[0] = *current;
        current_and_lookahead_as_string[1] = *lookahead;
        current_and_lookahead_as_string[2] = NULL_CHAR;

        switch (state)
        {
            case ParseState_Start: 
            {
                switch (*current)
                {
                    case '+':
                    case '-':
                    case '*':
                    case '/':
                    case '%': { *NextTokenMem(pool) = Token_Operator_Create(Token_BinaryOperator, current_as_string); break; }    
                    case '(': { *NextTokenMem(pool) = Token_Operator_Create(Token_OpenParen, current_as_string); break; }
                    case ')': { *NextTokenMem(pool) = Token_Operator_Create(Token_CloseParen, current_as_string); break; }   
                    case '{': { *NextTokenMem(pool) = Token_Operator_Create(Token_OpenBrace, current_as_string); break; }
                    case '}': { *NextTokenMem(pool) = Token_Operator_Create(Token_CloseBrace, current_as_string); break; }
                    case ',': { *NextTokenMem(pool) = Token_Operator_Create(Token_Comma, current_as_string); break; }
                    case ';': { *NextTokenMem(pool) = Token_Operator_Create(Token_Semicolon, current_as_string); break; }
                    case '.': { *NextTokenMem(pool) = Token_Operator_Create(Token_Dot, current_as_string); break; }
                    case '!': {   
                        if(*lookahead == '=')
                        {
                            *NextTokenMem(pool) = Token_Operator_Create(Token_ComparisonOperator, current_and_lookahead_as_string);  
                            iterator++;
                        }
                        else{
                            // Some kind of negation operator
                        }
                        
                        break;
                    }
                    case '>': {   
                        if(*lookahead == '=') {
                            *NextTokenMem(pool) = Token_Operator_Create(Token_ComparisonOperator, current_and_lookahead_as_string);                  
                            iterator++;
                        }
                        else {
                            *NextTokenMem(pool) = Token_Operator_Create(Token_ComparisonOperator, current_as_string); 
                        }
                        
                        break;
                    }
                    case '<': {   
                        if(*lookahead == '=') {
                            *NextTokenMem(pool) = Token_Operator_Create(Token_ComparisonOperator, current_and_lookahead_as_string);  
                            iterator++;
                        }
                        else {
                            *NextTokenMem(pool) = Token_Operator_Create(Token_ComparisonOperator, current_as_string); 
                        }
                        
                        break;
                    }
                    case '=': {   
                        if(*lookahead == '=') {
                            *NextTokenMem(pool) = Token_Operator_Create(Token_ComparisonOperator, current_and_lookahead_as_string);  
                            iterator++;
                        }
                        else {
                            *NextTokenMem(pool) = Token_Operator_Create(Token_Assignment, current_as_string); 
                        }
                        
                        break;
                    }
                    case '"': {             
                        // String literals

                        char* start = current + 1;
                        size_t start_pos = iterator + 1;
                        size_t length = 0;

                        // Count the length of the string
                        for (; file_buffer[start_pos + length] != '"'; length++) {} 

                        iterator += length + 1;

                        *NextTokenMem(pool) = Token_Create(Token_String, start, length); 

                        break;
                    }
                    default: {
                        if (isdigit(*current)) {
                            // Numbers

                            char number[50] = "";
                            size_t start_pos = iterator;
                            size_t length = 0;

                            while (isdigit(file_buffer[start_pos + length]))
                            {
                                strncat(number, &file_buffer[start_pos + length], 1);
                                length++;
                            }

                            // Reduce by one cause we also increment at the end of loop
                            iterator += length - 1;

                            // atof for float. Slow as fuck. Just parse int and cast to float?
                            float64 parsed_number = atoi(number); 

                            *NextTokenMem(pool) = Token_Number_Create(Token_Number, parsed_number); 
                        }
                        else if (isalpha(*current)) {
                            // Reserved keywords and identifiers
                            state = ParseState_Symbol;
                            buff_start = current;
                            buff_length = 1;
                            break;;
                        }
                        else if (Is_Skippable(*current)) {
                            break;
                        }
                        else {   
                            printf("Lexer error. Unrecognized character in source: \"%c\".\n", *current);
                            exit(0);
                        }
                    }
                }


                break;
            }

            case ParseState_Symbol: {
                if (isalpha(*current)) {
                    // Reserved keywords and identifiers
                    buff_length++;
                }
                else { //if (Is_Skippable(*current)) 
                    state = ParseState_Start;

                    // uint32_t type = hash_function("type");
                    // uint32_t var = hash_function("var");
                    // uint32_t if1 = hash_function("if");
                    // uint32_t else1 = hash_function("else");
                    // uint32_t assd = hash_function("while");
                    // uint32_t asddd = hash_function("func");

                    if(buff_length == 4 && strncmp(buff_start, "type", buff_length) == 0) {
                        *NextTokenMem(pool) = Token_Create(Token_Type, buff_start, buff_length); 
                    }
                    else if(buff_length == 3 && strncmp(buff_start, "var", buff_length) == 0) {
                        *NextTokenMem(pool) = Token_Create(Token_Let, buff_start, buff_length); 
                    }
                    else if(buff_length == 2 && strncmp(buff_start, "if", buff_length) == 0) {
                        *NextTokenMem(pool) = Token_Create(Token_If, buff_start, buff_length); 
                    }
                    else if(buff_length == 4 && strncmp(buff_start, "else", buff_length) == 0) {
                        *NextTokenMem(pool) = Token_Create(Token_Else, buff_start, buff_length); 
                    }
                    else if(buff_length == 5 && strncmp(buff_start, "while", buff_length) == 0) {
                        *NextTokenMem(pool) = Token_Create(Token_While, buff_start, buff_length); 
                    }
                    else if(buff_length == 4 && strncmp(buff_start, "func", buff_length) == 0) {
                        *NextTokenMem(pool) = Token_Create(Token_Func, buff_start, buff_length); 
                    }
                    else {
                        *NextTokenMem(pool) = Token_Create(Token_Identifier, buff_start, buff_length); 
                    }

                    continue;
                }
            }

            default: 
            {
                break;
            }
        }

        iterator++;
    }  

    *NextTokenMem(pool) = Token_Create(Token_EOF, NULL, 0);  

    // for (size_t i = 0; i < token_count; i++)
    // {
    //     Token* asd = &pool->bytes[i];
    //     int asdd = 0;
    //     //printf("%c%c\n", pool->bytes[i].operator_value[0], pool->bytes[i].operator_value[1]);
    //     //printf("%i %.*s\n", tokens[i].type, (int)tokens[i].string_value.length, tokens[i].string_value.start);
    // }
    
    int64 t2 = timestamp();
    printf("Tokenization: %d ms\n", t2/1000-t1/1000);

    return (Token*)pool->bytes;
}