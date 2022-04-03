#ifndef __MEKOY_H__
#define __MEKOY_H__

struct lucretia;
struct puppeteer;

#include "map.h"
#include "lucretia.h"
#include "process_listener.h"
#include "m_process.h"
#include "conf_type.h"
#include "puppeteer.h"

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
#define MEKOY_ERROR_FETCH_PROCESS -8
#define MEKOY_ERROR_REMOTE_CONTROL_CONFIGURATION_NOT_DEFINED -9
#define MEKOY_ERROR_REMOTE_CONTROL_SERVER_CREATION -10
#define MEKOY_ERROR_REMOTE_CONTROL_CREATION -11
#define MEKOY_ERROR_REMOTE_CONTROL_NULL_POINTER -12
#define MEKOY_ERROR_REMOTE_CONTROL_PROCESS_NOT_CREATED -13


#define MEKOY_SIZE_PROCESS 4

struct mekoy 
{
    struct map* configurations;
    struct map* ps;
    struct lucretia *lucretia;
    struct puppeteer *puppeteer;

    enum conf_type type;
};

struct mekoy *create_mekoy(struct map* configurations, int *rtrn_status);
int m_run(struct mekoy *ccf);
int m_listen(struct mekoy *ccf);

#endif // !__MEKOY_H__