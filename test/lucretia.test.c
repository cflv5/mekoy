#include "include/lucretia.test.h"

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

static struct map *create_map(const char *conf, const char *port)
{
    struct map *propeties = mapNew();

    mapAdd("CONF_TYPE", conf, propeties);
    mapAdd("PORT", port, propeties);
    mapAdd("MAX_SLAVE", "10", propeties);

    return propeties;
}