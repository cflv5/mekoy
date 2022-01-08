/*
 * Created at: 29.11.2021
 * Created by Bedirhan AKÇAY

 * Implementation of Lucretia master-slave server
*/

#include "include/lucretia.h"
#include "include/util.h"
#include "include/lcp.h"

#include "include/dispatch.h"

#include <uuid/uuid.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#define BUFF_LEN 1024

static enum conf_type set_conf_type(enum conf_type type);
static u_int16_t get_port(char *portstr);
static int get_max_slave_amount(char *max_slavestr);
static struct l_node_list **set_slave_list(int max_amount);
static int extract_port(const char *message);
static int check_if_slave_array_avaliable(struct l_node_list *slaves);

struct lucretia *new_lucretia(struct map *props)
{
    char *typestr = NULL;
    char *portstr = NULL;
    char *max_slave_amountstr = NULL;
    char *max_concurrent_connstr = NULL;

    int max_slave_amount;
    int max_connection;

    if (props == NULL)
    {
        return LUCRETIA_ERROR_PROPS_NULL;
    }

    struct lucretia *lucretia = (struct lucretia *)malloc(sizeof(struct lucretia));
    if (lucretia == NULL)
    {
        return LUCRETIA_ERROR_MEM_ALLOC;
    }

    lucretia->properties = props;
    // getting properties
    typestr = (char *)mapGet("CONF_TYPE", props);
    portstr = (char *)mapGet("PORT", props);
    max_slave_amountstr = (char *)mapGet("MAX_SLAVE", props);
    max_concurrent_connstr = (char *)mapGet("MAX_CONNECTION", props);

    // setting server internet address
    bzero((char *)&lucretia->addr, sizeof(lucretia->addr));
    lucretia->addr.sin_port = htons(get_port(portstr));
    lucretia->addr.sin_family = AF_INET;
    lucretia->addr.sin_addr.s_addr = INADDR_ANY;

    // Setting configuration type
    lucretia->type = set_conf_type(get_conf_type(typestr));

    max_slave_amount = lucretia->max_slave = get_max_slave_amount(max_slave_amountstr);
    max_connection = lucretia->max_connection = get_max_slave_amount(max_concurrent_connstr);

    if (lucretia->type == MASTER)
    {
        lucretia->master = NULL;
        lucretia->slaves = set_slave_list(max_slave_amount);
        if (lucretia->slaves == NULL)
        {
            return LUCRETIA_ERROR_MEM_ALLOC;
        }

        lucretia->status = READY;

        lucretia->id_by_master = 0;
    }
    else
    {
        lucretia->slaves = NULL;
        lucretia->master = NULL;

        lucretia->status = NOT_CONNEDTED;
    }

    return lucretia;
}

