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

struct l_in_addr
{
    int16_t PORT;
    in_addr_t addr;
};

struct lucretia
{
    enum conf_type type;

    struct l_in_addr in_addr;

    struct map *properties;
};

struct lucretia *new_lucretia(struct map *props);

#endif // !__LUCRETIA_H_