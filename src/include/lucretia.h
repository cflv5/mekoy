#ifndef __LUCRETIA_H_
#define __LUCRETIA_H_

#include "map.h"

#include <sys/types.h>
#include <netinet/in.h>

#define LUCRETIA_ERROR_PROPS_NULL -1
#define LUCRETIA_ERROR_MEM_ALLOC -2
#define LUCRETIA_ERROR_NON_MASTER_OPERATION -3
#define LUCRETIA_ERROR_SOCKET_CREATION -4
#define LUCRETIA_ERROR_CONNECTION -5
#define LUCRETIA_ERROR_REQUEST_SERIALIZATION -6
#define LUCRETIA_ERROR_SEND_OPERATION -7
#define LUCRETIA_ERROR_CONNECTION_SHUTDOWN -8
#define LUCRETIA_ERROR_HANDSHAKE_MISMACHED_RESPONSE_VALUES -9
#define LUCRETIA_ERROR_HANDSHAKE_MASTER_REJECTS -10

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

enum l_status
{
    READY,
    CONNECTED,
    NOT_CONNEDTED,
    DISCONNECTED
};

struct lucretia
{
    node_id id_by_master;
    enum l_status status;

    enum conf_type type;

    struct sockaddr_in addr;

    struct map *properties;

    struct l_node* master;
    struct l_node** slaves;
};

struct lucretia *new_lucretia(struct map *props);
int lcp_handshake(struct lucretia *server, const char *address, in_port_t port)

#endif // !__LUCRETIA_H_