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

#define BUFF_LEN 1024

static enum conf_type set_conf_type(enum conf_type type);
static u_int16_t get_port(char *portstr);
static int get_max_slave_amount(char *max_slavestr);
static struct l_node **set_slave_arr(int max_amount);
static void
create_req(struct lcp_req *req, u_char ver, const char *id,
           const char *msgid, u_int16_t opcode, const char *header, const char *body);
static int send_req(int sockfd, struct lcp_req *req);
static int extract_port(const char *message);

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
        lucretia->slaves = set_slave_arr(max_slave_amount);
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

    struct sockaddr_in master_addr;

    struct lcp_req req;
    struct lcp_req *resp;
    int resp_size;

    const char *hello = "CAN I BE YOUR SLAVE";
    const char *OK = "OK";

    if (server->type != SLAVE)
    {
        return LUCRETIA_ERROR_NON_MASTER_OPERATION;
    }

    // creating socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        return LUCRETIA_ERROR_SOCKET_CREATION;
    }

    master_addr.sin_family = AF_INET;
    master_addr.sin_addr.s_addr = inet_addr(address);
    master_addr.sin_port = htons(port);

    if (connect(sockfd, (const struct sockaddr *)&master_addr, sizeof(master_addr)) < 0)
    {
        close(sockfd);
        return LUCRETIA_ERROR_CONNECTION;
    }

    get_uuid(msgid, UUID_STR_LEN);

    // starting handshake
    create_req(&req, 1, NULL, msgid, L_OP_HANDSHAKE, NULL, hello);
    nsend = send_req(sockfd, &req);
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

    resp = deserialize_lcp_req(buffer, BUFF_LEN, resp_size);
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
    create_req(&req, 1, NULL, msgid, L_OP_HANDSHAKE, NULL, buffer);
    nsend = send_req(sockfd, &req);
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
    resp = deserialize_lcp_req(buffer, BUFF_LEN, resp_size);
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
    strncpy(server->id_by_master, resp->body, UUID_STR_LEN);

    // returning id to master to ack.
    create_req(&req, 1, NULL, msgid, L_OP_HANDSHAKE, NULL, server->id_by_master);
    nsend = send_req(sockfd, &req);
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
    resp = deserialize_lcp_req(buffer, BUFF_LEN, resp_size);
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

    shutdown(sockfd, SHUT_RDWR);

    return 0;
}

int handle_lcp_handshake(struct lucretia *server, int sockfd, struct sockaddr_in req_addr, struct lcp_req *original_req)
{
    int nsend;
    int nread;
    int resp_size;

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
        create_req(&req, 1, NULL, msgid, L_OP_HANDSHAKE, NULL, FAILED);
        send_req(sockfd, &req);
        shutdown(sockfd, SHUT_RDWR);
        return LUCRETIA_ERROR_MEM_ALLOC;
    }

    strncpy(msgid, original_req->msgid, strlen(original_req->msgid));

    // TODO: check slave array then send OK response
    create_req(&req, 1, NULL, msgid, L_OP_HANDSHAKE, NULL, OK);
    nsend = send_req(sockfd, &req);
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

    resp = deserialize_lcp_req(buffer, BUFF_LEN, resp_size);
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
    slave->addr.sin_port = (in_port_t)x_port;
    slave->addr.sin_family = AF_INET;
    slave->addr.sin_addr = req_addr.sin_addr;

    get_uuid(sid, UUID_STR_LEN);
    slave->id = (char *)malloc(UUID_STR_LEN * sizeof(char));
    strncpy(slave, sid, UUID_STR_LEN);

    create_req(&req, 1, sid, msgid, L_OP_HANDSHAKE, NULL, sid);
    nsend = send_req(sockfd, &req);
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
    resp = deserialize_lcp_req(buffer, BUFF_LEN, resp_size);
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

    create_req(&req, 1, sid, msgid, L_OP_HANDSHAKE, NULL, OK);
    nsend = send_req(sockfd, &req);
    if (nsend < 0)
    {
        shutdown(sockfd, SHUT_RDWR);
        return nsend;
    }

    server->slaves[0] = slave;
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
        perror("ERROR>> socket creation failed");
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        return LUCRETIA_ERROR_SOCKET_CREATION;
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
        memset((void*)buff, 0, BUFF_LEN);
        int msize;
        int mapping_result;

        if (fork() == 0)
        {
            n = recv(sock_req, buff, BUFF_LEN, 0);
            if (n > 0)
            {
                req = deserialize_lcp_req(buff, BUFF_LEN, &msize);
                //TODO: add filter for id verification
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
    while (p && i < 7 && p[i] != '\0' && p[i] != "\n")
    {
        buff[i] = p[i];
        i++;
    }
    buff[++i] = 0;

    return atoi(buff);
}

static void
create_req(struct lcp_req *req, u_char ver, const char *id,
           const char *msgid, u_int16_t opcode, const char *header, const char *body)
{
    req->ver = ver;
    req->id = id;
    req->msgid = msgid;
    req->opcode = opcode;
    req->header = header;
    req->body = body;
}

static int send_req(int sockfd, struct lcp_req *req)
{
    char message[BUFF_LEN];
    int nsend;

    int msglen = serialize_lcp_req(req, message, BUFF_LEN);
    if (msglen <= 0)
    {
        return LUCRETIA_ERROR_REQUEST_SERIALIZATION;
    }

    nsend = send(sockfd, (void *)message, msglen, 0);
    if (nsend < 0)
    {
        return LUCRETIA_ERROR_SEND_OPERATION;
    }

    return nsend;
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

static struct l_node **set_slave_arr(int max_amount)
{
    struct l_node **slaves = (struct l_node **)malloc(max_amount * sizeof(struct l_node *));
    if (slaves == NULL)
    {
        return NULL;
    }

    struct l_node **tmp = slaves;
    for (size_t i = 0; i < max_amount; i++)
    {
        *tmp++ = NULL;
    }

    return slaves;
}