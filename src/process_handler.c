/**
 * @author Bedirhan AKÇAY
 * @date 16.12.2021
 */

#include "include/process_handler.h"
#include "include/process_message.h"
#include "include/lcp.h"
#include "include/lucretia.h"
#include "include/util.h"

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <arpa/inet.h>
#include <stdlib.h>

struct process_handler_ctx;
struct process_message;

static int handle_common(struct process_message *message, struct process_handler_ctx *ctx);
static void cv_communication_bw_processes(char *anomaly_msg, struct process_handler_ctx *ctx);
static int send_message_to_pfd(struct m_process *ps, char code, char *message, int as_child);
static void cv_communication_bw_processes_clear(struct process_handler_ctx *ctx);
static int set_master(struct process_message *message, struct process_handler_ctx *ctx);
static u_int16_t get_port(char *portstr);

process_handler_f get_process_handler(char *name)
{
    if (strcmp(name, MEKOY_PROCESS_CV) == 0)
    {
        return handle_cv;
    }
    else if (strcmp(name, MEKOY_PROCESS_LIGHTING_CONTROL) == 0)
    {
        return handle_lighting_control;
    }
    else if (strcmp(name, MEKOY_PROCESS_AID_CAR_CONTROL) == 0)
    {
        return handle_aid_car_control;
    }
    else if (strcmp(name, MEKOY_PROCESS_ROAD_INFORM_CONTROL) == 0)
    {
        return handle_road_inform_control;
    }
    else if (strcmp(name, MEKOY_PROCESS_LUCRETIA_SERVER) == 0)
    {
        return handle_lucretia_server;
    }
    else
    {
        return handle_unsupported_process;
    }
}

int listen_process(struct process_listener *listener, struct mekoy *ccf)
{
    char buff[PROCESS_HANDLER_BUFF_SIZE];
    int pread = listener->ps->pfd[M_PROCESS_READ_END];

    process_handler_f handler;
    struct process_message *msg;

    struct process_handler_ctx ctx;
    ctx.ccf = ccf;
    ctx.ps = listener->ps;

    int size;
    int st;
    int hnd_st;

    int cont = 1;
    fprintf(stderr, "[INFO][MEKOY][LISTEN] Listening: %s\n", ctx.ps->name);
    while (cont)
    {
        size = read(pread, (void *)buff, PROCESS_HANDLER_BUFF_SIZE);

        // TODO: implement proper logger
        fprintf(stderr, "[INFO][MEKOY][PHANDLER] - %s - (%d)%s\n", ctx.ps->name, size, buff);
        msg = deserialize_process_message(buff, size, &st);
        fprintf(stderr, "[INFO][MEKOY][PHANDLER]Message desialized to: Code: %c Message: %s\n", msg->code, msg->data ? msg->data : "");

        handler = listener->handler;
        hnd_st = handler(msg, &ctx);

        if (hnd_st == PROCESS_HANDLER_CODE_FINISH)
        {
            cont = 0;
        }

        memset((void *)buff, 0, BUFF_LEN);
        free_process_message(&msg);
    }
    fprintf(stderr, "[INFO][MEKOY][LISTEN] Stopping listener job for: %s\n", ctx.ps->name);
    return 0;
}

int handle_cv(struct process_message *message, struct process_handler_ctx *ctx)
{
    char *anomaly;
    fprintf(stderr, "[ERROR][PHANDLER][CV] GOT => Code: %d(%c)\tMessage: %s\n", message->code, message->code,
            message->data ? message->data : "");
    switch (message->code)
    {
    case 'F':
        return PROCESS_HANDLER_CODE_FINISH;
        break;
    case 'D':
        anomaly = message->data;
        cv_communication_bw_processes(anomaly, ctx);
        return PROCESS_HANDLER_CODE_OK;
    case 'C':
        anomaly = message->data;
        cv_communication_bw_processes_clear(ctx);
        return PROCESS_HANDLER_CODE_OK;
    default:
        return PROCESS_HANDLER_CODE_UNSUPPORTED_CODE;
        break;
    }
    return 0;
}

int handle_lighting_control(struct process_message *message, struct process_handler_ctx *ctx)
{
    return handle_common(message, ctx);
}

