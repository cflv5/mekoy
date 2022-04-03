#ifndef __PUP_COMMAND_HANDLERS_H__
#define __PUP_COMMAND_HANDLERS_H__

struct pup;

#define PUP_PARAM_MAX_BUFF_LEN 1024

#include "pup.h"

#define PUP_COMMAND_HANDLER_ERROR_NULL_POINTER -1
#define PUP_COMMAND_HANDLER_ERROR_MEM_ALLOC -2

#define PUP_COMMAND_OK 0
#define PUP_COMMAND_FAIL 2

#define BUFF_SIZE 1024

int command_help_handler(struct pup *pup, const char command[][PUP_PARAM_MAX_BUFF_LEN]);
int command_connect_handler(struct pup *pup, const char command[][PUP_PARAM_MAX_BUFF_LEN]);
int command_disconnect_handler(struct pup *pup, const char command[][PUP_PARAM_MAX_BUFF_LEN]);
int command_list_op_handler(struct pup *pup, const char command[][PUP_PARAM_MAX_BUFF_LEN]);
int command_op_handler(struct pup *pup, const char command[][PUP_PARAM_MAX_BUFF_LEN]);

#endif // !__PUP_COMMAND_HANDLERS_H__