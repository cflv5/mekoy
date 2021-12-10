#ifndef __UTIL_C_
#define __UTIL_C_

#include "lucretia.h"

#define UTIL_ERROR_INSUFFICIENT_BUFFER_LEN -1
#define UTIL_ERROR_NULL_POINTER -2
#define UTIL_ERROR_ARRAY_OVERFLOW -3

enum conf_type get_conf_type(char *property);
int get_uuid(char *buff, int len);

struct l_node* get_l_node_by_id(struct l_node_list *list, const char *id);
int insert_l_node(struct l_node_list *list, struct l_node *node);
#endif // !__UTIL_C_