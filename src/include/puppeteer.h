#ifndef __PUPPETEER_H__
#define __PUPPETEER_H__

struct mekoy;

#include "map.h"
#include "mekoy.h"

#include <sys/socket.h>

#define BUFF_LEN 1024

#define PUPPETEER_ERROR_CONFIGURATION_MISSING -1
#define PUPPETEER_ERROR_MEMMORY_ALLOCATION -2
#define PUPPETEER_ERROR_CONF_PORT_NOT_DEFINED -3
#define PUPPETEER_ERROR_NULL_POINTER -4
#define PUPPETEER_ERROR_SOCKET_CREATION -5
#define PUPPETEER_ERROR_SOCKET_BINDING -6
#define PUPPETEER_ERROR_SOCKET_LISTENING -7

#define PUPPETEER_STATUS_OK 0

#define PUPPETEER_MAX_CONNECTION 2

struct puppeteer
{
    struct map *conf;
    struct mekoy *ccf;

    int sockfd;
    struct sockaddr_in addr;

};

struct puppeteer *create_puppeteer(struct map *conf, int *status);
int run_puppeteer(struct puppeteer *puppeteer);

#endif // !__PUPPETEER_H__