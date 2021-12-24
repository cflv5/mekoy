#ifndef __MEKOY_H__
#define __MEKOY_H__

#include "map.h"
#include "lucretia.h"
#include "process_listener.h"
#include "m_process.h"

#include <pthread.h>

#define M_MAX_PIDS 10

#define MEKOY_RETURN_STATUS_OK 0

#define MEKOY_WARNING_NO_PROCESS_DEFINED 1

#define MEKOY_ERROR_MEM_ALLOCATION -1
#define MEKOY_ERROR_PROCESSES_SIZE_NOT_DEFINED -2
#define MEKOY_ERROR_NULL_POINTER -3
#define MEKOY_ERROR_PROCESSES_LUCRETIA_CONFIGURATION_NOT_DEFINED -4
#define MEKOY_ERROR_FORK -5
#define MEKOY_ERROR_PROCESS_CREATION -6
#define MEKOY_ERROR_MUTEX_INIT -7


#define MEKOY_SIZE_PROCESS 4

struct mekoy 
{
    struct map* configurations;
    struct map* ps;

    enum conf_type type;
};

struct mekoy *create_mekoy(struct map* configurations, int *rtrn_status);

#endif // !__MEKOY_H__