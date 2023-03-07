#pragma once

typedef enum TokenType TokenType;
typedef struct Token Token; 
typedef struct BufferString BufferString; 
typedef enum ParseState ParseState;

enum TokenType {
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
    ParseState_Symbol,
    ParseState_Number,
    ParseState_String
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
    char* string;
};

#define ALPHA \
         'A': \
    case 'B': \
    case 'C': \
    case 'D': \
    case 'E': \
    case 'F': \
    case 'G': \
    case 'H': \
    case 'I': \
    case 'J': \
    case 'K': \
    case 'L': \
    case 'M': \
    case 'N': \
    case 'O': \
    case 'P': \
    case 'Q': \
    case 'R': \
    case 'S': \
    case 'T': \
    case 'U': \
    case 'V': \
    case 'W': \
    case 'X': \
    case 'Y': \
    case 'Z': \
    case 'a': \
    case 'b': \
    case 'c': \
    case 'd': \
    case 'e': \
    case 'f': \
    case 'g': \
    case 'h': \
    case 'i': \
    case 'j': \
    case 'k': \
    case 'l': \
    case 'm': \
    case 'n': \
    case 'o': \
    case 'p': \
    case 'q': \
    case 'r': \
    case 's': \
    case 't': \
    case 'u': \
    case 'v': \
    case 'w': \
    case 'x': \
    case 'y': \
    case 'z'

#define DIGIT \
         '1': \
    case '2': \
    case '3': \
    case '4': \
    case '5': \
    case '6': \
    case '7': \
    case '8': \
    case '9': \
    case '0'

unsigned long hash_function(char* str) 
{
    unsigned long i = 0;

    for (int j = 0; str[j]; j++)
        i += str[j];

    return i % 50000;
}

Token Token_Create(TokenType type, char* start, size_t length, Arena* arena) {
    Token token;
    token.type = type;
    token.string_value.length = length;
    token.string_value.start = start;

    token.string = arena_alloc(arena, sizeof(char) * (length + 1));
    strncpy(token.string, start, length);
    token.string[length] = NULL_CHAR;

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

    Arena* arena = arena_create(500 * sizeof(Token));

    MemPool* pool = MemPool_Make(sizeof(Token) * tokens_max);
    size_t pos = 0;

    ParseState state = ParseState_Start;

    char* buff_start = 0;
    size_t buff_length = 0;

    while(file_buffer[pos] != NULL_CHAR) {
        char* current = &file_buffer[pos];
        char* lookahead = &file_buffer[pos + 1];

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
                        if(*lookahead == '=')                        {
                            *NextTokenMem(pool) = Token_Operator_Create(Token_ComparisonOperator, current_and_lookahead_as_string);  
                            pos++;
                        } else{
                            // Some kind of negation operator
                        }
                        break;
                    }
                    case '>': {   
                        if(*lookahead == '=') { 
                            *NextTokenMem(pool) = Token_Operator_Create(Token_ComparisonOperator, current_and_lookahead_as_string);                  
                            pos++;
                        } else {
                            *NextTokenMem(pool) = Token_Operator_Create(Token_ComparisonOperator, current_as_string); 
                        }
                        break;
                    }
                    case '<': {   
                        if(*lookahead == '=') {
                            *NextTokenMem(pool) = Token_Operator_Create(Token_ComparisonOperator, current_and_lookahead_as_string);  
                            pos++;
                        } else {
                            *NextTokenMem(pool) = Token_Operator_Create(Token_ComparisonOperator, current_as_string); 
                        }
                        
                        break;
                    }
                    case '=': {   
                        if(*lookahead == '=') {
                            *NextTokenMem(pool) = Token_Operator_Create(Token_ComparisonOperator, current_and_lookahead_as_string);  
                            pos++;
                        } else {
                            *NextTokenMem(pool) = Token_Operator_Create(Token_Assignment, current_as_string); 
                        }
                        break;
                    }
                    case '"': {       
                        state = ParseState_String;
                        buff_start = current + 1;
                        buff_length = 0;
                        break;
                    }
                    case ALPHA: {
                        state = ParseState_Symbol;
                        buff_start = current;
                        buff_length = 0;
                        continue;
                    }
                    case DIGIT: {
                        state = ParseState_Number;
                        buff_start = current;
                        buff_length = 0;
                        continue;
                    }
                    default: {
                        if (Is_Skippable(*current)) {
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

            case ParseState_String: {
                switch (*current)
                {
                    case '"': {
                        state = ParseState_Start;
                        *NextTokenMem(pool) = Token_Create(Token_String, buff_start, buff_length, arena); 
                        break;
                    }
                    default: {
                        buff_length++;
                        break;
                    }
                }
                break;
            }

            case ParseState_Symbol: { 
                switch (*current)
                {
                    case ALPHA: {
                        buff_length++;
                        break;
                    }  
                    default: {
                        state = ParseState_Start;

                        if(buff_length == 4 && strncmp(buff_start, "type", buff_length) == 0) {
                            *NextTokenMem(pool) = Token_Create(Token_Type, buff_start, buff_length, arena); 
                        }
                        else if(buff_length == 3 && strncmp(buff_start, "var", buff_length) == 0) {
                            *NextTokenMem(pool) = Token_Create(Token_Let, buff_start, buff_length, arena); 
                        }
                        else if(buff_length == 2 && strncmp(buff_start, "if", buff_length) == 0) {
                            *NextTokenMem(pool) = Token_Create(Token_If, buff_start, buff_length, arena);
                        }
                        else if(buff_length == 4 && strncmp(buff_start, "else", buff_length) == 0) {
                            *NextTokenMem(pool) = Token_Create(Token_Else, buff_start, buff_length, arena); 
                        }
                        else if(buff_length == 5 && strncmp(buff_start, "while", buff_length) == 0) {
                            *NextTokenMem(pool) = Token_Create(Token_While, buff_start, buff_length, arena); 
                        }
                        else if(buff_length == 4 && strncmp(buff_start, "func", buff_length) == 0) {
                            *NextTokenMem(pool) = Token_Create(Token_Func, buff_start, buff_length, arena); 
                        }
                        else {
                            *NextTokenMem(pool) = Token_Create(Token_Identifier, buff_start, buff_length, arena); 
                        }

                        continue;
                    }
                }
                break;
            }

            case ParseState_Number: {
                switch (*current)
                {
                    case DIGIT: {
                        buff_length++;
                        break;
                    }
                    default: {
                        state = ParseState_Start;

                        char asd[buff_length];
                        strncpy(asd, buff_start, buff_length);
                        asd[buff_length] = NULL_CHAR;
                        float64 parsed_number = atoi(asd); 

                        *NextTokenMem(pool) = Token_Number_Create(Token_Number, parsed_number);

                        continue;
                    }
                }
                break;
            }

            default: 
            {
                break;
            }
        }

        pos++;
    }  

    *NextTokenMem(pool) = Token_Create(Token_EOF, NULL, 0, arena);  

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