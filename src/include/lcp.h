#ifndef __LCP_H_
#define __LCP_H_

#include <sys/types.h>
#include <netinet/in.h>

#define LCP_ERROR_NULL_POINTER -1
#define LCP_ERROR_REQUEST_SERIALIZATION -2
#define LCP_ERROR_SEND_OPERATION -3

#define L_OP_HANDSHAKE 1
#define L_OP_UNSUPPORTED_OPERATION 99

#define LCP_BUFF_SIZE 1024

struct lcp_req
{
    u_char ver;
    char *id;
    char *msgid;
    u_int16_t opcode;
    char *header; // "key:val;key:val;key:val"
    char *body;
};

int serialize_lcp_req(struct lcp_req *req, char *buffer, int len);
struct lcp_req *deserialize_lcp_req(const char *message, int msize, int *size);
int clear_lcp_req(struct lcp_req *req);
int destroy_lcp_req(struct lcp_req *req);
int to_str_lcp_req(struct lcp_req *req, char *buff, int len);

int send_lcp_request(int sockfd, struct lcp_req *req);
int send_lcp_request_to_addr(struct sockaddr_in *addr, struct lcp_req *req);
int populate_lcp_request(struct lcp_req *req, u_char ver, const char *id,
                const char *msgid, u_int16_t opcode, const char *header, 
                const char *body);
struct lcp_req *create_lcp_request(u_char ver, const char *id, const char *msgid, 
                                    u_int16_t opcode, const char *header, 
                                    const char *body);               

#endif // !__LCP_H_