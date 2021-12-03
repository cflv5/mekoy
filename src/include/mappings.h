#ifndef __MAPPINGS_H_
#define __MAPPINGS_H_

#include "lcp.h"
#include "lucretia.h"

int mapper(struct lucretia *server, struct lcp_req *req, int sockfd, struct sockaddr_in req_addr);

#endif // !__MAPPINGS_H_