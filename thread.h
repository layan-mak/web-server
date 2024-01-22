#ifndef THREADS_H
#define THREADS_H

typedef struct thread_t* Thread;
Thread createNewThread(int id);
void incDynReqCount(Thread t);
void incStatReqCount(Thread t);
void incTotalReqCount(Thread t);
int getThreadID(Thread t);
int getStatReq(Thread t);
int getDynReq(Thread t);
int getTotalReq(Thread t);

#endif
