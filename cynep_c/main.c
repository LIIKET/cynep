#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#include "util/defines.c"
#include "util/diagnostics.c"
#include "util/list.c"
#include "util/file.c"
#include "util/array.c"
#include "util/arena.c"

#include "frontend/lexer.c"
#include "frontend/ast.c"
#include "frontend/parser.c"

#include "backend/runtime.c"
#include "backend/compiler.c"

bool arg(int argc, char**argv, char* search){
    for (uint64 i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], search) == 0){
            return true;
        }     
    }  

    return false;
}

RuntimeValue Multiply(size_t argc, RuntimeValue* argv){

    RuntimeValue arg1 = argv[0];
    RuntimeValue arg2 = argv[1];

    // printf("NATIVE CALL: %f\n", arg2.number);

    float64 result = arg1.number * arg2.number;

    return NUMBER(result);
}

int main(int argc, char**argv) 
{
    bool show_ast = arg(argc, argv, "-ast");
    bool show_disassemble = arg(argc, argv, "-dis");

    int64 total_begin = timestamp();

    size_t asd = sizeof(AstNode);

    TextFile* file = read_entire_file("input.cynep");
    Token* tokens = lexer_tokenize(file);
    AstNode* program = Build_SyntaxTree(tokens);

    // Setup global object
    Global* global = Create_Global();
    Global_Add(global, "VERSION", NUMBER(0.1));
    Global_AddNativeFunction(global, "multiply", &Multiply, 2);



    // Compile
    CodeObject codeObject = Compile((AstNode*)program, global);

    int64 total_end = timestamp();
    printf("Total: %d ms\n", total_end/1000-total_begin/1000);

    if (show_ast) 
        ast_print_begin((AstNode*)program);
    
    if (show_disassemble) 
        Disassemble(global);

    // Start execution
    VM virtualMachine;
    RuntimeValue result = VM_exec(&virtualMachine, global, &codeObject);

    printf("Execution result: %s", RuntimeValue_ToString(result));
}

#pragma region THREADING

void *parseTask(void *vargp)
{
    TextFile* file = read_entire_file((char*)vargp);
    Token* tokens = lexer_tokenize(file);
}

void startThread(){
    pthread_t thread1;
    pthread_create(&thread1, NULL, parseTask, (void *)"input.cynep");
    pthread_join(thread1, NULL);
}

#pragma endregion