#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include <iostream>
#include <algorithm>
#include "client_util.h"

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  rio_readinitb(&m_fdbuf, m_fd);
}

void Connection::connect(const std::string &hostname, int port) {
  const char* char_port = std::to_string(port).c_str();
  m_fd = open_clientfd(hostname.c_str(), char_port);
  if (m_fd < 0) {
    std::cerr << "Could not connect to server" << std::endl;
  }
  rio_readinitb(&m_fdbuf, m_fd);
}

Connection::~Connection() {
  this->close();
}

bool Connection::is_open() const {
  if (m_fd < 0) { 
    return false;
  }
  return true;
}

void Connection::close() {
  if (m_fd > 0) { 
    Close(m_fd);
    m_fd = -1;
  }
}

bool Connection::send(const Message &msg) {
  char * cmsg_str = new char[Message::MAX_LEN];
  std::string message = msg.tag + ":" + msg.data + "\n";
  std::string trim_message = trim(message); 
  trim_message = trim_message + "\n";
  strcpy(cmsg_str, trim_message.c_str());
  
  ssize_t bytes_written = rio_writen(m_fd, cmsg_str, strlen(cmsg_str)); // write exact number of bytes in msg
    if (bytes_written == (ssize_t)strlen(cmsg_str)) {
    m_last_result = SUCCESS;
    return true;
  }
  m_last_result = EOF_OR_ERROR; 
  return false;

}

bool Connection::receive(Message &msg) {
  //read from fd representing socket, give string, parse string
  // TODO: send a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately

  //check if u can split it in half at the colon for error handling
  //if it can be split then the message is valid
  char * cstring_message = new char[Message::MAX_LEN];
  
  ssize_t line = rio_readlineb(&m_fdbuf, cstring_message, Message::MAX_LEN);
  
   if (line <= 0) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  
  //char buffer string
  //chop at the colon
  //insert tag into msg.tag
  //insert payload into msg.payload
  //done 
  //error if i read 0 or less than 0
  //Make sure that there is only 1 colon in the c string
  int count_of_colon = std::count(cstring_message, cstring_message + msg.MAX_LEN, ':');
  if (count_of_colon == 0) {
    m_last_result = INVALID_MSG;
    return false;
  }

  char* tag_index = strchr(cstring_message, ':');
  std::string c(cstring_message);
  int tag_length = tag_index - cstring_message + 1;
  std::string tag_string = c.substr(0, tag_length - 1);
  std::string payload_string = c.substr(tag_length, Message::MAX_LEN);
  
  msg.tag = tag_string;
  msg.data = payload_string;
  m_last_result = SUCCESS;

  return true;
}
