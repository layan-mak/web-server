# web-server
I upgraded a basic web server that has a single thread of control to make it multi-threaded.

Multi-threaded:
- The server consists of a master thread that begins by creating a pool of worker threads, the number of which is specified on the command line. The master
thread is then responsible for accepting new HTTP connections over the network and placing the connection descriptor in the queue.
- Each worker thread can handle both static and dynamic requests. A worker thread wakes when there is an HTTP request in the queue; when there are multiple HTTP requests
available, it should pick the oldest request in the queue.
- Once the worker thread wakes, it performs the read on the network descriptor, obtains the specified content (by either reading the static file or executing the CGI process), and then returns the content to the client by writing to the descriptor. The worker thread then waits for another HTTP request. This is implemented using **condition variables** and no busy-waiting.

Overload Handling:
- block: The code thread blocks until a buffer becomes available.
- drop_tail: The code drops the new request immediately by closing the socket and continues listening for new requests.
- drop_head: The code drops the oldest request in the queue that is not currently being processed by a thread and adds the new request to the end of the queue.

Usage Statistics:
- The web server collects a variety of statistics; per-request and per-thread basis.

Use the makefile to compile.
