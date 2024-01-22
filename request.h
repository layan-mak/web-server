#ifndef __REQUEST_H__

#include "queue.h"
#include "thread.h"


void requestHandle(int fd, Thread thread, Node req);

#endif
