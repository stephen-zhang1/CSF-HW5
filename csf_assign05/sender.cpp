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
  Connection conn(); 
  conn.connect(server_hostname, server_port);
  // checks if connection is open
  if (!conn.is_open) {
    cerr << "Failed to connect to server\n";
    return 2;
  }

  // TODO: send slogin message
  conn.send(TAG_SLOGIN);
  conn.receive(TAG_OK);
  conn.send(TAG_JOIN);
  conn.receive(TAG_OK);

  //declaring message formation objects
  std::string data;
  std::stringstream ss;
  Message message;
  bool done = false;

  // TODO: loop reading commands from user, sending messages to
  //       server as appropriate
  while (!done) {
    std::getline(std::cin, message);
    ss << data;
    message = new Message(username, message);
    if (message == "/leave") {
      done = true;
    }
    conn.send(message);
    conn.receive(TAG_OK);
  }

  conn.send(TAG_QUIT);
  conn.receive(TAG_OK);
  conn.close();

  return 0;
}
