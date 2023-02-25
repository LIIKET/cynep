#pragma once

typedef struct SSNode SSNode;
typedef struct SSList SSList;

struct SSNode 
{
    void* value;
    SSNode* next;
    SSNode* prev;
};

struct SSList 
{
    SSNode* first;
    SSNode* last;
    size_t count;
};

void SSList_Append(SSList* list, SSNode* node)
{
    if(list->first == NULL){
        list->first = node;
    }
    else{
        list->last->next = node;
    }

    node->prev = list->last;
    list->last = node;

    list->count++;
}

SSList* SSList_Create(SSList* list)
{
    list->first = NULL;
    list->last = NULL;
    list->count = 0;

    return list;
}

SSNode* SSNode_Create(SSNode* node, void* value)
{
    node->next = NULL;
    node->prev = NULL;
    node->value = value;

    return node;
}

void SSList_Free(SSList* list)
{
    // SSNode* headNode = list->first;
    // SSNode* currentNode;

    // Cannot free cause they are allocated in chunks
    // TODO: Set freeable property on ones created only for display purposes and free them
    // while (headNode != NULL){
    //     currentNode = headNode;
    //     headNode = headNode->next;
    //     free(currentNode);
    // }

    // free(list);
}