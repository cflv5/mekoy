#include "include/dispatch.h"
#include "include/lucretia.h"
#include "include/lcp_handler.h"

#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_TRY 5

static int inform_unsupported_operation(struct lucretia *server, struct lcp_req *oreq, int sockfd);

int dispatch(struct lucretia *server, struct lcp_req *req, int sockfd, struct sockaddr_in req_addr)
{
    int try;
    int return_code;
    if (req == NULL)
    {
        return LCP_ERROR_NULL_POINTER;
    }

    switch (req->opcode)
    {
    case L_OP_HANDSHAKE:
        return_code = handle_lcp_handshake(server, sockfd, req_addr, req);

        switch (return_code)
        {
        case LUCRETIA_ERROR_NON_SLAVE_OPERATION:
            // TODO: handle errors
            break;
        default:
            break;
        }

        return return_code;
    case L_OP_INFORM_MASTER:
        return_code = handle_lcp_informed(server, sockfd, req_addr, req);

        return return_code;
    default:
        try = 0;
        while ((return_code = inform_unsupported_operation(server, req, sockfd) < 0) && try < MAX_TRY)
            try++;
        return 0;
    }
}

static int inform_unsupported_operation(struct lucretia *server, struct lcp_req *oreq, int sockfd)
{
    struct lcp_req req;
    int nsend;
    int msglen;

    char buff[LCP_BUFF_SIZE];

    req.ver = 1;
    req.id = oreq->id;
    req.msgid = oreq->msgid;
    req.opcode = L_OP_UNSUPPORTED_OPERATION;
    req.header = NULL;
    req.body = "UNSUPPORTED OPERATION";

    msglen = serialize_lcp_req(&req, buff, LCP_BUFF_SIZE);
    if (msglen <= 0)
    {
        return LUCRETIA_ERROR_REQUEST_SERIALIZATION;
    }

    nsend = send(sockfd, (void *)buff, msglen, 0);
    if (nsend < 0)
    {
        return LUCRETIA_ERROR_SEND_OPERATION;
    }

    close(sockfd);

    return 0;
}