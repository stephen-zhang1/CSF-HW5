#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  // TODO: connect to server
  Connection conn; 
  conn.Connection::connect(server_hostname, server_port);
  // checks if connection is open
  if (!conn.Connection::is_open()) {
    std::cerr << "Failed to connect to server\n";
    return 2;
  }

  struct Message message(TAG_SLOGIN, username);
  struct Message verify(TAG_OK, "ok");

  // TODO: send slogin message
  conn.Connection::send(message);
  conn.Connection::receive(verify); // 

  //declaring message formation objects
  std::string data;
  std::stringstream ss;
  bool is_done = false;

  // TODO: loop reading commands from user, sending messages to
  //       server as appropriate
  while (!is_done) {
    std::getline(std::cin, data, ' ');
    ss << data;
    message.data = data;
    if (data == "/leave") {
      message.tag = TAG_LEAVE;
    } else if (data == "/join") {
      message.tag = TAG_JOIN;
    } else if (data == "/quit") {
      message.tag = TAG_QUIT;
      is_done = true;
    } else {
      message.tag = TAG_SENDALL;
      message.data = data;
    }
    if (message.tag != TAG_SENDALL) {
    std::getline(std::cin, data);
    ss << data;
    message.data = data;
    }
    message.split_payload();
    conn.send(message);
    conn.receive(verify);
  }

  conn.close();

  return 0;
}