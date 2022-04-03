#ifndef __PROCESS_MESSAGE_H__
#define __PROCESS_MESSAGE_H__

#define PROCESS_MESSAGE_OK 0

#define PROCESS_MESSAGE_ERROR_NULL_POINTER -1
#define PROCESS_MESSAGE_ERROR_MEMORY_ALLOCATION -2
#define PROCESS_MESSAGE_ERROR_SERILIAZATION -3
#define PROCESS_MESSAGE_ERROR_WRITE_TO_PIPE -4
#define PROCESS_MESSAGE_ERROR_EMPTY_MESSAGE -5
#define PROCESS_MESSAGE_ERROR_SEND_TO_NOT_STARTED -6
#define PROCESS_MESSAGE_ERROR_MUTEX_LOCK -7
#define PROCESS_MESSAGE_ERROR_MUTEX_UNLOCK -8


#include "process_message.h"
#include "m_process.h"

struct process_message
{
    char code;
    char *data;
};

int serialize_process_message(struct process_message *message, char *buff, int len);
struct process_message *deserialize_process_message(char *message, int len, int *status);

int send_process_message(struct process_message *message, struct m_process *ps);
int send_process_message_as_child(struct process_message *message, struct m_process *ps);

int free_process_message(struct process_message **msg);

#endif // !__PROCESS_MESSAGE_H__

