README.txt

Michael Enaye and Stephen Zhang

Michael - Implemented sender, receiver, and parts of MS2

Stephen - Implemented connection, split_payload(), and parts of MS2


/* REPORT */

Thread synchronization in the server was done by thinking about where the critical sections could be - we observed critical sections in the following places:
 - The room collection of the server (Creating rooms can be done by multiple threads at once)
 - The user collection of the rooms (Joining and leaving rooms can be done by multiple threads at once)
 - The message queue data structure (Deque can be accessed for pushing and popping operations at the same time by the threads) 

 However, not all of them were responded to in the same way. In the case where threads should not be running code at the same time and can be accessed by each thread otherwise, we used a mutex,
 since it allows only one thread to run that code at a time before releasing its lock to a waiting thread. This was applicable for finding or creating a room, adding/removing members
 from a room, broadcasting a message to room members, and for enqueuing and dequeuing messages to the users. Only the message queue data structure used a semaphore, since it was necessary for information
 to be available before performing an operation, managing that resource. Thus, it was used in both the enqueuing and dequeuing process by the enqueue operation notifying the dequeue operation when a message
 is available to extract.

 Synchronization requirements were met in this assignment by introducing the locking of a mutex immediately when entering the data of a critical section. This would prevent other threads from access until the lock is released,
 and the lock is released on our mutex automatically when we exit each method (falling out of scope). As for the message queue, which uses a mutex AND a semaphore, we opt not to lock the mutex straight away, as doing so could 
 deadlock the server operations if there isn't a message available. Instead, we wait until sem_post() (from enqueue) wakes up the semaphore and THEN lock the mutex in dequeue, as now we actually have a message in the queue. 