#include <cassert>
#include <ctime>
#include "message_queue.h"
#include <semaphore.h>
#include "guard.h"

MessageQueue::MessageQueue() {
  //Initialize the mutex and semaphore
  pthread_mutex_init(&m_lock, nullptr);
  sem_init(&m_avail, 0, 0);
}

MessageQueue::~MessageQueue() {
  //Destroy the mutex and semaphore
  pthread_mutex_destroy(&m_lock);
  sem_destroy(&m_avail);
}

void MessageQueue::enqueue(Message *msg) {
  Guard g(m_lock);
  //Add the message to the queue
  m_messages.push_back(msg);

  // be sure to notify any thread waiting for a message to be
  // available by calling sem_post
  sem_post(&m_avail);
 
}

Message *MessageQueue::dequeue() {
  //Wait for a message to be available before dequeuing
  sem_wait(&m_avail);
  {
    Guard g(m_lock);
    Message *msg = m_messages.front();
    m_messages.pop_front();
    return msg;
  }

}
