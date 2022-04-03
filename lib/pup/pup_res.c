/**
 * @author Bedirhan AKÇAY
 * @date 12/01/2021
 */

#include "pup_res.h"

#include <string.h>
#include <stdlib.h>

int serialize_pup_res(struct pup_res *res, char *buff, int buff_len)
{
    int null_body = 0;

    if (res == NULL)
    {
        return PUP_RESPONSE_ERROR_NULL_POINTER;
    }

    memset(buff, 0, buff_len);

    buff[0] = res->stcode;

    if (res->body == NULL)
    {
        null_body = 1;
        res->body = "";
    }

    strncpy(&buff[1], res->body, buff_len - 2);

    if (null_body == 1)
    {
        res->body = NULL;
    }

    return strlen(buff) + 1;
}

int deserialize_pup_res(struct pup_res *res, char *msg, int len)
{
    int mlen;

    if (res == NULL)
    {
        return PUP_RESPONSE_ERROR_NULL_POINTER;
    }

    mlen = strlen(msg);
    len = mlen > len ? len : mlen;

    if (len == 0)
    {
        return PUP_RESPONSE_ERROR_EMPTY_MESSAGE;
    }

    res->stcode = *msg++;

    res->body = (char *)malloc(len * sizeof(char));
    memset((void *)res->body, 0, len);
    strncpy(res->body, msg, len - 1);

    return PUP_RESPONSE_OK;
}

int clear_pup_res(struct pup_res *res)
{
    if (res == NULL || res->body == NULL)
    {
        return PUP_RESPONSE_ERROR_NULL_POINTER;
    }

    free(res->body);

    return PUP_RESPONSE_OK;
}
int clear_dyn_pup_res(struct pup_res **res)
{
    int clear_result = clear_pup_res(*res);
    if ( clear_result != 0)
    {
        return clear_result;
    }

    free(*res);
    *res = NULL;

    return PUP_RESPONSE_OK;
}
