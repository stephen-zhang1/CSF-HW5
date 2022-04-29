#include "guard.h"
#include "message.h"
#include "message_queue.h"
#include "user.h"
#include "room.h"

//mutex synchronization
//collection of user objects only necessary for receivers

Room::Room(const std::string &room_name)
  : room_name(room_name) {
  // TODO: initialize the mutex
  pthread_mutex_init(&lock, nullptr);
}

Room::~Room() {
  // TODO: destroy the mutex
  pthread_mutex_destroy(&lock);
}

void Room::add_member(User *user) {
  Guard g(lock);
  members.set<User*>::insert(user);

}

void Room::remove_member(User *user) {
  // TODO: remove User from the room
  Guard g(lock);
  members.set<User*>::erase(user);
}

void Room::broadcast_message(const std::string &sender_username, const std::string &message_text) {
  // TODO: send a message to every (receiver) User in the room
  Guard g(lock);
  //MessageQueue queue;
  Message *msg = new Message(sender_username, message_text);
  //TODO COME BACK TO THIS PERSON MIGHT BE NOT WHAT WE THINK
  for (std::set<User *>::iterator it=members.begin(); it!=members.end(); ++it) {
    User * person = *it;
    if (sender_username != person->username) {
      person->mqueue.MessageQueue::enqueue(msg);
    }
  }
  
  delete msg;
  //iterate
  //if statement if the user = sender_username
  //if it is then skip over it
  //if its not then call queue.enqueue
 

}
