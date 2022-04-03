#ifndef __PUP_FILTERS_H__
#define __PUP_FILTERS_H__

#include "pup_filter_ctx.h"

#define PUP_FILTER_PASSED 0
#define PUP_FILTER_FAILED 1

int pup_filter_is_auth(struct pup *pup, struct pup_filter_ctx *ctx, struct pup_command_ctx *command_ctx);
int pup_filter_is_connected(struct pup *pup, struct pup_filter_ctx *ctx, struct pup_command_ctx *command_ctx);

#endif // !__PUP_FILTERS_H__