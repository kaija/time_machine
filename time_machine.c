#include <stdio.h>
#include <stdlib.h>
#include "simclist.h"
static list_t user_list;
int tm_init()
{
    list_init(&user_list);
    return 0;
}
