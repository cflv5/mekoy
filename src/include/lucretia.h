#ifndef __LUCRETIA_H_
#define __LUCRETIA_H_

#include "map.h"

#include <sys/types.h>
#include <netinet/in.h>

#define LUCRETIA_ERROR_PROPS_NULL -1
#define LUCRETIA_ERROR_MEM_ALLOC -2

/*
 * Configuration types of a Lucretia server.
 */
enum conf_type
{
    MASTER,
    SLAVE,
    UNKNOWN
};

typedef u_int32_t node_id;

struct l_node
{
    node_id id;
    int fd;
    struct sockaddr_in addr;
};


struct lucretia
{
    node_id id_by_master;

    enum conf_type type;

    struct sockaddr_in addr;

    struct map *properties;

    struct l_node* master;
    struct l_node** slaves;
};

struct lucretia *new_lucretia(struct map *props);

#endif // !__LUCRETIA_H_