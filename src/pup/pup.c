/**
 * @author Bedirhan AKÇAY
 * @date 15.01.2022
 */

#include "include/pup.h"
#include "include/pup_filter_ctx.h"
#include "include/pup_filters.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFF_LEN 1024

int connect_to_addr(const char *ip, in_port_t port);
static struct pup *create_pup(int *st);
static int split_params(char params[][PUP_PARAM_MAX_BUFF_LEN], char *command);
static int process_command(struct pup *pup, char *command, int len);
static struct pup_command_ctx *find_pup_command_by_name(char *name, struct list *commands);
static int apply_filters(struct pup *pup, struct pup_command_ctx *ctx);
static int command_exit_handler(struct pup *pup, const char command[][PUP_PARAM_MAX_BUFF_LEN]);

int main(int argc, char const *argv[])
{
    int st = pup_run();
    if (st != 0)
    {
        fprintf(stderr, "[ERROR][PUP_CLIENT] Pup client failed with status: %d\n", st);
        exit(st);
    }

    fprintf(stderr, "[INFO][PUP_CLIENT] Pup client successfuly shut down.\n");
    return 0;
}

int pup_run(void)
{
    int st;
    int cont;

    size_t len;

    char *buff = NULL;

    size_t n;

    struct pup *pup = create_pup(&st);
    if (pup == NULL)
    {
        return st;
    }

    char *pass_phrase = getenv("PUP_CLIENT_PASS");
    if (pass_phrase == NULL)
    {
        free(pup);
        return PUP_ERROR_CONF_NO_PASS_PHRASE_DEFINED;
    }

    do
    {
        buff = NULL;
        fprintf(stdout, ">> Password: ");
        n = getline(&buff, &len, stdin);
        buff[n - 1] = 0;

        if (strcmp(pass_phrase, buff) == 0)
        {
            pup->is_client_authenticated = 1;
        }

        free(buff);
    } while (pup->is_client_authenticated == 0);

    fprintf(stdout, "Pup Client v. %d\n", pup->version);
    fprintf(stdout, "Type help for avaliable commands\n");

    cont = 1;
    while (cont)
    {
        buff = NULL;
        fprintf(stdout, ">> ");
        n = getline(&buff, &len, stdin);

        st = process_command(pup, buff, n);
        if (st == PUP_STATUS_END)
        {
            cont = 0;
        }
        else if (st == PUP_STATUS_FILTER_FAILED)
        {
            fprintf(stdout, "[INFO][PUP_CLIENT] A filter failed.\n");
        }
        else if (st < 0)
        {
            fprintf(stdout, "[ERROR][PUP_CLIENT] An error with code '%d'.\n", st);
        }

        free(buff);
    }

    return 0;
}

static int process_command(struct pup *pup, char *command, int len)
{
    int st;
    char params[PUP_PARAM_MAX_SIZE][PUP_PARAM_MAX_BUFF_LEN];

    if (split_params(params, command))
    {
        return PUP_ERROR_ACQUIRING_PARAMS;
    }

    struct pup_command_ctx *ctx = find_pup_command_by_name(params[0], pup->commands);
    if (ctx == NULL)
    {
        fprintf(stdout, ">> Command '%s' not found.\n", params[0]);
    }
    else
    {
        if (apply_filters(pup, ctx) != PUP_FILTER_PASSED)
        {
            return PUP_STATUS_FILTER_FAILED;
        }

        st = ctx->handler(pup, params);
        return st;
    }

    return PUP_STATUS_OK;
}

static int apply_filters(struct pup *pup, struct pup_command_ctx *ctx)
{
    struct node *node;
    struct pup_filter_ctx *filter_ctx;

    int i;

    pup_filter_f filter_f;

    struct list_iterator *iterator = create_list_iterator(pup->filters);
    if (iterator != NULL)
    {
        while (list_iterator_next(iterator) == 0)
        {
            node = list_iterator_get_node(iterator);
            filter_ctx = (struct pup_filter_ctx *)list_get_node_data(node);
            if (filter_ctx != NULL)
            {
                filter_f = filter_ctx->f;
                if ((i = filter_f(pup, filter_ctx, ctx)) == PUP_FILTER_FAILED)
                {
                    return PUP_FILTER_FAILED;
                }
            }
        }
    }
    return PUP_FILTER_PASSED;
}

static int split_params(char params[][PUP_PARAM_MAX_BUFF_LEN], char *command)
{
    int is_str;
    int i;
    int k;
    int j;
    int cont;
    for (i = 0; i < PUP_PARAM_MAX_SIZE; i++)
    {
        bzero((void *)(params[i]), PUP_PARAM_MAX_BUFF_LEN);
    }

    j      = 0;
    k      = 0;
    i      = 0;
    cont   = 1;
    is_str = 0;
    while (cont == 1)
    {
        if (i == PUP_PARAM_MAX_SIZE )
        {
            cont = 0;
        }
        else if(k == PUP_PARAM_MAX_BUFF_LEN)
        {
            i++;
            k = 0;
        }
        else if (command[j] == ' ')
        {
            if (is_str != 1)
            {
                i++;
                j++;
                k = 0;
            }
            else 
            {
                params[i][k++] = command[j++];
            }
        }
        else if (command[j] == '"')
        {
            is_str = is_str == 1 ? 0 : 1;
            j++;
        }
        else if (command[j] == '\0' || command[j] == '\n' )
        {
            params[i][k] = 0;
            cont = 0;
        }
        else
        {
            params[i][k++] = command[j++];
        }
    }

    return 0;
}

