
#include "request.h"
#include "segel.h"


// 
// server.c: A very, very simple web server
//
// To run:
//  ./server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

pthread_mutex_t lock;
Queue waiting_requests;
Queue in_action_requests; // requests that are being handled
pthread_cond_t waiting_request_exists;
pthread_cond_t buff_size_not_full; //there space for requests

/*This is the only function that threads will "work" in! 
 * This function is given as a parameter (the thread's starting function) when each thread is created.
 * thread means the thread struct we have created in thread.c .
 * */
void* thread_work(void* t);

// HW3: Parse the new arguments too
void getargs(int argc, char *argv[], int *port, int* thread_num, int* queue_size, int* schedalg)
{
    if (argc < 5) {
	fprintf(stderr, "Usage: %s <port>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *thread_num = atoi(argv[2]);
    *queue_size = atoi(argv[3]);
    
    if(strcmp(argv[4],"dt") == 0)
		*schedalg = 0;
    if(strcmp(argv[4],"block") == 0)
		*schedalg = 1;
    if(strcmp(argv[4],"random") == 0)
		*schedalg = 2;
    if(strcmp(argv[4],"dh") == 0)
		*schedalg = 3;
}

void* thread_work(void* t)
{
	Thread thread = (Thread)t;
	while(1)
	{
		pthread_mutex_lock(&lock);
		while (getQueueSize(waiting_requests) == 0)
		{
			pthread_cond_wait(&waiting_request_exists, &lock);
		}
		Node tmp = popFromQ(waiting_requests);
		if(!tmp)
		{
			pthread_mutex_unlock(&lock);
			return NULL;
		}
		int res = gettimeofday(getNodePickupTime(tmp),NULL);
		
		if(res != 0) //gettimeofday failed
		{
			close(getNodeFd(tmp));
			pthread_mutex_unlock(&lock);
			return NULL;
		}
		
		timersub(getNodePickupTime(tmp),getNodeArrivalTime(tmp),getNodeDispatchTime(tmp));
		
		addNewNode(in_action_requests ,tmp);
		
		int fd = getNodeFd(tmp);
	    pthread_mutex_unlock(&lock);
	    
		requestHandle(fd,thread,tmp);  //why is this not critical? Answer: Bec of sync issues!
		
		pthread_mutex_lock(&lock);
		removeNodeByFD(in_action_requests, fd);    
		pthread_cond_signal(&buff_size_not_full);
        pthread_cond_signal(&waiting_request_exists);
		pthread_mutex_unlock(&lock);
	}
	
	return NULL;
}


int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen, threads_num, max_queue_size, schedalg;
    struct sockaddr_in clientaddr;
	struct timeval timev;
	
	
	int res = pthread_cond_init(&waiting_request_exists, NULL);
	if(res != 0) return 0;
	res = pthread_cond_init(&buff_size_not_full, NULL);
	if(res != 0)
	{
		pthread_cond_destroy(&waiting_request_exists);
		return 0;
	}
	res = pthread_mutex_init(&lock, NULL);
	if(res != 0)
	{
		pthread_cond_destroy(&waiting_request_exists);
		pthread_cond_destroy(&buff_size_not_full);
		return 0;
	}
	
	//ask layan - layan says ok
	waiting_requests = createNewQueue();
	in_action_requests = createNewQueue();
	
	if(!waiting_requests || !in_action_requests)
	{
		pthread_cond_destroy(&waiting_request_exists);
		pthread_cond_destroy(&buff_size_not_full);
		pthread_mutex_destroy(&lock);
		return 0;
	}
	
	
    getargs(argc, argv, &port, &threads_num, &max_queue_size,  &schedalg);

	pthread_t * threads_array = malloc(threads_num*(sizeof(pthread_t)));
	if(!threads_array)
	{
		pthread_cond_destroy(&waiting_request_exists);
		pthread_cond_destroy(&buff_size_not_full);
		pthread_mutex_destroy(&lock);
		destroyQueue(waiting_requests);
		destroyQueue(in_action_requests);
		return 0;
	}
	for(int i = 0; i < threads_num; i++)
	{
		Thread thread = createNewThread(i);
		pthread_create(&threads_array[i], NULL, thread_work, (void*)thread);
	}
	
	
    listenfd = Open_listenfd(port);
    while (1) 
    {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

		int res = gettimeofday(&timev, NULL); //why not lock before? may change acc to contex switch
		if(res != 0)
		{
			pthread_cond_destroy(&waiting_request_exists);
		pthread_cond_destroy(&buff_size_not_full);
		pthread_mutex_destroy(&lock);
		destroyQueue(waiting_requests);
		destroyQueue(in_action_requests);
		free(threads_array);
		return 0;
		}
	
		pthread_mutex_lock(&lock);
		
		int qs_waiting_req = getQueueSize(waiting_requests);
		int qs_in_action_req = getQueueSize(in_action_requests);
		if(qs_in_action_req + qs_waiting_req >= max_queue_size) //overload
		{
			if(schedalg == 0) //drop tail
			{
				Close(connfd);   
				pthread_mutex_unlock(&lock);   
				continue;
			}
			else if(schedalg == 1) //block
			{ 
				while(getQueueSize(waiting_requests) + getQueueSize(in_action_requests) >= max_queue_size)
				{
						pthread_cond_wait(&buff_size_not_full, &lock);
				}
			}
			else if(schedalg == 2) //drop random
			{
				int qs_waiting = getQueueSize(waiting_requests); 
				if(qs_waiting == 0)  
				{
					close(connfd);
					pthread_mutex_unlock(&lock);
					continue;     
				}
				int to_delete = (int)ceil(qs_waiting * 0.5);    //ceil or down????   
				for(int i = 0; i < to_delete; i++)
				{ 
					qs_waiting = getQueueSize(waiting_requests);
					int index = rand()%(qs_waiting);    //can index+1 > qs?? no, we checked it in removeNodeByIndex
					removeNodeByindex(waiting_requests, index);
				}
			}
			else if(schedalg == 3) //drop head
			{
				Node temp = popFromQ(waiting_requests);           
				if(!temp)    
				{
					close(connfd);
					pthread_mutex_unlock(&lock);
					continue;     
				}
                     
				close(getNodeFd(temp));
				free(temp);
			}
		
		}
		addNewNode(waiting_requests, createNewNode(connfd,timev)); //timev
		pthread_cond_signal(&waiting_request_exists);
		pthread_mutex_unlock(&lock);
		
    }
	pthread_cond_destroy(&waiting_request_exists);
	pthread_cond_destroy(&buff_size_not_full);
}


    


 