int lcp_handshake(struct lucretia *server, const char *address, in_port_t port)
{
    int sockfd;
    int nsend;
    int nread;
    int id_len;

    char msgid[UUID_STR_LEN];
    char buffer[BUFF_LEN];
    int buff_len;

    struct lcp_req req;
    struct lcp_req *resp;
    int resp_size = 0;

    const char *hello = "CAN I BE YOUR SLAVE";
    const char *OK = "OK";

    if (server->type != SLAVE)
    {
        return LUCRETIA_ERROR_NON_MASTER_OPERATION;
    }

    if (server->master != NULL || server->status == CONNECTED)
    {
        return LUCRETIA_ERROR_ALREADY_CONNECTED;
    }

    struct l_node *master = (struct l_node *)malloc(sizeof(struct l_node));
    if (master == NULL)
    {
        return LUCRETIA_ERROR_MEM_ALLOC;
    }

    // creating socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        return LUCRETIA_ERROR_SOCKET_CREATION;
    }

    bzero((char *)&master->addr, sizeof(master->addr));
    master->addr.sin_family = AF_INET;
    master->addr.sin_addr.s_addr = inet_addr(address);
    master->addr.sin_port = htons(port);

    if (connect(sockfd, (const struct sockaddr *)&master->addr, sizeof(master->addr)) < 0)
    {
        close(sockfd);
        return LUCRETIA_ERROR_CONNECTION;
    }

    get_uuid(msgid, UUID_STR_LEN);

    // starting handshake
    populate_lcp_request(&req, 1, NULL, msgid, L_OP_HANDSHAKE, NULL, hello);
    nsend = send_lcp_request(sockfd, &req);
    if (nsend < 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return nsend;
    }

    // master's response for handshake request
    nread = recv(sockfd, (void *)buffer, BUFF_LEN, 0);
    if (nread < 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_CONNECTION_SHUTDOWN;
    }

    resp = deserialize_lcp_req(buffer, BUFF_LEN, &resp_size);
    if (resp->opcode != L_OP_HANDSHAKE || strncmp(msgid, resp->msgid, strlen(msgid)) != 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_HANDSHAKE_MISMACHED_RESPONSE_VALUES;
    }

    if (strncmp(OK, resp->body, strlen(OK)) != 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_HANDSHAKE_MASTER_REJECTS;
    }

    // master allowed, sending port
    buff_len = snprintf(buffer, BUFF_LEN, "PORT=%hu", server->addr.sin_port);
    populate_lcp_request(&req, 1, NULL, msgid, L_OP_HANDSHAKE, NULL, buffer);
    nsend = send_lcp_request(sockfd, &req);
    if (nsend < 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return nsend;
    }

    // waiting master's response with id
    nread = recv(sockfd, (void *)buffer, BUFF_LEN, 0);
    if (nread < 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_CONNECTION_SHUTDOWN;
    }

    destroy_lcp_req(resp);
    resp = deserialize_lcp_req(buffer, BUFF_LEN, &resp_size);
    if (resp->opcode != L_OP_HANDSHAKE || strncmp(msgid, resp->msgid, strlen(msgid)) != 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_HANDSHAKE_MISMACHED_RESPONSE_VALUES;
    }

    id_len = strlen(resp->body);
    if (id_len == 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_HANDSHAKE_MISMACHED_RESPONSE_VALUES;
    }

    // setting id appointed by master
    server->id_by_master = (char *)malloc(UUID_STR_LEN * sizeof(char));
    memset((void *)server->id_by_master, 0, UUID_STR_LEN);
    strncpy(server->id_by_master, resp->body, UUID_STR_LEN - 1);

    // returning id to master to ack.
    populate_lcp_request(&req, 1, NULL, msgid, L_OP_HANDSHAKE, NULL, server->id_by_master);
    nsend = send_lcp_request(sockfd, &req);
    if (nsend < 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return nsend;
    }

    // waiting master's ack. message
    nread = recv(sockfd, (void *)buffer, BUFF_LEN, 0);
    if (nread < 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_CONNECTION_SHUTDOWN;
    }

    destroy_lcp_req(resp);
    resp = deserialize_lcp_req(buffer, BUFF_LEN, &resp_size);
    if (resp->opcode != L_OP_HANDSHAKE || strncmp(msgid, resp->msgid, strlen(msgid)) != 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        free(server->id_by_master);
        return LUCRETIA_ERROR_HANDSHAKE_MISMACHED_RESPONSE_VALUES;
    }

    if (strncmp(OK, resp->body, strlen(OK)) != 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        free(server->id_by_master);
        return LUCRETIA_ERROR_HANDSHAKE_MASTER_REJECTS;
    }

    server->status = CONNECTED;
    server->master = master;

    shutdown(sockfd, SHUT_RDWR);

    return 0;
}

