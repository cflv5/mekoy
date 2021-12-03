#ifndef __UTIL_C_
#define __UTIL_C_

#include "lucretia.h"

#define UTIL_ERROR_INSUFFICIENT_BUFFER_LEN -1

enum conf_type get_conf_type(char *property);
int get_uuid(char *buff, int len);

#endif // !__UTIL_C_