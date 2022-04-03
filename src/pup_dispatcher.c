/**
 * @author Bedirhan AKÇAY
 * @date 12.01.2021
 */

#include "include/m_process.h"
#include "include/pup_dispatcher.h"

#include <string.h>
#include <stdlib.h>

// DELETE
#include <stdio.h>

static int handle_op_list(struct puppeteer *puppeteer, struct pup_req *req, struct pup_res *res);
static int handle_op_generic(struct puppeteer *puppeteer, struct pup_req *req, struct pup_res *res);

int dispatch_pup_req(struct puppeteer *puppeteer, struct pup_req *req, struct pup_res *res)
{
    int st;
    if (puppeteer == NULL || req == NULL || res == NULL)
    {
        return PUP_DISPATCH_ERROR_NULL_POINTER;
    }

    switch (req->opcode)
    {
    case PUP_REQUEST_OP_CODE_LIST:
        st = handle_op_list(puppeteer, req, res);
        if (st != 0)
        {
            // TODO: log status of handle
        }
        return st;
    case PUP_REQUEST_OP_CODE_OP:
        st = handle_op_generic(puppeteer, req, res);
        if (st != 0)
        {
            // TODO: log status of handle
        }
        return st;
    default:
        break;
    }

    return PUP_STATUS_OK;
}

static int handle_op_list(struct puppeteer *puppeteer, struct pup_req *req, struct pup_res *res)
{
    struct map *ps;
    struct mapIterator *iterator;
    char *body;
    struct m_process *pr;
    int size;

    if (puppeteer == NULL || req == NULL || res == NULL)
    {
        return PUP_DISPATCH_ERROR_NULL_POINTER;
    }

    if (puppeteer->ccf == NULL)
    {
        return PUP_DISPATCH_ERROR_NO_MEKOY_ATTACHED;
    }

    ps = puppeteer->ccf->ps;
    iterator = getMapIterator(ps);
    if (iterator == NULL)
    {
        return PUP_DISPATCH_ERROR_PROCESS_LIST;
    }

    body = (char *)malloc(BUFF_LEN);
    bzero((void *)body, BUFF_LEN);

    size = 0;
    while (mapIteratorNext(iterator) == 0)
    {
        pr = (struct m_process *)getIteratorVal(iterator);
        if (pr != NULL && pr->is_system_pr != 1)
        {
            strncat((body + size), pr->name, BUFF_LEN - size);
            size += strlen(pr->name);
            strncat((body + size), "\n", BUFF_LEN - size);
            size++;
        }
    }

    res->stcode = PUP_RESPONSE_CODE_SUCCESS;
    res->body = body;

    return 0;
}

static int handle_op_generic(struct puppeteer *puppeteer, struct pup_req *req, struct pup_res *res)
{
    struct mapIterator *iterator;
    struct m_process *pr = NULL;
    struct process_message msg;
    int found;
    char *name;
    char *pmsg;
    int st = 0;

    if (puppeteer == NULL || req == NULL || res == NULL)
    {
        return PUP_DISPATCH_ERROR_NULL_POINTER;
    }

    if (puppeteer->ccf == NULL)
    {
        return PUP_DISPATCH_ERROR_NO_MEKOY_ATTACHED;
    }

    if (req->data == NULL || req->data[0] == '\0')
    {
        return PUP_DISPATCH_ERROR_NULL_POINTER;
    }

    iterator = getMapIterator(puppeteer->ccf->ps);
    if (iterator == NULL)
    {
        return PUP_DISPATCH_ERROR_PROCESS_LIST;
    }

    // make request immutable
    name = strchr(req->data, '\n');
    *name = 0;
    pmsg = name + 1;
    name = req->data;

    found = 0;

    // DELETE
    while (mapIteratorNext(iterator) == 0 && found == 0)
    {
        pr = (struct m_process *)getIteratorVal(iterator);
        if (pr != NULL && strcmp(pr->name, name) == 0)
        {
            found = 1;
        }
    }

    if (pr != NULL && found == 1)
    {
        msg.code = pmsg[0];
        msg.data = (char *)malloc(BUFF_LEN);
        bzero((void *)msg.data, BUFF_LEN);
        memcpy(msg.data, pmsg + 1, strlen(pmsg + 1));
        msg.data[strlen(msg.data) - 1] = 0;

        st = send_process_message(&msg, pr);

        free(msg.data);

        res->body = NULL;
        res->stcode = PUP_RESPONSE_CODE_SUCCESS;
    }
    else
    {
        res->stcode = PUP_RESPONSE_CODE_NOT_FOUND;
        res->body = NULL;
    }

    return st;
}
