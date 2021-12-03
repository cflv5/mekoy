#include "include/lcp.test.h"

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int to_str_lcp_req__given_lcp_req()
{
    int len;
    char lcp_str_buff[BUFF_LEN];
    memset((void *)lcp_str_buff, 0, BUFF_LEN);

    char *expected = "req [\n\tver=1\n\tid=\"id\"\n\tmsgid=\"msgid\"\n\topcode=99\n\theader=\"header\"\n\tbody=\"body\"\n]";

    struct lcp_req req;
    req.ver = 1;
    req.id = "id";
    req.msgid = "msgid";
    req.opcode = 99;
    req.header = "header";
    req.body = "body";

    len = to_str_lcp_req(&req, lcp_str_buff, BUFF_LEN);
    if (len <= 0)
    {
        return 0;
    }

    fprintf(stdout, "Expected:\n%s\n", expected);
    fprintf(stdout, "Returned:\n%s\n", lcp_str_buff);

    return 1;
}

int serialize_lcp_req__given_lcp_req()
{
    char buff[BUFF_LEN];
    memset((void *)buff, 0, BUFF_LEN);

    struct lcp_req req;
    req.ver = 1;
    req.id = "id";
    req.msgid = "msgid";
    req.opcode = 99;
    req.header = "header";
    req.body = "body";

    int len = serialize_lcp_req(&req, buff, BUFF_LEN);

    if (len < 0)
    {
        return 0;
    }

    return 1;
}

int deserialize_lcp_req__given_lcp_req()
{
    int pos = 1;
    u_int16_t *p_opcode = NULL;

    char message[BUFF_LEN];
    memset((void *)message, 0, BUFF_LEN);

    char str_req[BUFF_LEN];
    memset((void *)str_req, 0, BUFF_LEN);

    struct lcp_req req;

    message[0] = 1;

    strncpy(&message[pos], "id", 4);
    pos += 3;

    strncpy(&message[pos], "msgid", 7);
    pos += 6;

    p_opcode = (u_int16_t *)&message[pos];
    *p_opcode = htons(99);
    pos += 2;

    message[pos] = 0;
    pos++;
    // strncpy(&message[pos], "header", 7);
    // pos += 7;

    strncpy(&message[pos], "body", 6);
    pos += 5;

    if (deserialize_lcp_req(&req, message, BUFF_LEN) < 0)
        return 0;

    to_str_lcp_req(&req, str_req, BUFF_LEN);
    fprintf(stdout, "\nDeserialized req:\n%s", str_req);

    return 1;
}
