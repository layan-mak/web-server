#include "queue.h"
#include <stdlib.h>
#include <unistd.h>

//each node represents a request
struct node_t
	{
      struct node_t* next; 
      struct timeval dispatch_time;
      struct timeval arrival_time;
      struct timeval pickup_time;
      int fd;
    } ;

/* FIFO queue!
 * */
struct queue_t
	{
      Node head;
      int size;
    } ;


/* Creates a new node without linking it to the queue
 * */
//Node createNewNode(int fd)
Node createNewNode(int fd, struct timeval arrival)
{
   Node my_node=malloc(sizeof(struct node_t));
   if (!my_node)
   {
       return NULL;
   }
   else
   {
       my_node->next = NULL; 
       my_node->arrival_time = arrival;
       my_node->fd = fd;
   }
   return my_node;
}

/*Creates an empty queue
 * */
Queue createNewQueue()
{
  Queue my_queue=malloc(sizeof(struct queue_t));
  if (!my_queue)
  {
      return NULL;
  }
  else
  {
      my_queue->head=NULL;
      my_queue->size=0;  
  }
  return my_queue;
}  

void destroyQueue(Queue q)
{
	if(!q) return;
	Node curr = q->head;
	while(curr)
	{
		Node tmp = curr->next;
		close(curr->fd);
		free(curr);
		curr = tmp;
	}
	free(q);
}

/* returns the head node and removes it from queue
 * */
Node popFromQ(Queue q)
{
	if(!q->head)
  {
    return NULL;
  }
    Node temp=q->head;
    q->head = temp->next;
    q->size--;
    temp->next = NULL;   //commented just for testing!!
    return temp;	
}


/*Adds a new node and returns 1
 * */
void addNewNode(Queue q,Node node)
{
	if(!q || !node) return;
	Node temp = q->head;
	if (!temp)
	{ 
	   q->head = node;
	   (q->size)++;
	   //q->size = 1;
       return;
    }
	else
	{
		while(temp->next)
		{
			Node temp1 = temp->next;
			temp=temp1;
		}
	}
	temp->next = node;
	(q->size)++;
	return;
}

/*returns -1 if queue is NULL
 * */
int getQueueSize(Queue q)
{
	if(!q)
  {
    return -1;
  }
	return q->size;	
}

/* Reutrns -1 if node is null
 * */
int getNodeFd(Node node)
{
	if(!node)
  {
    return -1;
  }
	return node->fd;
}

/*Starts counting from 0
 * removes a node : frees its memory and closes the connection by its fd
 * what if index is bigger than size??
 * */
void removeNodeByindex(Queue q,int index)
{
	if (!q)
	{
		return;
	}
	if(index + 1 > q->size) return;
	Node temp=q->head;
	if(index==0)
	{
		temp = popFromQ(q);
		if(temp != NULL)
		{
			close(temp->fd);
			free(temp);
		}
		return;
	}
	//not head
	while (index!=0)
	{
		temp = temp->next;
		index--;
	}

	removeNodeByFD(q, getNodeFd(temp));
}

void removeNodeByFD(Queue q,int fd)
{
	if(!q) return;
	if(!q->head) return;
	Node temp = q->head;
   
	if(q->head->fd == fd)
	{
		q->head = q->head->next;
		close(temp->fd);
		free(temp);
		q->size--;
		return; 
	}

	while (temp->next && temp->next->fd!=fd)
	{ 
		temp=temp->next;
	}
	if(!temp->next) return;
	
	Node res=temp->next;
	temp->next=res->next;
	close(res->fd);
	free(res);
	q->size--; 
	return;
}



struct timeval* getNodeArrivalTime(Node node)
{
	return &(node->arrival_time);
}

struct timeval* getNodeDispatchTime(Node node)
{
	return &(node->dispatch_time);
}

struct timeval* getNodePickupTime(Node node)
{
	return &(node->pickup_time);
}
