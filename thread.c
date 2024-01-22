#include "thread.h"
#include <stdlib.h>

struct thread_t
{
    int total_count; //total num of requests this thread has handled
	int stat_req; //num of static requests this thread has handled
	int dyn_req;  //num of dynamic requests this thread has handled
	int id;
};

Thread createNewThread(int id)
{
  Thread new_thread = malloc(sizeof(struct thread_t));
   if (!new_thread)
   {
       return NULL;
   }
   else
   {
		new_thread->total_count = 0;
		new_thread->stat_req = 0;
		new_thread->dyn_req = 0;
		new_thread->id = id;
   }
   return new_thread;
}

void incDynReqCount(Thread t)
{
	t->dyn_req = t->dyn_req + 1;
}

void incStatReqCount(Thread t)
{
	t->stat_req = t->stat_req + 1;
}

void incTotalReqCount(Thread t)
{
	t->total_count = t->total_count + 1;
}

int getThreadID(Thread t)
{
    return t->id;
}

int getStatReq(Thread t)
{
    return t->stat_req;
}

int getDynReq(Thread t)
{
    return t->dyn_req;
}

int getTotalReq(Thread t)
{
    return t->total_count;
}