static struct pup_command_ctx *find_pup_command_by_name(char *name, struct list *commands)
{
    struct pup_command_ctx *ctx;

    int cont;
    int found;

    struct list_iterator *iterator = create_list_iterator(commands);
    if (iterator == NULL)
    {
        return NULL;
    }

    cont = 1;
    found = 0;
    while (list_iterator_next(iterator) == 0 && cont == 1)
    {
        struct node *node = list_iterator_get_node(iterator);
        ctx = list_get_node_data(node);
        if (ctx != NULL && strcmp(ctx->name, name) == 0)
        {
            cont = 0;
            found = 1;
        }
    }

    return found == 1 ? ctx : NULL;
}

static struct pup *create_pup(int *st)
{
    struct pup_command_ctx *command;
    struct pup_filter_ctx *filter;
    *st = 0;
    struct pup *pup = (struct pup *)malloc(sizeof(struct pup));
    if (pup == NULL)
    {
        *st = PUP_ERROR_MEM_ALLOC;
        return NULL;
    }

    pup->version = 1; // TODO: get it from conf. file

    pup->curr_conn.is_established = PUP_CONNECTION_NOT_ESTABLISHED;
    pup->curr_conn.sockfd = -1;

    pup->commands = create_list();
    if (pup->commands == NULL)
    {
        free(pup);
        *st = PUP_ERROR_LIST_CREATION;
        return NULL;
    }

    command = (struct pup_command_ctx *)malloc(sizeof(struct pup_command_ctx));
    if (command == NULL)
    {
        *st = PUP_ERROR_NULL_POINTER;
        free(pup);
        return NULL;
    }
    command->name = "help";
    command->command = HELP;
    command->handler = command_help_handler;
    command->need_auth = 0;
    insert_list(pup->commands, (void *)command);

    command = (struct pup_command_ctx *)malloc(sizeof(struct pup_command_ctx));
    if (command == NULL)
    {
        *st = PUP_ERROR_NULL_POINTER;
        free(pup);
        return NULL;
    }
    command->name = "list";
    command->command = LIST;
    command->handler = command_list_op_handler;
    command->need_auth = 1;
    insert_list(pup->commands, (void *)command);

    command = (struct pup_command_ctx *)malloc(sizeof(struct pup_command_ctx));
    if (command == NULL)
    {
        *st = PUP_ERROR_NULL_POINTER;
        free(pup);
        return NULL;
    }
    command->name = "connect";
    command->command = CONNECT;
    command->handler = command_connect_handler;
    command->need_auth = 0;
    insert_list(pup->commands, (void *)command);

    command = (struct pup_command_ctx *)malloc(sizeof(struct pup_command_ctx));
    if (command == NULL)
    {
        *st = PUP_ERROR_NULL_POINTER;
        free(pup);
        return NULL;
    }
    command->name = "op";
    command->command = OP;
    command->handler = command_op_handler;
    command->need_auth = 1;
    insert_list(pup->commands, (void *)command);

    command = (struct pup_command_ctx *)malloc(sizeof(struct pup_command_ctx));
    if (command == NULL)
    {
        *st = PUP_ERROR_NULL_POINTER;
        free(pup);
        return NULL;
    }
    command->name = "disconnect";
    command->command = DISCONNECT;
    command->handler = command_disconnect_handler;
    command->need_auth = 1;
    insert_list(pup->commands, (void *)command);

    command = (struct pup_command_ctx *)malloc(sizeof(struct pup_command_ctx));
    if (command == NULL)
    {
        *st = PUP_ERROR_NULL_POINTER;
        free(pup);
        return NULL;
    }
    command->name = "exit";
    command->command = EXIT;
    command->handler = command_exit_handler;
    command->need_auth = 1;
    insert_list(pup->commands, (void *)command);

    // FILTERS
    pup->filters = create_list();
    if (pup->filters == NULL)
    {
        free(pup->commands);
        free(pup);
        *st = PUP_ERROR_LIST_CREATION;
        return NULL;
    }

    filter = (struct pup_filter_ctx *)malloc(sizeof(struct pup_filter_ctx));
    if (filter == NULL)
    {
        *st = PUP_ERROR_NULL_POINTER;
        free(pup);
        return NULL;
    }
    filter->type = AUTH;
    filter->f = pup_filter_is_auth;
    insert_list(pup->filters, (void *)filter);

    filter = (struct pup_filter_ctx *)malloc(sizeof(struct pup_filter_ctx));
    if (filter == NULL)
    {
        *st = PUP_ERROR_NULL_POINTER;
        free(pup);
        return NULL;
    }
    filter->type = CONNECTED;
    filter->f = pup_filter_is_connected;
    insert_list(pup->filters, (void *)filter);

    pup->is_client_authenticated = 0;

    return pup;
}

static int command_exit_handler(struct pup *pup, const char command[][PUP_PARAM_MAX_BUFF_LEN])
{
    return PUP_STATUS_END;
}
