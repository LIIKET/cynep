#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "util/defines.c"
#include "util/diagnostics.c"
#include "util/list.c"
#include "util/file.c"
#include "util/array.c"

#include "frontend/lexer.c"
#include "frontend/ast.c"
#include "frontend/parser.c"

#include "backend/runtime.c"
#include "backend/compiler.c"

bool arg(int argc, char**argv, char* search){
    return false;
    for (uint64 i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], search) == 0){
            return true;
        }     
    }  

    return false;
}

int main(int argc, char**argv) 
{
    bool show_ast = arg(argc, argv, "-ast");
    bool show_disassemble = arg(argc, argv, "-dis");

    int64 total_begin = timestamp();

    SourceFile* file = File_Read_Text("input.cynep");
    Token* tokens = lexer_tokenize(file);
    Statement* program = Build_SyntaxTree(tokens);

    // Setup global object
    Global* global = Create_Global();
    Global_Add(global, "leet", 1337);
    Global_Add(global, "y", 7331);

    // Compile
    CodeObject codeObject = Compile((Statement*)program, global);

    int64 total_end = timestamp();
    printf("Total: %d ms\n", total_end/1000-total_begin/1000);

    if (show_ast) 
        Prinst_AST((Statement*)program);
    
    if (show_disassemble) 
        Disassemble(&codeObject, global);

    // Start execution
    VM virtualMachine;
    RuntimeValue result = VM_exec(&virtualMachine, global, &codeObject);

    printf("Execution result: %s", RuntimeValue_ToString(result));
}

#pragma region THREADING

void *parseTask(void *vargp)
{
    SourceFile* file = File_Read_Text((char*)vargp);
    Token* tokens = lexer_tokenize(file);
}

void startThread(){
    pthread_t thread1;
    pthread_create(&thread1, NULL, parseTask, (void *)"input.cynep");
    pthread_join(thread1, NULL);
}

#pragma endregion