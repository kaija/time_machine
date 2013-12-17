#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "simclist.h"
#include "time_machine.h"

struct time_machine{
    list_t user_list;
    pthread_mutex_t list_mutex;
    int (*timeout_cb)(void *data);
    int run;
};
struct tm_thread_param{
    int interval;
};
static struct time_machine tmd;
static int list_comparator_time(const void *a, const void *b)
{
    struct user_info *pre = (struct user_info *)a;
    struct user_info *pos = (struct user_info *)b;
    //printf("compare %lu  %lu\n", pre->timeout, pos->timeout);
    if(pre->timeout < pos->timeout) return 0;
    return -1;
}
unsigned long tm_get_time()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return 1000000 * tv.tv_sec + tv.tv_usec;
}

static int tm_id_seeker(const void *data, const void *id)
{
    struct user_info *user = (struct user_info *) data;
    if(user->id == *((int *)id)){
        return 1;
    }
    return 0;
}

static int tm_name_seeker(const void *data, const void *name)
{
    struct user_info *user = (struct user_info *) data;
    if(user && strncmp(user->name, (char *)name, strlen((char *)name))==0){
        return 1;
    }
    return 0;
}

int tm_init()
{
    list_init(&tmd.user_list);
    list_attributes_comparator(&tmd.user_list, list_comparator_time);
    tmd.timeout_cb = NULL;
    if(pthread_mutex_init(&tmd.list_mutex, NULL) != 0){
        return -1;
    }
    return 0;
}

int tm_reg_timeout_cb(int (*callback)(void *))
{
    tmd.timeout_cb = callback;
    return 0;
}

int tm_user_insert(char *name, int id, int timeout)
{
    int ret = 0;
    pthread_mutex_lock(&tmd.list_mutex);
    struct user_info *user = malloc(sizeof(struct user_info));
    if(user){
        memset(user, 0, sizeof(struct user_info));
        strncpy(user->name, name , USER_NAME_LEN);
        user->id = id;
        user->start_time = tm_get_time();
        user->timeout = user->start_time + 1000000 * timeout;
        if(list_append(&tmd.user_list, user) == 1){
            //printf("Add user %s successful\n", name);
            list_attributes_comparator(&tmd.user_list, list_comparator_time);
            list_sort(&tmd.user_list, 1);
        }else{
            ret =  -TM_LINK_LIST_ERROR;
        }
    }else{
        ret = -TM_OUT_OF_MEMORY;
    }
    pthread_mutex_unlock(&tmd.list_mutex);
    return ret;
}

int tm_user_delete_by_id(int id)
{
    int ret = 0;
    pthread_mutex_lock(&tmd.list_mutex);
    list_attributes_seeker(&tmd.user_list, tm_id_seeker);
    struct user_info *user = (struct user_info *)list_seek(&tmd.user_list, &id);
    if(user != NULL){
        //TODO add delete user behavior
        printf("find user by id %s delete it\n", user->name);
        if(tmd.timeout_cb){
            tmd.timeout_cb(user);
            free(user);
        }
        list_attributes_comparator(&tmd.user_list, NULL);
        list_delete(&tmd.user_list, user);
    }else{
        ret = -1;
    }
    pthread_mutex_unlock(&tmd.list_mutex);
    return ret;
}

int tm_user_delete_by_name(char *name)
{
    int ret = 0;
    pthread_mutex_lock(&tmd.list_mutex);
    list_attributes_seeker(&tmd.user_list, tm_name_seeker);
    struct user_info *user = (struct user_info *)list_seek(&tmd.user_list, name);
    if(user != NULL){
        //TODO add delete user behavior
        printf("find user by id %s delete it\n", user->name);
        if(tmd.timeout_cb){
            tmd.timeout_cb(user);
            free(user);
        }
        list_attributes_comparator(&tmd.user_list, NULL);
        list_delete(&tmd.user_list, user);
    }else{
        ret = -1;
    }
    pthread_mutex_unlock(&tmd.list_mutex);
    return ret;
}

void tm_check_timeout()
{
    pthread_mutex_lock(&tmd.list_mutex);
    int count = list_size (&tmd.user_list);
    int index;
    unsigned long now = tm_get_time();
    for(index = 0; index < count ; index ++){
        struct user_info *user = list_get_at(&tmd.user_list, index);
        if(user){
            if(user->timeout > now){
                break;
            }else{
                if(tmd.timeout_cb){
                    tmd.timeout_cb(user);
                    free(user);
                }
            }
        }
    }
    if(index > 0){
        list_delete_range(&tmd.user_list, 0, index - 1);
        printf("delete prefix %d item total %d\n", index, count);
    }
    pthread_mutex_unlock(&tmd.list_mutex);
}
int tm_loop_exit()
{
    tmd.run = 0;
    return 0;
}
int tm_loop(int interval)
{
    struct timeval tv;
    tmd.run = 1;
    while(tmd.run){
        tv.tv_sec = interval / 1000;
        tv.tv_usec = (interval % 1000) * 1000;
        int rc = select(0, NULL, NULL, NULL, &tv);
        if(rc == 0){
            //printf("timeout\n");
            tm_check_timeout();
        }
    }
    return 0;
}

void *tm_loop_func(void *data)
{
    struct tm_thread_param *param = (struct tm_thread_param *)data;
    int interval = param->interval;
    printf("Time Machine thread check interval %d\n", interval);
    tm_loop(interval);
    pthread_exit("time_machine thread exit\n");
}

int tm_loop_thread(int interval)
{
    pthread_t thread_tm;
    struct tm_thread_param *param = malloc(sizeof(struct tm_thread_param));
    if(param){
        param->interval = interval;
    }
    pthread_create(&thread_tm, NULL, &tm_loop_func, param);
    return 0;
}

void tm_sort()
{
    list_attributes_comparator(&tmd.user_list, list_comparator_time);
    list_sort(&tmd.user_list, 1);
}

void tm_dump()
{
    int count = list_size (&tmd.user_list);
    int index;
    for(index = 0; index < count ; index ++){
        struct user_info *user = list_get_at(&tmd.user_list, index);
        if(user){
            printf("timeout = %lu\n", user->timeout);
        }
    }
}
