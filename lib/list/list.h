#ifndef __LIST_H__
#define __LIST_H__

#define LIST_CODE_OK 0

#define LIST_ERROR_NULL_POINTER -1
#define LIST_ERROR_MEMMORY_ALLOCATION -2

#define LIST_ITERATOR_ERROR_LIST_NOT_DEFINED -1
#define LIST_ITERATOR_ERROR_INDEX_OUT_OF_BOUNDRY -2

struct node;
struct list;

struct list_iterator;

struct list *create_list();
int insert_list(struct list *list, void *data);

void *list_get_node_data(struct node *node);

struct list_iterator *create_list_iterator(struct list *list);
int list_iterator_next(struct list_iterator *iterator);
struct node* list_iterator_get_node(struct list_iterator *iterator);

#endif // !__LIST_H__