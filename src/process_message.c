/**
 * @author Bedirhan AKÇAY - bedirhanakcay@icloud.com
 * @date 16.12.2021
 */

#include "include/process_message.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int serialize_process_message(struct process_message *message, char *buff, int len)
{
    int ind;

    if (message == NULL || buff == NULL)
    {
        return PROCESS_MESSAGE_ERROR_NULL_POINTER;
    }

    memset(buff, 0, len);

    buff[0] = message->code;

    if(message->data == NULL)
    {
        message->data = "";
    }

    strncpy(&buff[1], message->data, len - 2);

    ind = strlen(buff);

    buff[ind] = '\n';

    return ind;
}

struct process_message *deserialize_process_message(char *message, int len, int *status)
{
    struct process_message *msg = NULL;
    int mlen;

    *status = PROCESS_MESSAGE_OK;

    if (message == NULL)
    {
        *status = PROCESS_MESSAGE_ERROR_NULL_POINTER;
        return NULL;
    }

    msg = (struct process_message *)malloc(sizeof(struct process_message));
    if (msg == NULL)
    {
        *status = PROCESS_MESSAGE_ERROR_MEMORY_ALLOCATION;
        return NULL;
    }

    mlen = strlen(message);
    len = mlen > len ? len : mlen;

    if(len == 0)
    {
        *status = PROCESS_MESSAGE_ERROR_EMPTY_MESSAGE;
        return NULL;
    }
    
    msg->code = *message++;

    msg->data = (char *)malloc(len * sizeof(char));
    memset((void *)msg->data, 0, len);
    strncpy(msg->data, message, len - 1);

    if (len > 1 && msg->data[len - 2] == '\n')
    {
        msg->data[len - 2] = '\0';
    }

    return msg;
}

int send_process_message(struct process_message *message, struct m_process *ps)
{
    char buff[PROCESS_HANDLER_BUFF_SIZE];

    if(ps == NULL)
    {
        return PROCESS_MESSAGE_ERROR_NULL_POINTER;
    }
    int pwrite = ps->pfd[1];
    
    int size = serialize_process_message(message, buff, PROCESS_HANDLER_BUFF_SIZE);

    if (size < 0)
    {
        return PROCESS_MESSAGE_ERROR_SERILIAZATION;
    }
    
    fprintf(stderr, "[INFO][MEKOY][PMESSAGE] - Send message (%s) to: %s", buff, ps->name);
    ssize_t wsize = write(pwrite, buff, size);
    if (wsize < 0)
    {
        return PROCESS_MESSAGE_ERROR_WRITE_TO_PIPE;
    }
    
    return PROCESS_MESSAGE_OK;
}

int free_process_message(struct process_message **msg)
{
    free((*msg)->data);
    free(*msg);

    *msg = NULL;

    return 0;
}