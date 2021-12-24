#include "include/lucretia.test.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

static struct map *create_map(const char *conf, const char *port);

int create_new_lucretia__given_propety_map()
{
    struct map *propeties = create_map("SLAVE", "5353");

    struct lucretia *server = new_lucretia(propeties);

    if (server->type != SLAVE || server->status != NOT_CONNEDTED ||
        server->properties != propeties || server->max_slave != 10 ||
        server->max_connection != 5)
    {
        mapClose(propeties);
        return 0;
    }

    mapClose(propeties);
    return 1;
}

int handshake__given_master_and_slave()
{
    int returnval = 1;
    int server_pid;
    int slave_pid;

    struct map *master_propeties = create_map("MASTER", "5354");
    struct map *slave_propeties = create_map("SLAVE", "5355");

    struct lucretia *master = new_lucretia(master_propeties);
    struct lucretia *slave = new_lucretia(slave_propeties);

    if ((server_pid = fork()) == 0)
    {
        l_run(master);
        exit(EXIT_SUCCESS);
    }

    if ((slave_pid = fork()) == 0)
    {
        l_run(slave);
        exit(EXIT_SUCCESS);
    }

    sleep(5);

    int r = lcp_handshake(slave, "127.0.0.1", 5354);
    if (r < 0)
    {
        returnval = 0;
    }

    if (slave->status != CONNECTED)
    {
        returnval = 0;
    }

    kill(server_pid, SIGKILL);
    kill(slave_pid, SIGKILL);

    return returnval;
}

static struct map *create_map(const char *conf, const char *port)
{
    struct map *propeties = mapNew();

    mapAdd("CONF_TYPE", (void *)conf, propeties);
    mapAdd("PORT", (void *)port, propeties);
    mapAdd("MAX_SLAVE", (void *)"10", propeties);

    return propeties;
}
