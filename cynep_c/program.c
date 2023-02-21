#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#include "helper/defines.c"
#include "helper/diagnostics.c"
#include "helper/list.c"
#include "helper/file.c"
#include "helper/smart_array.c"

#include "frontend/lexer.c"
#include "frontend/ast.c"
#include "frontend/parser.c"


#include "backend/vm.c"
#include "backend/compiler.c"


void *parseTask(void *vargp)
{
    SourceFile* file = File_Read_Text((char*)vargp);
    Token* tokens = lexer_tokenize(file);
}

int main(int argc, char**argv) {
    // DynArray arr = DynArray_Create(1, sizeof(uint64_t));
    // uint64_t val = 5;
    // uint64_t val2 = 10;
    // DynArray_Append(&arr, &val);
    // DynArray_Append(&arr, &val2);
    // uint64_t test = arr.array[1 * arr.segment_size];


    int64 t1 = timestamp();

    // pthread_t thread1;
    // pthread_create(&thread1, NULL, myThreadFun, (void *)"input.cynep");
    // pthread_join(thread1, NULL);

    SourceFile* file = File_Read_Text("input.cynep");
    Token* tokens = lexer_tokenize(file);
    Statement* program = Build_SyntaxTree(tokens);

    // if (argc > 1 && strcmp(argv[1], "-ast") == 0|| true) { //|| true
    //     printf("\n---------------- ABSTRACT SYNTAX TREE ----------------\n\n");
    //     PrettyPrint((Statement*)program, "", true);
    // }
    // printf("\n");

    int64 te1 = timestamp();

    Global* global = Create_Global();
    Global_Add(global, "leet", 1337);
    Global_Add(global, "y", 7331);

    CodeObject codeObject = Compile((Statement*)program, global);

    int64 te2 = timestamp();
    printf("Compiling: %d ms\n", te2/1000-te1/1000);

    int64 t2 = timestamp();
    printf("Total: %d ms\n", t2/1000-t1/1000);




    // printf("\n------------------ MAIN DISASSEMBLY ------------------\n\n");
    // Disassemble(&codeObject, global);


    VM virtualMachine;
    RuntimeValue result = VM_exec(&virtualMachine, global, &codeObject);



    printf("Execution result: %s", RuntimeValueToString(result));



    // printf("\n");
    // int64 tesr1 = timestamp();
    // int a = 0;
    // for (size_t i = 0; i < 1000000; i++)
    // {
    //     if(50 > 10){
    //         a++;
    //     }
    // }
    
    // int64 tesr2 = timestamp();
    // printf("C Execution time: %d ms\n", tesr2/1000-tesr1/1000);
}