#ifndef __M_PROCESS__H__
#define __M_PROCESS__H__

#include "process_listener.h"

#include <pthread.h>
#include <sys/types.h>

#define MEKOY_PROCESS_CV "CV"
#define MEKOY_PROCESS_LUCRETIA_SERVER "LUCRETIA"
#define MEKOY_PROCESS_LIGHTING_CONTROL "LIGHTING_CONTROL"
#define MEKOY_PROCESS_AID_CAR_CONTROL "AID_CAR_CONTROL"
#define MEKOY_PROCESS_ROAD_INFORM_CONTROL "ROAD_INFORM_CONTROL"

#define M_PROCESS_READ_END 0
#define M_PROCESS_WRITE_END 1

enum m_p_status
{
    CREATED, STARTED, STOPED
};

struct m_process 
{
    pid_t pid;
    int pfd[2];
    int cfd[2];
    enum m_p_status status;
    char *path;
    char *name;
    struct process_listener *ps_listener;
    pthread_mutex_t lock;
};

#endif // !__M_PROCESS__H__