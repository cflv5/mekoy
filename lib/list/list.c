/**
 * @author: Bedirhan AKÇAY
 * @date: 18.12.2021
 * 
 * Link list implementation
*/

#include "list.h"

#include <stdlib.h>

struct node
{
    void *data;
    struct node *next;
};

struct list
{
    struct node *data;
    int len;
};

struct list *create_list()
{
    struct list *list = (struct list*)malloc(sizeof(struct list));
    if(list == NULL)
    {
        return NULL;
    }

    list->data = NULL;
    list->len = 0;

    return list;
}

int insert_list(struct list *list, void *data)
{
    struct node *tmp;
    struct node *node;
    int i;

    if(list == NULL)
    {
        return LIST_ERROR_NULL_POINTER;
    }

    node = (struct node*)malloc(sizeof(struct node));
    if (node == NULL)
    {
        return LIST_ERROR_MEMMORY_ALLOCATION;
    }
    node->data = data;
    node->next = NULL;
    
    tmp = list->data;
    for (i = 0; i < list->len; i++)
    {
        tmp = node->next;
    }
    
    if (tmp == NULL)
    {
        list->data = node;
    }
    else
    {
        tmp->next = node;
    }
    
    list->len++;

    return LIST_CODE_OK;
}