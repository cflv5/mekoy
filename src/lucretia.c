/*
 * Created at: 29.11.2021
 * Created by Bedirhan AKÇAY

 * Implementation of Lucretia master-slave server
*/

#include "include/lucretia.h"
#include "include/util.h"

#include <stdlib.h>
#include <limits.h>

static enum conf_type set_conf_type(enum conf_type type);
static u_int16_t get_port(char *portstr);
static int get_max_slave_amount(char *max_slavestr);
static struct l_node **set_slave_arr(int max_amount);

struct lucretia *new_lucretia(struct map *props)
{
    char *typestr = NULL;
    char *portstr = NULL;
    char *max_slave_amountstr = NULL;
    char *max_concurrent_connstr = NULL;

    int max_slave_amount;

    if (props == NULL)
    {
        return LUCRETIA_ERROR_PROPS_NULL;
    }

    struct lucretia *lucretia = (struct lucretia *)malloc(sizeof(struct lucretia));
    if (lucretia == NULL)
    {
        return LUCRETIA_ERROR_MEM_ALLOC;
    }

    lucretia->properties = props;
    // getting properties
    typestr = (char *)mapGet("CONF_TYPE", props);
    portstr = (char *)mapGet("PORT", props);
    max_slave_amountstr = (char *)mapGet("MAX_SLAVE", props);
    max_concurrent_connstr = (char *)mapGet("MAX_CONNECTION", props);

    // setting server internet address
    bzero((char *)&lucretia->addr, sizeof(lucretia->addr));
    lucretia->addr.sin_port = htons(get_port(portstr));
    lucretia->addr.sin_family = AF_INET;
    lucretia->addr.sin_addr.s_addr = INADDR_ANY;

    // Setting configuration type
    lucretia->type = set_conf_type(get_conf_type(typestr));

    max_slave_amount = get_max_slave_amount(max_slave_amountstr);
    max_concurrent_connstr = get_max_slave_amount(max_concurrent_connstr);

    if (lucretia->type == MASTER)
    {
        lucretia->master = NULL;
        lucretia->slaves = set_slave_arr(max_slave_amount);
        if (lucretia->slaves == NULL)
        {
            return LUCRETIA_ERROR_MEM_ALLOC;
        }

        lucretia->status = READY;

        lucretia->id_by_master = 0;
    }
    else
    {
        lucretia->slaves = NULL;
        lucretia->master = NULL;

        lucretia->status = NOT_CONNEDTED;
    }

    return lucretia;
}

static enum conf_type set_conf_type(enum conf_type type)
{
    if (type == UNKNOWN)
    {
        return SLAVE;
    }
    else
    {
        return type;
    }
}

static u_int16_t get_port(char *portstr)
{
    int port = atoi(portstr);

    if (port >= USHRT_MAX)
    {
        return USHRT_MAX - 1;
    }

    return (u_int16_t)port;
}

static int get_max_slave_amount(char *max_slavestr)
{
    int amount = atoi(max_slavestr);

    if (amount <= 0)
    {
        amount = 5;
    }

    return amount;
}

static struct l_node **set_slave_arr(int max_amount)
{
    struct l_node **slaves = (struct l_node **)malloc(max_amount * sizeof(struct l_node *));
    if (slaves == NULL)
    {
        return NULL;
    }

    struct l_node **tmp = slaves;
    for (size_t i = 0; i < max_amount; i++)
    {
        *tmp++ = NULL;
    }

    return slaves;
}