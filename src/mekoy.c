/**
 * 10.12.2021
 * Author: Bedirhan AKÇAY
 */

#include "include/mekoy.h"
#include "include/util.h"
#include "include/lucretia.h"
#include "include/process_message.h"
#include "include/process_handler.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/prctl.h>

static void free_mekoy(struct mekoy *ccf);
static int create_m_process(char *name, struct mekoy *ccf);
static int start_m_process(const char *name, struct mekoy *ccf);
static void close_unrelated_fds(struct map *pss, struct m_process *ps);

struct mekoy *create_mekoy(struct map *configurations, int *rtrn_status)
{
    int create_p_status;

    *rtrn_status = MEKOY_RETURN_STATUS_OK;

    struct mekoy *ccf = (struct mekoy *)malloc(sizeof(struct mekoy));
    if (ccf == NULL)
    {
        *rtrn_status = MEKOY_ERROR_MEM_ALLOCATION;
        return NULL;
    }

    ccf->ps = mapNew();
    ccf->configurations = configurations;
    ccf->type = get_conf_type((char *)mapGet("MEKOY.TYPE", ccf->configurations));

    if (ccf->type == UNKNOWN)
    {
        ccf->type = SLAVE;
    }

    // TODO: process definitions should be from configuration file
    // CV
    if ((create_p_status = create_m_process(MEKOY_PROCESS_CV, ccf)) != 0)
    {
        *rtrn_status = create_p_status;
        free_mekoy(ccf);
        return NULL;
    }

    if (ccf->type == MASTER)
    {
        // AID CAR
        if ((create_p_status = create_m_process(MEKOY_PROCESS_AID_CAR_CONTROL, ccf)) != 0)
        {
            *rtrn_status = create_p_status;
            free_mekoy(ccf);
            return NULL;
        }

        // ROAD INFORM CONTROL
        if ((create_p_status = create_m_process(MEKOY_PROCESS_ROAD_INFORM_CONTROL, ccf)) != 0)
        {
            *rtrn_status = create_p_status;
            free_mekoy(ccf);
            return NULL;
        }

        // LIGHTING CONTROL
        if ((create_p_status = create_m_process(MEKOY_PROCESS_LIGHTING_CONTROL, ccf)) != 0)
        {
            *rtrn_status = create_p_status;
            free_mekoy(ccf);
            return NULL;
        }
    }

    return ccf;
}

int m_run(struct mekoy *ccf)
{
    pid_t pid;

    int mp_status;
    int listen_status;

    struct m_process *pr = NULL;

    if (ccf == NULL)
    {
        return MEKOY_ERROR_NULL_POINTER;
    }

    struct map *lucretia_conf = (struct map *)mapGet("MEKOY.LUCRETIA", ccf->configurations);
    if (lucretia_conf == NULL)
    {
        return MEKOY_ERROR_PROCESSES_LUCRETIA_CONFIGURATION_NOT_DEFINED;
    }

    struct lucretia *lucretia_server = new_lucretia(lucretia_conf);
    if (lucretia_server < 0)
    {
        return MEKOY_ERROR_MEM_ALLOCATION;
    }

    if (create_m_process(MEKOY_PROCESS_LUCRETIA_SERVER, ccf) != 0)
    {
        return MEKOY_ERROR_PROCESS_CREATION;
    }

    pr = (struct m_process *)mapGet(MEKOY_PROCESS_LUCRETIA_SERVER, ccf->ps);
    if (pr == NULL)
    {
        fprintf(stderr, "[ERROR][MEKOY] Could not get process\n");
        return MEKOY_ERROR_FETCH_PROCESS;
    }

    lucretia_server->ccf = ccf;

    pid = fork();
    if (pid == 0)
    {
        prctl(PR_SET_PDEATHSIG, SIGHUP);
        //TODO close unrelated pfds for lucretia server

        dup2(pr->cfd[0], STDIN_FILENO);
        dup2(pr->cfd[1], STDOUT_FILENO);
        close(pr->pfd[1]);
        close(pr->pfd[0]);

        mp_status = l_run(lucretia_server);

        fprintf(stdout, "F\n");
        fflush(stdout);
        exit(mp_status);
    }
    else if (pid < 0)
    {
        return MEKOY_ERROR_FORK;
    }


    pr->pid = pid;
    pr->status = STARTED;

    ccf->lucretia = lucretia_server;

    if ((mp_status = start_m_process(MEKOY_PROCESS_CV, ccf)) != 0)
        return mp_status;

    // TODO: make it able to scan indicies
    if (ccf->type == MASTER)
    {
        if ((mp_status = start_m_process(MEKOY_PROCESS_AID_CAR_CONTROL, ccf)) != 0)
            return mp_status;
        if ((mp_status = start_m_process(MEKOY_PROCESS_LIGHTING_CONTROL, ccf)) != 0)
            return mp_status;
        if ((mp_status = start_m_process(MEKOY_PROCESS_ROAD_INFORM_CONTROL, ccf)) != 0)
            return mp_status;
    }
    listen_status = m_listen(ccf);

    return listen_status != 0 ? listen_status : MEKOY_RETURN_STATUS_OK;
}

