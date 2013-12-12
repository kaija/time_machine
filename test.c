#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "time_machine.h"

#define TEST_COUNT 2000
unsigned long rudp_curr_time()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return 1000000 * tv.tv_sec + tv.tv_usec;
}

int timeout(void *data)
{
    struct user_info *user = (struct user_info *) data;
    if(user){
        //printf("delete user %s user->name %lu \n", user->name, user->timeout);
    }
    return 0;
}

int main()
{
    unsigned long a, b;
    tm_init();

    tm_reg_timeout_cb(timeout);
    int c;
    a = rudp_curr_time();
    srand(time(NULL));
    unsigned long start = rand();
    for(c = 0; c < TEST_COUNT ; c++){
        unsigned long r = rand() % 20;
        srand(start += r);
        tm_user_insert("kaija", 1, r);
    }
    b = rudp_curr_time();
    printf("test insert %d elements used %lu usec\n",TEST_COUNT, b-a);
    tm_loop_thread(400);
    while(1){
        for(c = 0; c < TEST_COUNT ; c++){
            unsigned long r = rand() % 20;
            srand(start += r);
            tm_user_insert("kaija", 1, r);
        }
        sleep(1);
    }
    //tm_dump();
	return 0;
}
