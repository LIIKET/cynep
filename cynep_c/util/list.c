#pragma once

typedef struct ListNode ListNode;
typedef struct List List;

struct ListNode {
    void* value;
    ListNode* next;
    ListNode* prev;
};

struct List {
    ListNode* first;
    ListNode* last;
    size_t count;
};

void list_append(List* list, ListNode* node) {
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

List* list_create(List* list) {
    list->first = NULL;
    list->last = NULL;
    list->count = 0;

    return list;
}

ListNode* listNode_create(ListNode* node, void* value) {
    node->next = NULL;
    node->prev = NULL;
    node->value = value;

    return node;
}