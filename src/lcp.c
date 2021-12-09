/*
 * Created at: 02.12.2021
 * Created by Bedirhan AKÇAY

 * Implementation of Lucretia master-slave server
*/

#include "include/lcp.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

static int cpy(const char *src, char *dest, int len);
static char *d_cpy(const char *src, int *len);
static int free_str(char *str);

int serialize_lcp_req(struct lcp_req *req, char *buffer, int len)
{
    int pos = 1;
    u_int16_t *opcode = NULL;

    if (req == NULL)
    {
        return LCP_ERROR_NULL_POINTER;
    }

    buffer[0] = req->ver;

    pos += cpy(req->id, &buffer[pos], len - pos) + 1;
    pos += cpy(req->msgid, &buffer[pos], len - pos) + 1;

    opcode = (u_int16_t *)&buffer[pos];
    *opcode = htons(req->opcode);
    pos += 2;

    pos += cpy(req->header, &buffer[pos], len - pos) + 1;
    pos += cpy(req->body, &buffer[pos], len - pos) + 1;

    return pos;
}

struct lcp_req *deserialize_lcp_req(const char *message, int msize, int *size)
{
    int pos = 1;
    int npos = 0;
    u_int16_t *opcode;

    struct lcp_req *req = (struct lcp_req *)malloc(sizeof(struct lcp_req));
    if (req == NULL)
    {
        return LCP_ERROR_NULL_POINTER;
    }

    req->ver = message[0];

    req->id = d_cpy(&message[pos], &npos);
    pos += npos + 1;

    req->msgid = d_cpy(&message[pos], &npos);
    pos += npos + 1;

    opcode = &message[pos];
    req->opcode = ntohs(*opcode);
    pos += 2;

    req->header = d_cpy(&message[pos], &npos);
    pos += npos + 1;

    req->body = d_cpy(&message[pos], &npos);
    pos += npos + 1;

    *size = pos;

    return req;
}

int clear_lcp_req(struct lcp_req *req)
{
    if (req == NULL)
    {
        return LCP_ERROR_NULL_POINTER;
    }

    free_str(req->id);
    free_str(req->msgid);
    free_str(req->header);
    free_str(req->body);

    return 0;
}

int destroy_lcp_req(struct lcp_req *req)
{
    if (req == NULL)
    {
        return LCP_ERROR_NULL_POINTER;
    }

    clear_lcp_req(req);

    free(req);

    return 0;
}

int to_str_lcp_req(struct lcp_req *req, char *buff, int len)
{
    snprintf(buff, len, "req [\n\tver=%d\n\tid=\"%s\"\n\tmsgid=\"%s\"\n\topcode=%hu\n\theader=\"%s\"\n\tbody=\"%s\"\n]",
             req->ver, req->id, req->msgid, req->opcode, req->header, req->body);

    return strlen(buff);
}

static int cpy(const char *src, char *dest, int len)
{
    if (src == NULL || src[0] == 0)
    {
        *dest = 0;
    }
    else
    {
        strncpy(dest, src, len - 1);
    }

    return strlen(dest);
}

static char *d_cpy(const char *src, int *len)
{
    *len = 1;
    char *dest = NULL;

    if (src != NULL)
    {
        *len = strlen(src);
    }

    dest = (char *)malloc((*len + 1) * sizeof(char));
    if (dest != NULL)
    {
        strncpy(dest, src, *len);
    }

    return dest;
}

static int free_str(char *str)
{
    int len;
    if (str == NULL)
    {
        return 0;
    }

    len = strlen(str);

    free(str);

    return len;
}