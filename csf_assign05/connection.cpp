#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

#include <iostream>

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, m_fd);
}

void Connection::connect(const std::string &hostname, int port) {
  // TODO: call open_clientfd to connect to the server
  const char* char_port = std::to_string(port).c_str();
  m_fd = open_clientfd(hostname.c_str(), char_port);
  if (m_fd < 0) {
    std::cerr << "Could not connect to server" << std::endl;
  }
  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, m_fd);
}

Connection::~Connection() {
  // TODO: close the socket if it is open
  this->close();
}

bool Connection::is_open() const {
  // TODO: return true if the connection is open
  if (m_fd < 0) { 
    return false;
  }
  return true;
}

void Connection::close() {
  // TODO: close the connection if it is open
  if (m_fd > 0) { 
    Close(m_fd);
    m_fd = -1;
  }
}

bool Connection::send(const Message &msg) {
  // TODO: send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately

  std::vector<std::string> message_string = msg.split_payload();
  //C string to copy over result from split_payload
  char * cmsg_str = new char[message_string[0].length() + message_string[1].length() + message_string[2].length() + 1];

  //Make a string to copy into the C string
  std::string message = message_string[0] + message_string[1] + message_string[2];
  strcpy (cmsg_str, message.c_str());

  //Check if the bytes written is the same as length of the C string
  ssize_t bytes_written = rio_writen(m_fd, cmsg_str, msg.MAX_LEN);
  if (bytes_written == (ssize_t)strlen(cmsg_str)) {
    m_last_result = SUCCESS;
    return true;
  }
  m_last_result = EOF_OR_ERROR; //Which result to put? EOF_OR_ERROR or INVALID_MSG
  return false;
}

bool Connection::receive(Message &msg) {
  //read from fd representing socket, give string, parse string
  // TODO: send a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately

  //check if u can split it in half at the colon for error handling
  //if it can be split then the message is valid
  std::vector<std::string> string_message = msg.split_payload(); //["TAG", ":", "PAYLOAD"]
  char * cstring_message = new char[msg.MAX_LEN];
  
  ssize_t line = rio_readlineb(&m_fdbuf, cstring_message, msg.MAX_LEN);
  //Confused what to do from here, what does readlineb return

  //If we split the array maybe check if its equal to msg.tag and msg.data
  char * colon_found = strchr(cstring_message, ':');
  if (colon_found == NULL) {
    m_last_result = INVALID_MSG;
    return false;
  }
  std::string c(cstring_message);
  int tag_length = colon_found - cstring_message + 1;
  std::string tag_string = c.substr(0,tag_length - 1);
  std::string payload_string = c.substr(tag_length);
  if (tag_string == msg.tag) {
    return true;
  }
  if (payload_string == msg.data) {
    return true;
  }
  return false;
}
