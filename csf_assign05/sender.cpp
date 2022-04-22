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

  Connection conn; 
  conn.Connection::connect(server_hostname, server_port);
  // checks if connection is open
  if (!conn.Connection::is_open()) {
    std::cerr << "Failed to connect to server\n";
    return 2;
  }

  struct Message message(TAG_SLOGIN, username);
  bool good_state;

  // TODO: send slogin message
  good_state = conn.Connection::send(message);
  if (!good_state) {
    std::cerr << "Login failed\n";
    return 3;
  }
  good_state = conn.Connection::receive(message); 
  if (message.tag == TAG_ERR || !good_state) {
    std::cerr << "Login failed for receive\n";
    return 3;  
  }

  //declaring message formation objects
  std::string data;
  bool is_done = false;

  // Data loop
  while (is_done == false) {
    std::cout << ">";
    std::getline(std::cin, data); // check return value of getline
    //    if (std::cin.bad()) {
    //  std::cerr << "I/O Error\n"; 
    // } else if (!std::cin.eof()) {
    //   std::cerr << "Format Error\n";
    // } else {
    //   std::cerr << "Format or I/O Error\n";
    // }
    std::stringstream ss(data);
    std::string command;
    ss >> command;
    message.data = data;
    if (command == "/leave") {
      message.tag = TAG_LEAVE;
      //  } else if (command == "/join" && message.tag != TAG_LEAVE) {
      // std::cerr << "Cannot join a room without leaving current room\n";
    } else if (command == "/join") {
      ss >> message.data;
      message.tag = TAG_JOIN;
    } else if (command == "/quit") {
      message.tag = TAG_QUIT;
      is_done = true;
    } else { //check if tag can be sent
      message.tag = TAG_SENDALL;
    }
    good_state = conn.Connection::send(message);
    if (!good_state) {
    std::cerr << "Error occured when sending message\n";
  }
    good_state = conn.Connection::receive(message);
    if (message.tag == TAG_ERR || !good_state) {
      std::cerr << message.data << "\n";
    }
  }

  conn.close();

  return 0;
}

// Still don't understand the use of split_payload - what is it doing that we can use to our advantage?
