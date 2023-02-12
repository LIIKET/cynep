#pragma once

typedef struct SSNode SSNode;

struct SSNode 
{
    void* value;
    SSNode* next;
    SSNode* prev;
};

typedef struct SSList 
{
    SSNode* first;
    SSNode* last;
} SSList;

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
}

SSList* SSList_Create(SSList* list)
{
    list->first = NULL;
    list->last = NULL;

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
    SSNode* headNode = list->first;
    SSNode* currentNode;

    // Cannot free cause they are allocated in chunks
    // while (headNode != NULL){
    //     currentNode = headNode;
    //     headNode = headNode->next;
    //     free(currentNode);
    // }

    free(list);
}



// typedef struct ListNode 
// {
//     void* value;

// } ListNode;

// typedef struct List 
// {
//     size_t nodes_max;
//     size_t nodes_count;
//     ListNode* nodes;
    
// } List;


// ListNode* _List_Mem_Seed(List* list){
//     if(list->nodes_count == list->nodes_max-1){
//         list->nodes_max *= 2;
//         list->nodes = (ListNode*)realloc(list->nodes, sizeof(ListNode) * list->nodes_max);
//     }

//     return &list->nodes[list->nodes_count++];
// }

// List* Create_List() 
// {
//     List* list = (List*)malloc(sizeof(List));

//     list->nodes_max = 100000000; 
//     list->nodes_count = 0;
//     list->nodes = NULL;

//     list->nodes = (ListNode*)malloc(sizeof(ListNode) * list->nodes_max);

//     return list;
// }

// ListNode* Create_ListNode(List* list, void* value)
// {
//     ListNode* node = _List_Mem_Seed(list);
//     //node->next = NULL;
//     node->value = value;

//     return node;
// }

// List* list_destroy(){

// }

// void List_Append(List* list, ListNode* node)
// {
//     list->nodes[list->nodes_count+1];
// }












// list_node_t* list_first(list_t* list)
// {
//     return list->_first;
// }

// list_node_t* list_last(list_t* list)
// {
//     return list->_last;
// }

// void list_prepend(list_t* list, list_node_t* node)
// {
//     list_node_t* old_first = list->_first;
    
//     node->next = old_first;
//     list->_first = node;

//     if(list->_last == NULL){
//         list->_last = node;
//     }
// }

// void list_append(list_t* list, list_node_t* node)
// {
//     list_node_t* old_last = list->_last;

//     if(old_last != NULL){
//         old_last->next = node;
//     }    

//     list->_last = node;

//     if(list->_first == NULL){
//         list->_first = node;
//     }
// }

// list_node_t* list_remove_first(list_t* list){
//      struct list_node_t * old_first = list->_first;

//      if(old_first != NULL){
//          list->_first = old_first->next;
//          old_first->next = NULL;
//      }

//      return old_first;
//  }

// struct ListNode* list_add_last(){

// }

// struct ListNode* list_remove_last(){

// }
