/**
 * @author Bedirhan AKÇAY
 * @date 16.01.2022
 */

#include "include/pup_command_handlers.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <arpa/inet.h>

static int call_puppeteer(int sockfd, struct pup_req *req, struct pup_res *res);

int command_help_handler(struct pup *pup, const char command[][PUP_PARAM_MAX_BUFF_LEN])
{
    const char *help_message = "Structure: [COMMAND] ...[PARAMS]\nAvaliable commands:\n"
                               " - help: prints help message.\n"
                               " - connect [IP] [PORT]: makes TCP connection to Puppeteer server using specified ip and port.\n"
                               " - disconnect: disconnects from connected Puppeteer server.\n"
                               " - list: lists avaliable subsystem names.\n"
                               " - op [SYSTEM_NAME] [OP_NAME] ...[PARAMS]: sends operation to be executed by a subsystem specified in SYSTEM_NAME.\n"
                               " - exit: shuts down Pup client.\n";
    fprintf(stdout, "%s", help_message);
    return PUP_COMMAND_OK;
}

int command_list_op_handler(struct pup *pup, const char command[][PUP_PARAM_MAX_BUFF_LEN])
{
    struct pup_req req;
    struct pup_res res;

    req.opcode = PUP_REQUEST_OP_CODE_LIST;
    req.data = NULL;

    if (call_puppeteer(pup->curr_conn.sockfd, &req, &res) == 0)
    {
        fprintf(stdout, ">> List of avaliable subsystems:\n%s\n", res.body);
    }

    return PUP_COMMAND_OK;
}
int command_connect_handler(struct pup *pup, const char command[][PUP_PARAM_MAX_BUFF_LEN])
{
    int sockfd;

    if (command[1][0] == '\0' || command[2][0] == '\0')
    {
        fprintf(stdout, ">> Please provide Puppeteer server address and port.\n");
        return PUP_COMMAND_OK;
    }

    if (pup->curr_conn.is_established == PUP_CONNECTION_ESTABLISHED || pup->curr_conn.sockfd != -1)
    {
        fprintf(stdout, ">> Pup client already connected to: %s:%u\n", inet_ntoa(pup->curr_conn.addr.sin_addr), ntohs(pup->curr_conn.addr.sin_port));
        return PUP_COMMAND_OK;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <= 0)
    {
        fprintf(stderr, "[ERROR][PUP_CLIENT][HANDLER][CONNECT] Could not create socket for '%s:%s'\n", command[1], command[2]);
        return PUP_ERROR_MEM_ALLOC;
    }

    bzero((char *)&pup->curr_conn.addr, sizeof(pup->curr_conn.addr));
    pup->curr_conn.addr.sin_family = AF_INET;
    pup->curr_conn.addr.sin_addr.s_addr = inet_addr(command[1]);
    pup->curr_conn.addr.sin_port = htons(atoi(command[2]));

    if (connect(sockfd, (const struct sockaddr *)&pup->curr_conn.addr, sizeof(struct sockaddr)) < 0)
    {
        fprintf(stderr, "[ERROR][LCP][SEND] Could not connect to '%s:%s'\n", command[1], command[2]);
        bzero((char *)&pup->curr_conn.addr, sizeof(pup->curr_conn.addr));
        close(sockfd);
        return PUP_ERROR_MEM_ALLOC;
    }

    pup->curr_conn.sockfd = sockfd;
    pup->curr_conn.is_established = PUP_CONNECTION_ESTABLISHED;

    fprintf(stdout, ">> Pup client successfuly connected to: %s:%u\n", inet_ntoa(pup->curr_conn.addr.sin_addr), ntohs(pup->curr_conn.addr.sin_port));

    return PUP_COMMAND_OK;
}

int command_disconnect_handler(struct pup *pup, const char command[][PUP_PARAM_MAX_BUFF_LEN])
{
    close(pup->curr_conn.sockfd);

    fprintf(stdout, ">> Pup client successfuly disconnected from: %s:%u\n", inet_ntoa(pup->curr_conn.addr.sin_addr), ntohs(pup->curr_conn.addr.sin_port));

    bzero((char *)&pup->curr_conn.addr, sizeof(pup->curr_conn.addr));
    pup->curr_conn.sockfd = -1;
    pup->curr_conn.is_established = PUP_CONNECTION_NOT_ESTABLISHED;

    return PUP_COMMAND_OK;
}

int command_op_handler(struct pup *pup, const char command[][PUP_PARAM_MAX_BUFF_LEN])
{
    struct pup_req req;
    struct pup_res res;

    if (command[1][0] == '\0')
    {
        fprintf(stdout, ">> Please enter a subsystem name.\n");
        return PUP_COMMAND_OK;
    }
    if (command[2][0] == '\0')
    {
        fprintf(stdout, ">> Please enter operation parameter\n");
        return PUP_COMMAND_OK;
    }

    req.opcode = PUP_REQUEST_OP_CODE_OP;
    req.data = (char *)malloc(BUFF_SIZE);
    if (req.data != NULL)
    {
        snprintf(req.data, BUFF_SIZE, "%s\n%s\n", command[1], command[2]);
    }

    if (call_puppeteer(pup->curr_conn.sockfd, &req, &res) == 0)
    {
        fprintf(stdout, ">> Operation successfuly run on server.\n");
    }

    return PUP_COMMAND_OK;
}

static int call_puppeteer(int sockfd, struct pup_req *req, struct pup_res *res)
{
    size_t n;
    char buff[BUFF_SIZE];

    if ((n = serialize_pup_req(req, buff, BUFF_SIZE)) <= 0)
    {
        fprintf(stderr, "[ERROR][PUP_CLIENT][REQUEST_SERIALIZE] An error occured while serializing pup request: %ld\n", n);
        fprintf(stdout, ">> An error occured.\n");
        return PUP_COMMAND_FAIL;
    }

    n = write(sockfd, buff, n);

    if (n <= 0)
    {
        fprintf(stderr, "[ERROR][PUP_CLIENT][SEND_REQUEST] An error occured while sending the pup request: %ld\n", n);
        fprintf(stdout, ">> An error occured. Could not send the request.\n");
        return PUP_COMMAND_FAIL;
    }

    bzero((void *)buff, BUFF_SIZE);
    if ((n = recv(sockfd, buff, BUFF_SIZE, 0)) > 0)
    {
        n = deserialize_pup_res(res, buff, n);
        if (n != 0)
        {
            fprintf(stderr, "[ERROR][PUP_CLIENT][REQUEST_SERIALIZE] An error occured while serializing pup response: %ld\n", n);
            fprintf(stdout, ">> An error occured.\n");
            return PUP_COMMAND_FAIL;
        }
        fprintf(stderr, "[INFO][PUP_CLIENT][RESPONSE] pup_res status code: %d\n", res->stcode);
        return PUP_COMMAND_OK;
    }
    else
    {
        return PUP_COMMAND_FAIL;
    }
}