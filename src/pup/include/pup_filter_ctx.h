#ifndef __PUP_FILTER_CTX_H__
#define __PUP_FILTER_CTX_H__

struct pup_filter_ctx;
struct pup;
struct pup_command_ctx;

typedef int (*pup_filter_f)(struct pup *pup, struct pup_filter_ctx *ctx, struct pup_command_ctx *command_ctx);

#include "pup.h"

enum pup_filter
{
    AUTH, CONNECTED
};

struct pup_filter_ctx
{
    enum pup_filter type;
    pup_filter_f f;
};

#endif // !__PUP_FILTER_CTX_H__