int m_listen(struct mekoy *ccf)
{
    struct map *pss = ccf->ps;
    struct mapIterator *iterator = getMapIterator(pss);
    struct process_listener *listener = NULL;
    struct m_process *ps = NULL;

    pid_t pid;

    while (mapIteratorNext(iterator) == 0)
    {
        ps = (struct m_process *)getIteratorVal(iterator);
        listener = create_process_listener(ps);

        pid = fork();
        if (pid == 0)
        {
            prctl(PR_SET_PDEATHSIG, SIGHUP);
            listen_process(listener, ccf);
            exit(EXIT_SUCCESS);
        }
        listener->pid = pid;
        ps->ps_listener = listener;
    }

    return 0;
}

static void free_mekoy(struct mekoy *ccf)
{
    mapClose(ccf->ps);
    free(ccf);
}

static int create_m_process(char *name, struct mekoy *ccf)
{
    struct m_process *pr = NULL;
    int ptcpfd[2];
    int ctppfd[2];

    pipe(ptcpfd);
    pipe(ctppfd);

    char *pr_path = (char *)mapGet(name, ccf->configurations);
    if (pr_path != NULL)
    {
        pr = (struct m_process *)malloc(sizeof(struct m_process));
        if (pr == NULL)
        {
            return MEKOY_ERROR_MEM_ALLOCATION;
        }

        pr->pfd[0] = ctppfd[0];
        pr->pfd[1] = ptcpfd[1];
        pr->cfd[0] = ptcpfd[0];
        pr->cfd[1] = ctppfd[1];

        pr->path = pr_path;
        pr->status = CREATED;
        pr->name = name;

        if (pthread_mutex_init(&(pr->lock), NULL) != 0)
        {
            free(pr);
            return MEKOY_ERROR_MUTEX_INIT;
        }

        mapDynAdd(name, (void *)pr, ccf->ps);
        return MEKOY_RETURN_STATUS_OK;
    }
    else
    {
        return MEKOY_ERROR_NULL_POINTER;
    }
}

static int start_m_process(const char *name, struct mekoy *ccf)
{
    pid_t pid;
    int rtn;

    struct m_process *pr = mapGet((char *)name, ccf->ps);
    if (pr != NULL)
    {
        pid = fork();
        if (pid == 0)
        {
            prctl(PR_SET_PDEATHSIG, SIGHUP);
            //close_unrelated_fds(ccf->ps, pr);

            dup2(pr->cfd[0], STDIN_FILENO);
            dup2(pr->cfd[1], STDOUT_FILENO);
            close(pr->pfd[1]);
            close(pr->pfd[0]);

            pr->status = STARTED;

            rtn = execl(pr->path, pr->path, (const char *)NULL);

            // TODO: log errno
            exit(rtn);
        }
        else if (pid < 0)
        {
            return MEKOY_ERROR_FORK;
        }
        pr->pid = pid;
        pr->status = STARTED;
        close(pr->cfd[1]);
        close(pr->cfd[0]);
    }

    return MEKOY_RETURN_STATUS_OK;
}

static void close_unrelated_fds(struct map *pss, struct m_process *ps)
{
    struct m_process *tmp;
    struct mapIterator *iterator;
    if (pss && ps)
    {
        iterator = getMapIterator(pss);
        while (mapIteratorNext(iterator) == 0)
        {
            tmp = (struct m_process *)getIteratorVal(iterator);
            if (tmp->pid != ps->pid)
            {
                close(tmp->pfd[M_PROCESS_READ_END]);
                close(tmp->pfd[M_PROCESS_WRITE_END]);
            }
        }
    }
}