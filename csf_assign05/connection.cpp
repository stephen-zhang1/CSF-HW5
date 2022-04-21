#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

#include <iostream>
#include <algorithm>

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

  //get the vector that holds the payload
  std::vector<std::string> message_string = msg.split_payload();
  std::string data;
  for (std::vector<std::string>::iterator it = message_string.begin() ; it != message_string.end(); ++it) {
    data = data + *it;
  }

  //C string to copy over result from split_payload
  char * cmsg_str = new char[msg.MAX_LEN];

  //Make a string to copy into the C string
  std::string message = msg.tag + ":" + data + "\n";
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
  
  //std::string st(cstring_message);
  

  
  //char buffer string
  //chop at the colon
  //insert tag into msg.tag
  //insert payload into msg.payload
  //done 
  //error if i read 0 or less than 0
  //Make sure that there is only 1 colon in the c string
  int count_of_colon = std::count(cstring_message, cstring_message + msg.MAX_LEN, ':');
  if (count_of_colon != 1) {
    m_last_result = INVALID_MSG;
    return false;
  }

  if (line <= 0) {
    m_last_result = EOF_OR_ERROR;
    return false;
  }

  //If we split the array maybe check if its equal to msg.tag and msg.data
  char* tag_index = strchr(cstring_message, ':');
  std::string c(cstring_message);
  int tag_length = tag_index - cstring_message + 1;
  std::string tag_string = c.substr(0,tag_length - 1);
  std::string payload_string = c.substr(tag_length + 1);
  
  msg.tag = tag_string;
  msg.data = payload_string;

  //Check if the tag matches, payload matches, and the size of the message read is the same
  if (line == (ssize_t)strlen(cstring_message)) {
    m_last_result = SUCCESS;
    return true;
  }
  m_last_result = INVALID_MSG;
  return false;
}
