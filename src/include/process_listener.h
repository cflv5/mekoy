#ifndef __PROCESS_LISTENER_H__
#define __PROCESS_LISTENER_H__

#include "process_handler.h"
#include "m_process.h"

#include <unistd.h>

struct m_process;

struct process_listener
{
    struct m_process *ps;
    process_handler_f handler;
    pid_t pid;
};

struct process_listener *create_process_listener(struct m_process *ps);

#endif // !__PROCESS_LISTENER_H__