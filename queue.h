#ifndef QUEUE_H
#define QUEUE_H
#include <sys/time.h>

typedef struct node_t* Node;
typedef struct queue_t* Queue;

Node  createNewNode(int fd, struct timeval arrival);
Queue createNewQueue();
void destroyQueue(Queue q);
Node popFromQ(Queue q);
void addNewNode(Queue q,Node node);
int getQueueSize(Queue q);
int getNodeFd(Node node);
void removeNodeByindex(Queue q,int index);
void removeNodeByFD(Queue q,int fd);

struct timeval* getNodeArrivalTime(Node node);
struct timeval* getNodeDispatchTime(Node node);
struct timeval* getNodePickupTime(Node node);


#endif
