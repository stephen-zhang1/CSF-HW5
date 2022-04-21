#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  Connection conn;

  // TODO: connect to server
  conn.Connection::connect(server_hostname, server_port);
  if (!conn.Connection::is_open()) {
    std::cerr << "Failed to connect to server\n";
    return 2;
  }
  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)
  struct Message message(TAG_RLOGIN, username);
  bool good_state;

  good_state = conn.Connection::send(message);
  if (!good_state) {
    std::cerr << message.data << "\n";
    return 3;
  }
  good_state = conn.Connection::receive(message); 
  if (message.tag == TAG_ERR || !good_state) {
    std::cerr << message.data << "\n";
    return 3;  
  }

  message.tag = TAG_JOIN;
  message.data = room_name;

  good_state = conn.Connection::send(message);
  if (!good_state) {
    std::cerr << message.data << "\n";
    return 4;
  }
  good_state = conn.Connection::receive(message); 
  if (message.tag == TAG_ERR || !good_state) {
    std::cerr << message.data << "\n";
    return 4;  
  }

  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)
  bool loop = true;

  while (loop) {
    good_state = conn.Connection::receive(message);
    if (message.tag != TAG_DELIVERY || !good_state) {
    std::cerr << "Failed to receive message\n";
    } else {
    std::vector<std::string> payload = message.split_payload();
    std::cout << "[" << payload[1] << "]: [" << payload[2] << "]\n"; 
    }
  }

  conn.close();
  return 0;
}