int handle_aid_car_control(struct process_message *message, struct process_handler_ctx *ctx)
{
    return handle_common(message, ctx);
}

int handle_road_inform_control(struct process_message *message, struct process_handler_ctx *ctx)
{
    return handle_common(message, ctx);
}

int handle_lucretia_server(struct process_message *message, struct process_handler_ctx *ctx)
{
    char *anomalymsg;
    char msgid[UUID_STR_LEN];

    int sendmnt;

    struct lcp_req req;
    struct lucretia *lucretia;
    struct sockaddr_in *addr;

    fprintf(stderr, "[INFO][PHANDLER][LUCRETIA] GOT => Code: %d(%c)\tMessage: %s\n", message->code, message->code,
            message->data ? message->data : "");

    switch (message->code)
    {
    case 'F':
        return PROCESS_HANDLER_CODE_FINISH;
        break;
    case 'H':
        if (ctx->ccf->type == SLAVE)
        {
            if (ctx->ccf->lucretia->status == NOT_CONNEDTED)
            {
                return set_master(message, ctx);
            }
            return PROCESS_HANDLER_ERROR_SERVER_ALREADY_CONNECTED_TO_MASTER;
        }
        return PROCESS_HANDLER_ERROR_NON_MASTER_OPERATION;
    case 'A':
        if (ctx->ccf->type == SLAVE)
        {
            if (ctx->ccf->lucretia->status == CONNECTED)
            {
                fprintf(stderr, "[ERROR][PHANDLER][LUCRETIA] Lucretia server not connected to a master\n");
                return PROCESS_HANDLER_ERROR_SERVER_NOT_CONNECTED_TO_MASTER;
            }

            fprintf(stderr, "[INFO][PHANDLER][LUCRETIA] Preparing to send inform request Master\n");
            get_uuid(msgid, UUID_STR_LEN);

            anomalymsg = message->data;
            lucretia = ctx->ccf->lucretia;

            addr = &(lucretia->master->addr);
            populate_lcp_request(&req, 1, lucretia->id_by_master, msgid, L_OP_INFORM_MASTER, NULL, anomalymsg);

            fprintf(stderr, "[INFO][PHANDLER][LUCRETIA]  Sending inform request Master\n");
            sendmnt = send_lcp_request_to_addr(addr, &req);
            if (sendmnt <= 0)
            {
                fprintf(stderr, "[ERROR][PHANDLER][LUCRETIA] Error occured while sending request: %d\n", sendmnt);
                return sendmnt;
            }

            fprintf(stderr, "[INFO][PHANDLER][LUCRETIA][SEND] Message successfuly send\n");

            // TODO: wait for an acknowledge response

            return PROCESS_HANDLER_CODE_OK;
        }
        else
        {
            return PROCESS_HANDLER_CODE_INCORRECT_TYPE_OP;
        }
    case 'C':
        if (ctx->ccf->type == SLAVE)
        {
            get_uuid(msgid, UUID_STR_LEN);

            lucretia = ctx->ccf->lucretia;

            addr = &(lucretia->master->addr);
            populate_lcp_request(&req, 1, lucretia->id_by_master, msgid, L_OP_INFORM_CLEAR_MASTER, NULL, NULL);

            sendmnt = send_lcp_request_to_addr(addr, &req);
            if (sendmnt <= 0)
            {
                return PROCESS_HANDLER_CODE_ERROR_OCCURED;
            }

            // TODO: wait for an acknowledge response

            return PROCESS_HANDLER_CODE_OK;
        }
        else
        {
            return PROCESS_HANDLER_CODE_INCORRECT_TYPE_OP;
        }
    default:
        return PROCESS_HANDLER_CODE_UNSUPPORTED_CODE;
        break;
    }
    return handle_common(message, ctx);
}

int handle_unsupported_process(struct process_message *message, struct process_handler_ctx *ctx)
{
    fprintf(stderr, "[INFO][PHANDLER] Unsupported process for %s\n", ctx->ps->name);

    return handle_common(message, ctx);
}

static int handle_common(struct process_message *message, struct process_handler_ctx *ctx)
{
    switch (message->code)
    {
    case 'F':
        return PROCESS_HANDLER_CODE_FINISH;
        break;
    default:
        return PROCESS_HANDLER_CODE_UNSUPPORTED_CODE;
        break;
    }
}

