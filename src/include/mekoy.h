#ifndef __MEKOY_H__
#define __MEKOY_H__

#include "include/map.h"

#include <sys/types.h>

#define M_MAX_PIDS 10

#define MEKOY_RETURN_STATUS_OK 0

#define MEKOY_WARNING_NO_PROCESS_DEFINED 1

#define MEKOY_ERROR_MEM_ALLOCATION -1
#define MEKOY_ERROR_PROCESSES_SIZE_NOT_DEFINED -2
#define MEKOY_ERROR_NULL_POINTER -3
#define MEKOY_ERROR_PROCESSES_LUCRETIA_CONFIGURATION_NOT_DEFINED -4
#define MEKOY_ERROR_FORK -5
#define MEKOY_ERROR_PROCESS_CREATION -6

#define MEKOY_PROCESS_CV "CV"
#define MEKOY_PROCESS_LUCRETIA_SERVER "LUCRETIA"
#define MEKOY_PROCESS_LIGHTING_CONTROL "LIGHTING_CONTROL"
#define MEKOY_PROCESS_AID_CAR_CONTROL "AID_CAR_CONTROL"
#define MEKOY_PROCESS_ROAD_INFORM_CONTROL "ROAD_INFORM_CONTROL"

#define MEKOY_SIZE_PROCESS 4

enum m_p_status
{
    CREATED, STARTED, STOPED
};

struct m_process 
{
    pid_t pid;
    int pfd[2];
    enum m_p_status status;
    char *path;
    char *name;
};

enum m_type
{
    MASTER, SLAVE, UNKNOWN
};

struct mekoy 
{
    struct map* configurations;
    struct map* ps;

    enum m_type type;
};

struct mekoy *create_mekoy(struct map* configurations, int *rtrn_status);

#endif // !__MEKOY_H__