int handle_lcp_handshake(struct lucretia *server, int sockfd, struct sockaddr_in req_addr, struct lcp_req *original_req)
{
    int nsend;
    int nread;
    int resp_size = 0;

    int x_port;
    in_port_t slave_port;

    char buffer[BUFF_LEN];
    char msgid[UUID_STR_LEN];

    char sid[UUID_STR_LEN];

    struct lcp_req req;
    struct lcp_req *resp;
    const char *OK = "OK";
    const char *FAILED = "FAILED";

    if (server->type != MASTER)
    {
        return LUCRETIA_ERROR_NON_SLAVE_OPERATION;
    }

    struct l_node *slave = (struct l_node *)malloc(sizeof(struct l_node));
    if (slave == NULL)
    {
        populate_lcp_request(&req, 1, NULL, msgid, L_OP_HANDSHAKE, NULL, FAILED);
        send_lcp_request(sockfd, &req);
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_MEM_ALLOC;
    }

    memset((void *)msgid, 0, UUID_STR_LEN);
    strncpy(msgid, original_req->msgid, UUID_STR_LEN - 1);

    // checking slave array status
    if (check_if_slave_array_avaliable(server->slaves))
    {
        populate_lcp_request(&req, 1, NULL, msgid, L_OP_HANDSHAKE, NULL, FAILED);
        nsend = send_lcp_request(sockfd, &req);
        if (nsend < 0)
        {
            shutdown(sockfd, SHUT_RDWR);
            return nsend;
        }
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_SETTING_SLAVE;
    }

    populate_lcp_request(&req, 1, NULL, msgid, L_OP_HANDSHAKE, NULL, OK);
    nsend = send_lcp_request(sockfd, &req);
    if (nsend < 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return nsend;
    }

    // waiting slaves server port response
    nread = recv(sockfd, (void *)buffer, BUFF_LEN, 0);
    if (nread < 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_CONNECTION_SHUTDOWN;
    }

    resp = deserialize_lcp_req(buffer, BUFF_LEN, &resp_size);
    if (resp->opcode != L_OP_HANDSHAKE || strncmp(msgid, resp->msgid, strlen(msgid)) != 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_HANDSHAKE_MISMACHED_RESPONSE_VALUES;
    }

    x_port = extract_port(resp->body);
    if (x_port < 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_HANDSHAKE_MISMACHED_RESPONSE_VALUES;
    }

    bzero((char *)&slave->addr, sizeof(slave->addr));
    slave->addr.sin_port = (in_port_t)ntohs(x_port);
    slave->addr.sin_family = AF_INET;
    slave->addr.sin_addr = req_addr.sin_addr;

    get_uuid(sid, UUID_STR_LEN);
    slave->id = (char *)malloc(UUID_STR_LEN * sizeof(char));
    memset((void *)slave->id, 0, UUID_STR_LEN);
    strncpy(slave->id, sid, UUID_STR_LEN);

    populate_lcp_request(&req, 1, sid, msgid, L_OP_HANDSHAKE, NULL, sid);
    nsend = send_lcp_request(sockfd, &req);
    if (nsend < 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return nsend;
    }

    // waiting for slave's ack.
    nread = recv(sockfd, (void *)buffer, BUFF_LEN, 0);
    if (nread < 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_CONNECTION_SHUTDOWN;
    }

    destroy_lcp_req(resp);
    resp = deserialize_lcp_req(buffer, BUFF_LEN, &resp_size);
    if (resp->opcode != L_OP_HANDSHAKE || strncmp(msgid, resp->msgid, strlen(msgid)) != 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_HANDSHAKE_MISMACHED_RESPONSE_VALUES;
    }

    if (strncmp(sid, resp->body, strlen(sid)) != 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        free(server->id_by_master);
        return LUCRETIA_ERROR_HANDSHAKE_MISMACHED_RESPONSE_VALUES;
    }

    if (insert_l_node(server->slaves, slave))
    {
        populate_lcp_request(&req, 1, NULL, msgid, L_OP_HANDSHAKE, NULL, FAILED);
        nsend = send_lcp_request(sockfd, &req);
        if (nsend < 0)
        {
            shutdown(sockfd, SHUT_RDWR);
            return nsend;
        }
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_SETTING_SLAVE;
    }

    populate_lcp_request(&req, 1, sid, msgid, L_OP_HANDSHAKE, NULL, OK);
    nsend = send_lcp_request(sockfd, &req);
    if (nsend < 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return nsend;
    }

    shutdown(sockfd, SHUT_RDWR);

    return 0;
}

