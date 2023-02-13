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

void PrettyPrint(Statement* node, char* indent, bool isLast){
    char* marker;
    
    if(isLast)
        marker = "└───";
    else
        marker = "├───";

    SSList* children = Get_Children(node);

    printf("%s", indent);
    printf("%s", marker);
    Print_Node(node);
    printf("\n");

    char *new_indent = (char*)malloc(strlen(indent) + 8);
    strcpy(new_indent, indent);

    if(isLast)
    {
        strcat(new_indent, "    ");     

        // if(children->first != NULL){
        //     printf("%s│\n", new_indent);
        // }
    }
    else
        strcat(new_indent, "│   ");

    Statement* lastChild;

    if(children->first == NULL)
        lastChild = NULL;
    else
        lastChild = (Statement*)children->last->value;
    
    SSNode* cursor = children->first;
    while(cursor != NULL){
        PrettyPrint(cursor->value, new_indent, cursor->next == NULL);
        cursor = cursor->next;
    }

    // Cleanup
    SSList_Free(children);
    free(new_indent);
}

int main(int argc, char**argv) {
    int64 t1 = timestamp();

    SourceFile* file = File_Read_Text("input.cynep");

    Token* tokens = lexer_tokenize(file);
    Statement* program = Build_SyntaxTree(tokens);

    if (argc > 1 && strcmp(argv[1], "-tree") == 0 || true) { //|| true
        PrettyPrint((Statement*)program, "", true);
    }

    int64 t2 = timestamp();
    printf("Total: %d ms\n", t2/1000-t1/1000);
}