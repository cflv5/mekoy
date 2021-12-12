/**
 * 10.12.2021
 * Author: Bedirhan AKÇAY
 */

#include "include/mekoy.h"
#include "include/util.h"
#include "include/lucretia.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

static void free_mekoy(struct mekoy *ccf);
static int create_m_process(char *name, struct mekoy *ccf);

struct mekoy *create_mekoy(struct map *configurations, int *rtrn_status)
{
    char *pr_path;

    int create_p_status;

    struct m_process *pr = NULL;

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

    return ccf;
}

static void free_mekoy(struct mekoy *ccf)
{
    mapClose(ccf->ps);
    free(ccf);
}

static int create_m_process(char *name, struct mekoy *ccf)
{
    struct m_process *pr = NULL;

    char *pr_path = (char *)mapGet(name, ccf->configurations);
    if (pr_path != NULL)
    {
        pr = (struct m_process *)malloc(sizeof(struct m_process));
        if (pr == NULL)
        {
            return MEKOY_ERROR_MEM_ALLOCATION;
        }

        pr->path = pr_path;
        pr->status = CREATED;
        pr->name = name;

        mapDynAdd(name, (void *)pr, ccf->ps);
    }

    return MEKOY_RETURN_STATUS_OK;
}
