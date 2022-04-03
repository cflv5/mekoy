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

struct list_iterator
{
    struct list *list;
    int ind;
    struct node *node;
};

struct list *create_list()
{
    struct list *list = (struct list *)malloc(sizeof(struct list));
    if (list == NULL)
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

    if (list == NULL)
    {
        return LIST_ERROR_NULL_POINTER;
    }

    node = (struct node *)malloc(sizeof(struct node));
    if (node == NULL)
    {
        return LIST_ERROR_MEMMORY_ALLOCATION;
    }
    node->data = data;
    node->next = NULL;

    tmp = list->data;
    if (tmp == NULL)
    {
        list->data = node;
    }
    else
    {
        while (tmp->next != NULL)
        {
            tmp = tmp->next;
        }

        tmp->next = node;
    }

    list->len++;

    return LIST_CODE_OK;
}

void *list_get_node_data(struct node *node)
{
    void *data = NULL;
    if (node != NULL)
    {
        data = node->data;
    }
    return data;
}

struct list_iterator *create_list_iterator(struct list *list)
{
    struct list_iterator *iterator = NULL;

    if (list == NULL)
    {
        return NULL;
    }

    iterator = (struct list_iterator *)malloc(sizeof(struct list_iterator));
    if (iterator == NULL)
    {
        return NULL;
    }

    iterator->list = list;
    iterator->ind = -1;
    iterator->node = NULL;

    return iterator;
}

int list_iterator_next(struct list_iterator *iterator)
{
    struct list *list;
    if (iterator == NULL)
    {
        return 1;
    }

    list = iterator->list;
    if (list == NULL)
    {
        return LIST_ITERATOR_ERROR_LIST_NOT_DEFINED;
    }

    if (list->len == 0 || (iterator->ind + 1) >= list->len)
    {
        return LIST_ITERATOR_ERROR_INDEX_OUT_OF_BOUNDRY;
    }
    else
    {
        if (iterator->ind == -1)
        {
            iterator->node = list->data;
        }
        else
        {
            iterator->node = iterator->node->next;
        }
        iterator->ind++;
    }

    return 0;
}

struct node *list_iterator_get_node(struct list_iterator *iterator)
{
    void *node = NULL;

    if (iterator != NULL && iterator->node != NULL)
    {
        node = iterator->node;
    }

    return node;
}