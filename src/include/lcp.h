#ifndef __LCP_H_
#define __LCP_H_

#include <sys/types.h>

#define LCP_ERROR_NULL_POINTER -1

#define L_OP_HANDSHAKE 1

struct lcp_req
{
    u_char ver;
    char *id;
    char *msgid;
    u_int16_t opcode;
    char *header; // "key:val;key:val;key:val"
    char *body; 
};

int serialize_lcp_req(struct lcp_req* req, char* buffer, int len);
int deserialize_lcp_req(struct lcp_req* req, const char* message, int msize);
int clear_lcp_req(struct lcp_req *req);
int destroy_lcp_req(struct lcp_req *req);
int to_str_lcp_req(struct lcp_req *req, char *buff, int len);


#endif // !__LCP_H_