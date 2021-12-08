#ifndef __DISPATCH_H_
#define __DISPATCH_H_

#include "lcp.h"
#include "lucretia.h"

int dispatch(struct lucretia *server, struct lcp_req *req, int sockfd, struct sockaddr_in req_addr);

#endif // !__DISPATCH_H_