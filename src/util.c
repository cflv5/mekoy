#include "include/util.h"

#include <string.h>
#include <uuid/uuid.h>

enum conf_type get_conf_type(char *property)
{
    if (property == NULL)
    {
        return UNKNOWN;
    }
    else if (strcasecmp(property, "MASTER") == 0)
    {
        return MASTER;
    }
    else if (strcasecmp(property, "SLAVE") == 0)
    {
        return SLAVE;
    }
    else
    {
        return UNKNOWN;
    }
}

int get_uuid(char *buff, int len)
{
    uuid_t binuuid;
    uuid_generate_random(binuuid);

    if(len < UUID_STR_LEN){
        return UTIL_ERROR_INSUFFICIENT_BUFFER_LEN;
    }

    uuid_unparse_lower(binuuid, buff);

    return 0;
}