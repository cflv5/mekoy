#ifndef __PUP__H__
#define __PUP__H__

#define PUP_PARAM_MAX_SIZE 5
#define PUP_PARAM_MAX_BUFF_LEN 1024

struct pup;

typedef int (*command_handler_f)(struct pup *pup, const char params[][PUP_PARAM_MAX_BUFF_LEN]);

#include "../../include/list.h"
#include "../../include/pup_req.h"
#include "../../include/pup_res.h"
#include "pup_command_handlers.h"

#include <arpa/inet.h>

#define PUP_ERROR_MEM_ALLOC -1
#define PUP_ERROR_CONF_NO_PASS_PHRASE_DEFINED -2
#define PUP_ERROR_NULL_POINTER -3
#define PUP_ERROR_LIST_CREATION -4
#define PUP_ERROR_ACQUIRING_PARAMS -5

#define PUP_STATUS_OK 0
#define PUP_STATUS_END 1
#define PUP_STATUS_FILTER_FAILED 2

#define PUP_CONNECTION_ESTABLISHED 0
#define PUP_CONNECTION_NOT_ESTABLISHED 1

struct conn
{
    struct sockaddr_in addr;
    int sockfd;
    int is_established;
};

enum pup_command
{
    HELP, LIST, CONNECT, OP, DISCONNECT, EXIT
};

struct pup_command_ctx
{
    enum pup_command command;
    char *name;
    int need_auth;
    command_handler_f handler;
};

struct pup
{
    struct conn curr_conn;
    struct list *filters;
    struct list *commands;
    int is_client_authenticated;
    int version;
};

int pup_run(void);

#endif // !__PUP__H__