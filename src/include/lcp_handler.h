#ifndef __LCP_HANDLER__H
#define __LCP_HANDLER__H

#include "lucretia.h"
#include "lcp.h"

#include <netinet/in.h>

int handle_lcp_informed(struct lucretia *server, int sockfd, struct sockaddr_in req_addr, struct lcp_req *original_req);
int handle_lcp_informed_clear(struct lucretia *server, int sockfd, struct sockaddr_in req_addr, struct lcp_req *original_req);

#endif // !__LCP_HANDLER__H