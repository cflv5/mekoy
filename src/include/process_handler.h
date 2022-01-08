#ifndef _PROCESS_HANDLER_H__
#define _PROCESS_HANDLER_H__

struct mekoy;
struct process_message;
struct process_listener;
struct process_handler_ctx;

typedef int (*process_handler_f)(struct process_message *message, struct process_handler_ctx *ctx);

#include "process_listener.h"
#include "process_message.h"
#include "mekoy.h"

#define PROCESS_HANDLER_BUFF_SIZE 1024

#define PROCESS_HANDLER_ERROR_SERIALZATION -1

#define PROCESS_HANDLER_CODE_OK 0
#define PROCESS_HANDLER_CODE_FINISH 1
#define PROCESS_HANDLER_CODE_UNSUPPORTED_CODE 2
#define PROCESS_HANDLER_CODE_INCORRECT_TYPE_OP 3
#define PROCESS_HANDLER_CODE_ERROR_OCCURED 4


struct process_handler_ctx
{
    struct mekoy *ccf;
    struct m_process *ps;
};

/**
 * Maps defined process handler with given name
 * @param name:String - name of the process as registered
 * @return process_handler_f: handler function, might be defined some where else
*/
process_handler_f get_process_handler(char *name);

int listen_process(struct process_listener *listener, struct mekoy *ccf);

int handle_cv(struct process_message *message, struct process_handler_ctx *ctx);
int handle_lighting_control(struct process_message *message, struct process_handler_ctx *ctx);
int handle_aid_car_control(struct process_message *message, struct process_handler_ctx *ctx);
int handle_road_inform_control(struct process_message *message, struct process_handler_ctx *ctx);
int handle_lucretia_server(struct process_message *message, struct process_handler_ctx *ctx);
int handle_unsupported_process(struct process_message *message, struct process_handler_ctx *ctx);

#endif // !_PROCESS_HANDLER_H__