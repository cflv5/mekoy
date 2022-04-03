/**
 * @author Bedirhan AKÇAY
 * @date 16.01.2022
 */

#include "include/pup_filters.h"

#include <stdio.h>

int pup_filter_is_auth(struct pup *pup, struct pup_filter_ctx *ctx, struct pup_command_ctx *command_ctx)
{
    if (command_ctx->need_auth && !pup->is_client_authenticated)
    {
        printf(" >> Unauthorized!\n");
        return PUP_FILTER_FAILED;
    }

    return PUP_FILTER_PASSED;
}

int pup_filter_is_connected(struct pup *pup, struct pup_filter_ctx *ctx, struct pup_command_ctx *command_ctx)
{
    if (command_ctx->command == CONNECT || command_ctx->command == HELP || command_ctx->command == EXIT)
    {
        return PUP_FILTER_PASSED;
    }

    if (pup->curr_conn.is_established == PUP_CONNECTION_ESTABLISHED && pup->curr_conn.sockfd != -1)
    {
        return PUP_FILTER_PASSED;
    }

    printf(">> Pup client is not connected to a Puppeteer server.\n");
    return PUP_FILTER_FAILED;
}