static void cv_communication_bw_processes(char *anomaly_msg, struct process_handler_ctx *ctx)
{
    struct m_process *ps;

    if (ctx->ccf->type == SLAVE)
    {
        ps = mapGet(MEKOY_PROCESS_LUCRETIA_SERVER, ctx->ccf->ps);
        if (ps != NULL)
        {
            send_message_to_pfd(ps, 'A', anomaly_msg, 1);
        }
    }
    else
    {
        ps = mapGet(MEKOY_PROCESS_AID_CAR_CONTROL, ctx->ccf->ps);
        if (ps != NULL)
        {
            send_message_to_pfd(ps, 'O', NULL, 0);
        }

        ps = mapGet(MEKOY_PROCESS_ROAD_INFORM_CONTROL, ctx->ccf->ps);
        if (ps != NULL)
        {
            send_message_to_pfd(ps, 'I', anomaly_msg, 0);
        }
    }
}

static void cv_communication_bw_processes_clear(struct process_handler_ctx *ctx)
{
    struct m_process *ps;

    if (ctx->ccf->type == SLAVE)
    {
        ps = mapGet(MEKOY_PROCESS_LUCRETIA_SERVER, ctx->ccf->ps);
        if (ps != NULL)
        {
            send_message_to_pfd(ps, 'C', NULL, 1);
        }
    }
    else
    {
        ps = mapGet(MEKOY_PROCESS_AID_CAR_CONTROL, ctx->ccf->ps);
        if (ps != NULL)
        {
            send_message_to_pfd(ps, 'T', NULL, 0);
        }

        ps = mapGet(MEKOY_PROCESS_ROAD_INFORM_CONTROL, ctx->ccf->ps);
        if (ps != NULL)
        {
            send_message_to_pfd(ps, 'C', NULL, 0);
        }
    }
}

static int send_message_to_pfd(struct m_process *ps, char code, char *message, int as_child)
{
    struct process_message msg;

    msg.code = code;
    msg.data = message;
    fprintf(stderr, "[DEBUG][PHANDLER][SEND]Code: %d\tMessage: %s\n", msg.code, message ? message : "");
    if (as_child)
    {
        return send_process_message_as_child(&msg, ps);
    }

    return send_process_message(&msg, ps);
}

static int set_master(struct process_message *message, struct process_handler_ctx *ctx)
{
    char *msg = message->data;
    char *addr = strchr(msg, ':');
    char *port = strchr(addr + 1, ':');
    
    struct lucretia *server = ctx->ccf->lucretia;

    struct l_node *master = (struct l_node *)malloc(sizeof(struct l_node));
    if (master == NULL)
    {
        return PROCESS_HANDLER_ERROR_MEM_ALLOCATION;
    }

    if (addr == NULL)
    {
        fprintf(stderr, "[ERROR][PHANDLER][LUCRETIA][SET_MASTER] No address defined\n");
        return PROCESS_HANDLER_ERROR_NO_ADDR_DEFINED;
    }

    if (port == NULL)
    {
        fprintf(stderr, "[ERROR][PHANDLER][LUCRETIA][SET_MASTER] No port defined\n");
        return PROCESS_HANDLER_ERROR_NO_PORT_DEFINED;
    }

    *addr = 0;
    addr++;
    fprintf(stderr, "[INFO][PHANDLER][LUCRETIA][SET_MASTER] Id: %s\n", msg);
    server->id_by_master = (char *)malloc(UUID_STR_LEN * sizeof(char));
    memset((void *)server->id_by_master, 0, UUID_STR_LEN);
    strncpy(server->id_by_master, msg, UUID_STR_LEN - 1);

    *port = 0;
    port++;
    fprintf(stderr, "[INFO][PHANDLER][LUCRETIA][SET_MASTER] Address: %s\n", addr);
    fprintf(stderr, "[INFO][PHANDLER][LUCRETIA][SET_MASTER] Port: %s\n", port);
    bzero((char *)&master->addr, sizeof(master->addr));
    master->addr.sin_family = AF_INET;
    master->addr.sin_addr.s_addr = inet_addr(addr);
    master->addr.sin_port = htons(get_port(port));

    ctx->ccf->lucretia->master  = master;

    return PROCESS_HANDLER_OK;
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