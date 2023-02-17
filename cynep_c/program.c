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
    int64 t1 = timestamp();

    // pthread_t thread1;
    // pthread_create(&thread1, NULL, myThreadFun, (void *)"input.cynep");
    // pthread_join(thread1, NULL);

    SourceFile* file = File_Read_Text("input.cynep");
    Token* tokens = lexer_tokenize(file);
    Statement* program = Build_SyntaxTree(tokens);

    int64 te1 = timestamp();
    VM virtualMachine;
    CodeObject codeObject = Compile((Statement*)program);
    int64 te2 = timestamp();
    printf("Compiling: %d ms\n", te2/1000-te1/1000);


    int64 t2 = timestamp();
    printf("Total: %d ms\n", t2/1000-t1/1000);
    printf("\n");

    // if (argc > 1 && strcmp(argv[1], "-tree") == 0 || true) { //|| true
    //     PrettyPrint((Statement*)program, "", true);
    // }
    printf("\n");


    RuntimeValue result = VM_exec(&virtualMachine, &codeObject);

    printf("\n%s", RuntimeValueToString(result));
}