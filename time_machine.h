#ifndef __TIME_MACHINE_H
#define __TIME_MACHINE_H

#define USER_NAME_LEN 32
enum{
    TM_SUCCESS,
    TM_OUT_OF_MEMORY,
    TM_LINK_LIST_ERROR,
    TM_USER_EXIST,
    TM_ID_REPEAT
};

struct user_info{
    int             id;
    char            name[USER_NAME_LEN];
    unsigned long   start_time;
    unsigned long   timeout;
};

int tm_init();
int tm_reg_timeout_cb(int (*callback)(void *));
int tm_user_insert(char *name, int id, int timeout);
int tm_loop_exit();
int tm_loop(int interval);
int tm_loop_thread(int interval);
void tm_dump();
#endif
