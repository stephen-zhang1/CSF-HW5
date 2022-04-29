#include <pthread.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <set>
#include <vector>
#include <cctype>
#include <cassert>
#include "message.h"
#include "connection.h"
#include "user.h"
#include "room.h"
#include "guard.h"
#include "server.h"

////////////////////////////////////////////////////////////////////////
// Server implementation data types
////////////////////////////////////////////////////////////////////////

struct ConnInfo {
  Connection *conn;
  Server *server;

  ConnInfo(Connection *conn, Server *server) : conn(conn), server(server) { }
  ~ConnInfo() {
    // destroy connection when ConnInfo object is destroyed
    delete conn;
  }
};

////////////////////////////////////////////////////////////////////////
// Client thread functions
////////////////////////////////////////////////////////////////////////

namespace {

void chat_with_receiver(Connection *conn, Server *server, User *user, const std::string &data) {
  //look at receiver diagram from server perspective
  // assume that the receiver has already sent "rlogin"
  // and "join" messages, so we know the receiver's username
  // and the name of the room the receiver wants to join
  Message *msg = new Message();
  //conn->receive(msg);
  msg->data = data;
  Room *room = server->find_or_create_room(msg->data);

  room->add_member(user);
  
  while (true) {
    // try to dequeue a Message from the user's MessageQueue
    msg = user->mqueue.dequeue();
    // if a Message was successfully dequeued, send a "delivery"
    // message to the receiver. If the send is unsuccessful,
    // break out of the loop (because it's likely that the receiver
    // has exited and the connection is no longer valid)
    if (msg != nullptr) {
      if (conn->send(*msg) == false) {
        delete msg;
        break;
      }
    }
    delete msg;
  }

  // make sure to remove the User from the room
  room->remove_member(user);
  
}

void chat_with_sender(Connection *conn, Server *server, User *user, const std::string &data) {
  //look at sender diagram from server perspective
  //conn.receive
  //m.data
  // Call receive w/
  Message *msg = new Message();
  msg->data = data;
  Room *room = server->find_or_create_room(msg->data);
  room->add_member(user);
  //room->add_member(user);
  //guard on message queue(enqueue dequeue), guard on remove member (room), add member (room), 
  while (true) {
    conn->receive(*msg);
    if (msg->tag == TAG_JOIN) {
      room = server->find_or_create_room(msg->data);
      room->add_member(user);
    } else if (msg->tag == TAG_LEAVE) {
      room->remove_member(user);
    } else if (msg->tag == TAG_SENDALL) {
      room->broadcast_message(user->username, msg->data);
    } else if (msg->tag == TAG_QUIT) {
      conn->send(*msg);
      break;
    }
  }
  delete msg;
  //join leave sendall quit are valid mesages
  //find or create room for join
  //current room pointer to null
  //sendall call broadcast message
  //quit send back an ok and break out of the loop
}


//call broadcast message on room object when receiver thread receives message
//broadcast message creates message objects put into the queues 


//worker determines whether the client is a sender or receiver 
void *worker(void *arg) {
  pthread_detach(pthread_self());

  ConnInfo *info_ = static_cast<ConnInfo *>(arg);

  // use a std::unique_ptr to automatically destroy the ConnInfo object
  // when the worker function finishes; this will automatically ensure
  // that the Connection object is destroyed
  std::unique_ptr<ConnInfo> info(info_);

  Message msg;
  User *user = new User(""); //i declared a user object

  if (!info->conn->receive(msg)) {
    if (info->conn->get_last_result() == Connection::INVALID_MSG) {
      info->conn->send(Message(TAG_ERR, "invalid message"));
    }
    return nullptr;
  }

  if (msg.tag != TAG_SLOGIN && msg.tag != TAG_RLOGIN) {
    info->conn->send(Message(TAG_ERR, "first message should be slogin or rlogin"));
    return nullptr;
  }
  
  user->username = msg.data;
  //std::string username = msg.data;
  if (!info->conn->send(Message(TAG_OK, "welcome " + user->username))) {
    return nullptr;
  }
  // Just loop reading messages and sending an ok response for each one


  if (msg.tag == TAG_SLOGIN) {
    // Loop until you get a join message
    while (msg.tag != TAG_JOIN) {
      if (!info->conn->receive(msg)) {
        info->conn->send(Message(TAG_ERR, "invalid message"));
        return nullptr;
      }
      if (msg.tag != TAG_JOIN) {
        info->conn->send(Message(TAG_ERR, "you must enter a room before sending a message"));
      }
    }
    chat_with_sender(info->conn, info->server, user, msg.data);
  } else if (msg.tag == TAG_RLOGIN) {
      while (msg.tag != TAG_JOIN) {
        if (!info->conn->receive(msg)) {
          info->conn->send(Message(TAG_ERR, "invalid message"));
          return nullptr;
        }
        if (msg.tag != TAG_JOIN) {
          info->conn->send(Message(TAG_ERR, "you must enter a room before sending a message"));
        }
    }
    chat_with_receiver(info->conn, info->server, user, msg.data);
  }

  delete user;
  return nullptr;
}

}

////////////////////////////////////////////////////////////////////////
// Server member function implementation
////////////////////////////////////////////////////////////////////////

Server::Server(int port)
  : m_port(port)
  , m_ssock(-1) {
  pthread_mutex_init(&m_lock, nullptr);
}

Server::~Server() {
  pthread_mutex_destroy(&m_lock);
}

bool Server::listen() {
  std::string port = std::to_string(m_port);
  m_ssock = open_listenfd(port.c_str());
  return m_ssock >= 0;
}

void Server::handle_client_requests() {
  assert(m_ssock >= 0);

  while (true) {
    int clientfd = accept(m_ssock, nullptr, nullptr);
    if (clientfd < 0) {
      std::cerr << "Error accepting connection\n";
      return;
    }

    ConnInfo *info = new ConnInfo(new Connection(clientfd), this);

    pthread_t thr_id;
    if (pthread_create(&thr_id, nullptr, worker, static_cast<void *>(info)) != 0) {
      std::cerr << "Could not create thread\n";
      return;
    }
  }
}

Room *Server::find_or_create_room(const std::string &room_name) {
  // this function can be called from multiple threads, so
  // make sure the mutex is held while accessing the shared
  // data (the map of room names to room objects)


  //needs mutex synchronization because theres only 1 unique

  Guard g(m_lock);

  Room *room;

  auto i = m_rooms.find(room_name);
  if (i == m_rooms.end()) {
    // room does not exist yet, so create it and add it to the map
    room = new Room(room_name);
    m_rooms[room_name] = room;
  } else {
    room = i->second;
  }

  return room;
}