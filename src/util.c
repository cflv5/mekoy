#include "include/util.h"
#include "include/lucretia.h"

#include <string.h>

enum conf_type get_conf_type(char *property)
{
    if (property == NULL)
    {
        return UNKNOWN;
    }
    else if (strcasecmp(property, "MASTER") == 0)
    {
        return MASTER;
    }
    else if (strcasecmp(property, "SLAVE") == 0)
    {
        return SLAVE;
    }
    else
    {
        return UNKNOWN;
    }
}

int get_uuid(char *buff, int len)
{
    uuid_t binuuid;
    uuid_generate_random(binuuid);

    if(len < UUID_STR_LEN){
        return UTIL_ERROR_INSUFFICIENT_BUFFER_LEN;
    }

    uuid_unparse_lower(binuuid, buff);

    return 0;
}

struct l_node* get_l_node_by_id(struct l_node_list *list, const char *id)
{
    int size;
    struct l_node *node = NULL;

    if(list == NULL)
    {
        return NULL;
    }

    size = list->size;

    for (size_t i = 0; i < size; i++)
    {
        if(strncmp(id, list->list[i]->id, strlen(id)) == 0)
        {
            node = list->list[i];
            break;
        }
    }
    
    return node;
}

int insert_l_node(struct l_node_list *list, struct l_node *node)
{
    if(list == NULL)
    {
        return UTIL_ERROR_NULL_POINTER;
    }

    if((list->size + 1) == list->len)
    {
        return UTIL_ERROR_ARRAY_OVERFLOW;
    }

    list->list[list->len++] = node;

    return 0;
}