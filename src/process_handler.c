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

struct process_handler_ctx;
struct process_message;

static int handle_common(struct process_message *message, struct process_handler_ctx *ctx);
static void cv_communication_bw_processes(char *anomaly_msg, struct process_handler_ctx *ctx);
static int send_message_to_pfd(struct m_process *ps, char code, char *message);

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
    while (cont)
    {
        size = read(pread, (void *)buff, PROCESS_HANDLER_BUFF_SIZE);

        // TODO: implement proper logger
        fprintf(stderr, "[INFO][MEKOY][PHANDLER] - %s - %s\n", ctx.ps->name, buff);
        msg = deserialize_process_message(buff, size, &st);

        handler = listener->handler;
        hnd_st = handler(msg, &ctx);

        if (hnd_st == PROCESS_HANDLER_CODE_FINISH)
        {
            cont = 0;
        }

        free_process_message(&msg);
    }

    return 0;
}

int handle_cv(struct process_message *message, struct process_handler_ctx *ctx)
{
    char *anomaly;
    switch (message->code)
    {
    case 'F':
        return PROCESS_HANDLER_CODE_FINISH;
        break;
    case 'D':
        anomaly = message->data;
        cv_communication_bw_processes(anomaly, ctx);
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

    switch (message->code)
    {
    case 'F':
        return PROCESS_HANDLER_CODE_FINISH;
        break;
    case 'A':
        if (ctx->ccf->type == SLAVE)
        {
            get_uuid(msgid, UUID_STR_LEN);

            anomalymsg = message->data;
            lucretia = ctx->ccf->lucretia;

            addr = &(lucretia->addr);
            populate_lcp_request(&req, 1, lucretia->id_by_master, msgid, L_OP_INFORM_MASTER, NULL, anomalymsg);

            sendmnt = send_lcp_request_to_addr(addr, &req);
            if (sendmnt <= 0)
            {
                return PROCESS_HANDLER_CODE_ERROR_OCCURED;
            }

            //TODO: wait for an acknowledge response
            
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
            send_message_to_pfd(ps, 'A', anomaly_msg);
        }
    }
    else
    {
        ps = mapGet(MEKOY_PROCESS_AID_CAR_CONTROL, ctx->ccf->ps);
        if (ps != NULL)
        {
            send_message_to_pfd(ps, 'S', NULL);
        }

        ps = mapGet(MEKOY_PROCESS_ROAD_INFORM_CONTROL, ctx->ccf->ps);
        if (ps != NULL)
        {
            send_message_to_pfd(ps, 'I', anomaly_msg);
        }
    }
}

static int send_message_to_pfd(struct m_process *ps, char code, char *message)
{
    char buff[PROCESS_HANDLER_BUFF_SIZE];
    struct process_message msg;

    msg.code = code;
    msg.data = message;

    return send_process_message(&msg, ps);
}