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

  Message message = new Message(username, TAG_SLOGIN);
  Message verify = new Message(username, TAG_OK);

  // TODO: send slogin message
  conn.send(message);
  conn.receive(verify);
  message = new Message(username, TAG_JOIN);
  conn.send(message);
  conn.receive(verify);

  //declaring message formation objects
  std::string data;
  std::stringstream ss;
  bool done = false;

  // TODO: loop reading commands from user, sending messages to
  //       server as appropriate
  while (!done) {
    std::getline(std::cin, message);
    ss << data;
    message = new Message(username, message);
    if (message.data == "/leave") {
      done = true;
    }
    conn.send(message);
    conn.receive(verify);
  }

  message = new Message(username, TAG_QUIT);
  conn.send(message);
  conn.receive(verify);
  conn.close();

  return 0;
}
