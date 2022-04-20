#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <string>

struct Message {
  // An encoded message may have at most this many characters,
  // including the trailing newline ('\n'). Note that this does
  // *not* include a NUL terminator (if one is needed to
  // temporarily store the encoded message.)
  static const unsigned MAX_LEN = 255;

  std::string tag;
  std::string data;

  Message() { }

  Message(const std::string &tag, const std::string &data)
    : tag(tag), data(data) { }

  // returns the payload (data) as a vector of strings,
  // split using ':' as the separator
  std::vector<std::string> split_payload() const {
    std::vector<std::string> result;
    // TODO: split the message data into fields separated by ':', add them
    //       to result vector
    // text:asdasdas:asdcewfcwes:ewsdfcwefrsdg:wesfdcqwazff
    //split data separated by : and then add it to the result
    const char * colon_location = strchr(data.c_str(), ':'); //chunk of text before colon
    std::string new_input;
    int colon_index = 0;
    int previous_index;
    int size_of_text;
    while (colon_location != NULL) {
      previous_index = colon_index; //save where the previous index was to get the next input
      colon_index = colon_location - data.c_str(); //index where the colon is
      size_of_text = colon_index - previous_index; 
      new_input = data.substr(previous_index + 1,size_of_text + 1); //the chunk of text put into a string
      result.push_back(new_input); // add chunk of text before colon to vector
      colon_location = strchr(colon_location + 1,':'); //shift to look at where the next colon is
    }
    
    return result;
  }
};

// standard message tags (note that you don't need to worry about
// "senduser" or "empty" messages)
#define TAG_ERR       "err"       // protocol error
#define TAG_OK        "ok"        // success response
#define TAG_SLOGIN    "slogin"    // register as specific user for sending
#define TAG_RLOGIN    "rlogin"    // register as specific user for receiving
#define TAG_JOIN      "join"      // join a chat room
#define TAG_LEAVE     "leave"     // leave a chat room
#define TAG_SENDALL   "sendall"   // send message to all users in chat room
#define TAG_SENDUSER  "senduser"  // send message to specific user in chat room
#define TAG_QUIT      "quit"      // quit
#define TAG_DELIVERY  "delivery"  // message delivered by server to receiving client
#define TAG_EMPTY     "empty"     // sent by server to receiving client to indicate no msgs available

#endif // MESSAGE_H
