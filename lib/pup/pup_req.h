#ifndef __PUP_REQ_H__
#define __PUP_REQ_H__

#include <sys/types.h>

#define PUP_REQUEST_OP_CODE_LIST 21
#define PUP_REQUEST_OP_CODE_OP   11

#define PUP_REQUEST_ERROR_NULL_POINTER  -1
#define PUP_REQUEST_ERROR_EMPTY_MESSAGE -2

#define PUP_REQUEST_OK 0

struct pup_req
{
    u_char opcode;
    char *data;
};

int serialize_pup_req(struct pup_req *req, char *buff, int buff_len);
int deserialize_pup_req(struct pup_req *req, char *msg, int len);

int clear_dyn_pup_req(struct pup_req **req);
int clear_pup_req(struct pup_req *req);


#endif // !__PUP_REQ_H__