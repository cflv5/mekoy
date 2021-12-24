#include "include/process_message.test.h"

#include "../src/include/process_message.h"

#include <stdlib.h>
#include <string.h>

int serialize_process_message__given_process_message()
{
    struct process_message *msg = (struct process_message*)malloc(sizeof(struct process_message));
    char buff[BUFF_LEN];

    if(msg == NULL)
    {
        return 0;
    }

    msg->code = 'A';
    msg->data = "DATA";

    int len = serialize_process_message(msg, buff, BUFF_LEN);

    if (len < 0)
    {
        return 0;
    }
    
    return 1;
}

int deserialize_process_message__given_process_message()
{
    char *message = "ADATA\n";
    int std;
    deserialize_process_message(message, strlen(message), &std);

    if(std != 0) 
    {
        return 0;
    }

    return 1;
}