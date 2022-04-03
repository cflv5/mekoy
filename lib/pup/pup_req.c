/**
 * @author Bedirhan AKÇAY
 * @date 12/01/2021
 */

#include "pup_req.h"

#include <string.h>
#include <stdlib.h>

int serialize_pup_req(struct pup_req *req, char *buff, int buff_len)
{
    if (req == NULL)
    {
        return PUP_REQUEST_ERROR_NULL_POINTER;
    }

    memset(buff, 0, buff_len);

    buff[0] = req->opcode;

    if (req->data == NULL)
    {
        req->data = "";
    }

    strncpy(&buff[1], req->data, buff_len - 2);

    return strlen(buff) + 1;
}

int deserialize_pup_req(struct pup_req *req, char *msg, int len)
{
    int mlen;

    if (req == NULL)
    {
        return PUP_REQUEST_ERROR_NULL_POINTER;
    }

    mlen = strlen(msg);
    len = mlen > len ? len : mlen;

    if (len == 0)
    {
        return PUP_REQUEST_ERROR_EMPTY_MESSAGE;
    }

    req->opcode = *msg++;

    req->data = (char *)malloc(len * sizeof(char));
    memset((void *)req->data, 0, len);
    strncpy(req->data, msg, len - 1);

    return PUP_REQUEST_OK;
}

int clear_pup_req(struct pup_req *req)
{
    if (req == NULL || req->data == NULL)
    {
        return PUP_REQUEST_ERROR_NULL_POINTER;
    }

    free(req->data);

    return PUP_REQUEST_OK;
}
int clear_dyn_pup_req(struct pup_req **req)
{
    int clear_result = clear_pup_req(*req);
    if ( clear_result != 0)
    {
        return clear_result;
    }

    free(*req);
    *req = NULL;

    return PUP_REQUEST_OK;
}
