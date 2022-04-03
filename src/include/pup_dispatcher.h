#ifndef __PUP_DISPATCHER_H__
#define __PUP_DISPATCHER_H__

#include "puppeteer.h"
#include "pup_req.h"
#include "pup_res.h"

#define PUP_DISPATCH_ERROR_NULL_POINTER -1
#define PUP_DISPATCH_ERROR_NO_MEKOY_ATTACHED -2
#define PUP_DISPATCH_ERROR_PROCESS_LIST -3

#define PUP_STATUS_OK 0

int dispatch_pup_req(struct puppeteer *puppeteer, struct pup_req *req, struct pup_res *res);

#endif // !__PUP_DISPATCHER_H__