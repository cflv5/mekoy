/**
 * @author Bedirhan AKÇAY
 * @date 06.01.2022
 */

#include "include/lcp_handler.h"
#include "include/m_process.h"
#include "include/map.h"

static int send_message_to_pfd_as_child(struct m_process *ps, char code, char *message);

int handle_lcp_informed(struct lucretia *server, int sockfd, struct sockaddr_in req_addr, struct lcp_req *original_req)
{
    if (server->type != MASTER)
    {
        return 1; // NON SLAVE OP
    }

    char *anomaly_msg = original_req->body;
    
    struct m_process *pr = (struct m_process *)mapGet(MEKOY_PROCESS_CV, server->ccf->ps);

    if (pr != NULL)
    {
        send_message_to_pfd_as_child(pr, 'D', anomaly_msg);
    }

    return 0;
}

int handle_lcp_informed_clear(struct lucretia *server, int sockfd, struct sockaddr_in req_addr, struct lcp_req *original_req)
{
    if (server->type != MASTER)
    {
        return 1; // NON SLAVE OP
    }

    struct m_process *pr = (struct m_process *)mapGet(MEKOY_PROCESS_CV, server->ccf->ps);

    if (pr != NULL)
    {
        send_message_to_pfd_as_child(pr, 'C', NULL);
    }

    return 0;
}


static int send_message_to_pfd_as_child(struct m_process *ps, char code, char *message)
{
    struct process_message msg;

    msg.code = code;
    msg.data = message;

    return send_process_message_as_child(&msg, ps);
}