int l_run(struct lucretia *server)
{
    struct sockaddr_in *serv_addr = &server->addr;
    struct sockaddr_in client_addr;

    int client_addr_size;
    int sockfd;
    int sock_req;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        return LUCRETIA_ERROR_SOCKET_CREATION;
    }

    if (bind(sockfd, (struct sockaddr *)serv_addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("ERROR>> Binding error");
        return LUCRETIA_ERROR_SOCKET_BINDING;
    }

    if (listen(sockfd, server->max_connection))
    {
        perror("ERROR>> Could not listen port: ");
        exit(EXIT_FAILURE);
    }

    client_addr_size = sizeof(client_addr);
    while ((sock_req = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size)))
    {
        if (sock_req < 0)
        {
            perror("ERROR>> can not accept");
            continue;
        }

        struct lcp_req *req = NULL;
        int n;
        char buff[BUFF_LEN];
        memset((void *)buff, 0, BUFF_LEN);
        int msize;
        int mapping_result;

        if (fork() == 0)
        {
            n = recv(sock_req, buff, BUFF_LEN, 0);
            if (n > 0)
            {
                req = deserialize_lcp_req(buff, BUFF_LEN, &msize);
                // TODO: add filter for id verification
                dispatch(server, req, sock_req, client_addr);
                exit(EXIT_SUCCESS);
            }

            if (n < 0)
            {
                shutdown(sock_req, SHUT_RDWR);
                exit(EXIT_FAILURE);
            }
            else if (n == 0)
            {
                shutdown(sock_req, SHUT_RDWR);
                exit(EXIT_FAILURE);
            }
        }

        close(sock_req);
    }

    return 0;
}

int destroy_lucretia(struct lucretia *server)
{
    // TODO: implement destruction of the server
    return 0;
}

static int extract_port(const char *message)
{
    const char *PORT_STR = "PORT=";
    char *p = strstr(message, PORT_STR);
    char buff[7];
    int i = 0;

    if (p == NULL)
    {
        return LUCRETIA_ERROR_HANDSHAKE_MISMACHED_RESPONSE_VALUES;
    }

    p += 5;
    while (p && i < 7 && p[i] != '\0' && p[i] != '\n')
    {
        buff[i] = p[i];
        i++;
    }
    buff[++i] = 0;

    return atoi(buff);
}

static enum conf_type set_conf_type(enum conf_type type)
{
    if (type == UNKNOWN)
    {
        return SLAVE;
    }
    else
    {
        return type;
    }
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

static int get_max_slave_amount(char *max_slavestr)
{
    if (max_slavestr == NULL)
    {
        return 5;
    }
    int amount = atoi(max_slavestr);

    if (amount <= 0)
    {
        amount = 5;
    }

    return amount;
}

static struct l_node_list **set_slave_list(int max_amount)
{
    if (max_amount <= 0)
    {
        max_amount = 5;
    }

    struct l_node_list *slaves = (struct l_node_list *)malloc(sizeof(struct l_node_list));
    if (slaves == NULL)
    {
        return NULL;
    }

    slaves->len = max_amount;
    slaves->size = 0;
    
    slaves->list = (struct l_node **)malloc(max_amount * sizeof(struct l_node *));
    if (slaves->list == NULL)
    {
        return NULL;
    }

    struct l_node **tmp = slaves->list;
    for (size_t i = 0; i < max_amount; i++)
    {
        *tmp++ = NULL;
    }

    return slaves;
}

static int check_if_slave_array_avaliable(struct l_node_list *slaves)
{
    int result = 0;
    if ((slaves->size + 1) == slaves->len)
    {
        result = 1;
    }
    return result;
}