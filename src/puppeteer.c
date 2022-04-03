/**
 * @author Bedirhan AKÇAY
 * @date 12.01.2021
 */

#include "include/puppeteer.h"
#include "include/pup_req.h"
#include "include/pup_res.h"
#include "include/pup_dispatcher.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <arpa/inet.h>

static u_int16_t get_port(char *portstr);

struct puppeteer *create_puppeteer(struct map *conf, int *status)
{
    struct puppeteer *puppeteer;

    *status = PUPPETEER_STATUS_OK;

    if (conf == NULL)
    {
        *status = PUPPETEER_ERROR_CONFIGURATION_MISSING;
        return NULL;
    }

    char *s_port = (char *)mapGet("PORT", conf);
    if (s_port == NULL)
    {
        *status = PUPPETEER_ERROR_CONF_PORT_NOT_DEFINED;
        return NULL;
    }

    puppeteer = (struct puppeteer *)malloc(sizeof(struct puppeteer));
    if (puppeteer == NULL)
    {
        *status = PUPPETEER_ERROR_MEMMORY_ALLOCATION;
        return NULL;
    }

    puppeteer->conf = conf;

    bzero((void *)&(puppeteer->addr), sizeof(struct sockaddr_in));
    puppeteer->addr.sin_port = htons(get_port(s_port));
    puppeteer->addr.sin_family = AF_INET;
    puppeteer->addr.sin_addr.s_addr = INADDR_ANY;

    puppeteer->ccf = NULL;
    puppeteer->sockfd = -1;

    return puppeteer;
}

int run_puppeteer(struct puppeteer *puppeteer)
{
    struct sockaddr_in client_addr;

    int sockfd;
    int req_sock;
    int socket_addr_size;

    if (puppeteer == NULL)
    {
        return PUPPETEER_ERROR_NULL_POINTER;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        return PUPPETEER_ERROR_SOCKET_CREATION;
    }

    if (bind(sockfd, (struct sockaddr *)&(puppeteer->addr), sizeof(struct sockaddr_in)) < 0)
    {
        perror("ERROR>> Binding error");
        return PUPPETEER_ERROR_SOCKET_BINDING;
    }

    if (listen(sockfd, PUPPETEER_MAX_CONNECTION))
    {
        perror("ERROR>> Could not listen port: ");
        return PUPPETEER_ERROR_SOCKET_LISTENING;
    }
    fprintf(stderr, "[INFO][PUPPETEER] Listening: %s:%u\n", inet_ntoa(puppeteer->addr.sin_addr), ntohs(puppeteer->addr.sin_port));

    socket_addr_size = sizeof(client_addr);
    while ((req_sock = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&socket_addr_size)))
    {
        if (req_sock < 0)
        {
            perror("ERROR>> can not accept");
            continue;
        }

        struct pup_req req;
        struct pup_res res;
        char buff[BUFF_LEN];
        memset((void *)buff, 0, BUFF_LEN);
        int msize;
        int dsr_r;
        int dispatch_st;
        int n;
        char *client_ip = inet_ntoa(client_addr.sin_addr);

        fprintf(stderr, "[INFO][PUPPETEER] Client: %s:%u connected.\n", client_ip, ntohs(client_addr.sin_port));
        if (fork() == 0)
        {
            while ((n = recv(req_sock, buff, BUFF_LEN, 0)) > 0)
            {
                fprintf(stderr, "[INFO][PUPPETEER] incoming request from %s:%u.\n", client_ip, ntohs(client_addr.sin_port));

                dsr_r = deserialize_pup_req(&req, buff, BUFF_LEN);
                if (dsr_r == PUP_REQUEST_OK)
                {
                    dispatch_st = dispatch_pup_req(puppeteer, &req, &res);
                    if (dispatch_st != 0)
                    {
                        res.body = NULL;
                        res.stcode = PUP_RESPONSE_CODE_ERROR;
                    }
                }
                else
                {
                    res.stcode = PUP_RESPONSE_CODE_MALFORMED_REQUEST;
                }

                bzero((void *)buff, BUFF_LEN);
                msize = serialize_pup_res(&res, buff, BUFF_LEN);
                // TODO: log if serialization fails

                n = write(req_sock, buff, msize);
                // TODO: log if write fails

                clear_pup_req(&req);
                clear_pup_res(&res);
            }

            if (n < 0)
            {
                fprintf(stderr, "[INFO][PUPPETEER] Client: %s:%u failed and disconnected.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                shutdown(req_sock, SHUT_RDWR);
                exit(EXIT_FAILURE);
            }
            else if (n == 0)
            {
                fprintf(stderr, "[INFO][PUPPETEER] Client: %s:%u disconnected.\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                shutdown(req_sock, SHUT_RDWR);
                exit(EXIT_FAILURE);
            }
        }

        close(req_sock);
    }

    return 0;
}

static u_int16_t get_port(char *portstr)
{
    int port = atoi(portstr);

    if (port >= USHRT_MAX)
    {
        return USHRT_MAX - 1;
    }

    return (u_int16_t)port;
}