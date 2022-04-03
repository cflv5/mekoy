#ifndef __PUP_RES_H__
#define __PUP_RES_H__

#include <sys/types.h>

// Response code definitions START
#define PUP_RESPONSE_CODE_SUCCESS 20

#define PUP_RESPONSE_CODE_NOT_FOUND 40
#define PUP_RESPONSE_CODE_MALFORMED_REQUEST 41
#define PUP_RESPONSE_CODE_OP_NOT_DEFINED 46

#define PUP_RESPONSE_CODE_ERROR 50
// Response code definitions END

// Response error definitions START
#define PUP_RESPONSE_ERROR_NULL_POINTER -1
#define PUP_RESPONSE_ERROR_EMPTY_MESSAGE -2
// Response error definitions END

#define PUP_RESPONSE_OK 0

struct pup_res
{
    u_char stcode;
    char *body;
};

int serialize_pup_res(struct pup_res *res, char *buff, int buff_len);
int deserialize_pup_res(struct pup_res *res, char *msg, int len);

int clear_dyn_pup_res(struct pup_res **res);
int clear_pup_res(struct pup_res *res);

#endif // !__PUP_RES_H__