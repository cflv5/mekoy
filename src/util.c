#include "include/util.h"

#include <string.h>

enum conf_type get_conf_type(char *property)
{
    if(property == NULL)
    {
        return UNKNOWN;
    }
    else if(strcasecmp(property, "MASTER") == 0)
    {
        return MASTER;
    }
    else if(strcasecmp(property, "SLAVE") == 0)
    {
        return SLAVE;
    }
    else
    {
        return UNKNOWN;
    }
}
