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
  //Connects to server
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
  //Initialize array to hold trimmed message
  char * cmsg_str = new char[Message::MAX_LEN];
  std::string message = msg.tag + ":" + msg.data + "\n";
  std::string trim_message = trim(message); 
  trim_message = trim_message + "\n";
  strcpy(cmsg_str, trim_message.c_str());
  
  //Send the message that is in the correct format
  ssize_t bytes_written = rio_writen(m_fd, cmsg_str, strlen(cmsg_str)); // Write exact number of bytes in msg
  if (bytes_written == (ssize_t)strlen(cmsg_str)) {
    m_last_result = SUCCESS;
    delete[] cmsg_str;
    return true;
  }
  m_last_result = EOF_OR_ERROR;
  delete[] cmsg_str; 
  return false;

}

bool Connection::receive(Message &msg) {
  //Initialize array to read message into
  char * cstring_message = new char[Message::MAX_LEN]();
  ssize_t line = rio_readlineb(&m_fdbuf, cstring_message, Message::MAX_LEN);
  
  //Nothing was read
  if (line <= 0) {
    m_last_result = EOF_OR_ERROR;
    delete[] cstring_message;
    return false;
  }
  
  //Check if you can split by the colon
  int count_of_colon = std::count(cstring_message, cstring_message + msg.MAX_LEN, ':');
  if (count_of_colon == 0) {
    m_last_result = INVALID_MSG;
    delete[] cstring_message;
    return false;
  }
  
  //Find where colon is and then split it into tag and data
  char* tag_index = strchr(cstring_message, ':');
  std::string c(cstring_message);
  int tag_length = tag_index - cstring_message + 1;
  std::string tag_string = c.substr(0, tag_length - 1);
  std::string payload_string = c.substr(tag_length, Message::MAX_LEN);
  
  //Set the message tag and data to their respective strings
  msg.tag = tag_string;
  msg.data = payload_string;
  m_last_result = SUCCESS;
  delete[] cstring_message;
  
  return true;
}
