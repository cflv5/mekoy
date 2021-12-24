/**
 * @author: Bedirhan AKÇAY - bedirhanakcay@icloud.con
 * @date: 18.12.2021
*/

#include "include/process_listener.h"

#include <stdlib.h>

struct process_listener *create_process_listener(struct m_process *ps)
{
    struct process_listener *listener = (struct process_listener *)malloc(sizeof(struct process_listener));

    if(listener == NULL)
    {
        return NULL;
    }

    listener->ps = ps;
    listener->handler = get_process_handler(ps->name);
    listener->pid = 0;
    return listener;
}