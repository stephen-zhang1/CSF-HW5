#include "guard.h"
#include "message.h"
#include "message_queue.h"
#include "user.h"
#include "room.h"
#include "client_util.h"

Room::Room(const std::string &room_name)
  : room_name(room_name) {
  //Initialize the mutex
  pthread_mutex_init(&lock, nullptr);
}

Room::~Room() {
  //Destroy the mutex
  pthread_mutex_destroy(&lock);
}

void Room::add_member(User *user) {
  Guard g(lock);
  //Add the user to the room
  members.set<User*>::insert(user);

}

void Room::remove_member(User *user) {
  Guard g(lock);
  //Remove the user from the room
  members.set<User*>::erase(user);
}

void Room::broadcast_message(const std::string &sender_username, const std::string &message_text) {
  Guard g(lock);
  std::string message;
  //Send a message to every user in the room by iterating through the set of members
  for (User * user : members) {
    //Send the message to every user in the room but the sender
    if (sender_username != user->username) {
      //Create the message to send
      message = trim(this->room_name) + ":" + sender_username + ":" +  message_text;
      Message *msg = new Message(TAG_DELIVERY, message);
      user->mqueue.enqueue(msg);
    }
  }

}
