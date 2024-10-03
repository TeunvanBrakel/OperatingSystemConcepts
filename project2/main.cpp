/**
  * Assignment: synchronization
  * Operating Systems
  */

/**
  Hint: F2 (or Control-klik) on a functionname to jump to the definition
  Hint: Ctrl-space to auto complete a functionname/variable.
  */

// function/class definitions you are going to use
#include <algorithm>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <stack>
#include <map>
#include <functional>

// although it is good habit, you don't have to type 'std::' before many objects by including this line
using namespace std;

enum BufferState {
  Unbounded = static_cast<std::size_t>(-1)
};

//I want this to be a pair of <LogType, std::string> however I do not know if this allowed so I won't :(
std::stack<std::string> logs;
std::vector<int32_t> buffer; 
std::size_t BUFFER_SIZE = BufferState::Unbounded;

enum ErrorLogs {
  NO_LOGS_AVAILABLE,
  FULL_BUFFER,
  EMPTY_BUFFER,
  NEGATIVE_BUFFER_BOUND,
  REQUESTED_BOUND_TO_LOW
};

std::map<std::size_t, std::string> ErrorMessages {
  {NO_LOGS_AVAILABLE, "[Error] No logs are available"},
  {FULL_BUFFER, "[Error] Buffer is full"},
  {EMPTY_BUFFER, "[Error] Buffer is empty"},
  {NEGATIVE_BUFFER_BOUND, "[Erorr] Buffer bound can't be negative"},
  {REQUESTED_BOUND_TO_LOW, "[Error] The buffer contains more elements than the requested bound"}
};

enum SuccesLogs {
  ADDED_TO_BUFFER,
  REMOVED_FROM_BUFFER,
  SET_BOUND,
  UNBOUND
};

const std::string FormatSuccesMessage(const int item, std::string custom_message) {
    return "[Succes] " + std::to_string(item) + custom_message;
}

std::map<std::size_t, std::function<const std::string(int)>> SuccesMessages = {
    {ADDED_TO_BUFFER, [] (const int item) { return FormatSuccesMessage(item, "Item added to buffer" ); } },
    {REMOVED_FROM_BUFFER, [] (const int item) { return FormatSuccesMessage(item, "Item removed from bufer"); }},
    {SET_BOUND, [] (const int item) { return FormatSuccesMessage(item, "set as bound"); }},
    {UNBOUND, [] (const int _) { return FormatSuccesMessage(_, "Unbounded buffer");}}
};
void write_to_log(const std::string& s) {
  logs.push(s);
}

std::string read_from_string(std::size_t element) {
  if(logs.empty()) {
    write_to_log(ErrorMessages[NO_LOGS_AVAILABLE]);
    return "";
  }
  return logs.top();
}

void add_to_buffer(int32_t element) {
  if(BUFFER_SIZE != BufferState::Unbounded && BUFFER_SIZE <= buffer.size()) {
    write_to_log(ErrorMessages[FULL_BUFFER]);
    return; 
  }
  write_to_log(SuccesMessages[ADDED_TO_BUFFER](element));
  buffer.emplace_back(element);
}

int32_t remove_from_buffer() {
  if(buffer.empty()) {
    write_to_log(ErrorMessages[EMPTY_BUFFER]);
    return -1;
  }
  int32_t r = buffer.front();
  buffer.erase(buffer.begin());
  write_to_log(SuccesMessages[REMOVED_FROM_BUFFER](r));
  return r; 
}

void set_bound_buffer(size_t bound) {
  if(bound == BufferState::Unbounded) {
    write_to_log(ErrorMessages[NEGATIVE_BUFFER_BOUND]);
    return;
  }

  if(bound < buffer.size()) {
    write_to_log(ErrorMessages[REQUESTED_BOUND_TO_LOW]);
    return;
  }

  BUFFER_SIZE = bound;
  buffer.reserve(bound);
  write_to_log(SuccesMessages[SET_BOUND](bound));
}

void unbound_buffer() {
  BUFFER_SIZE = -1;
  write_to_log(SuccesMessages[UNBOUND](-1));
}


int main(int argc, char* argv[]) {
  remove_from_buffer();
  std::cout<< read_from_string(0) << endl;
  string s = "test";
  write_to_log(s);
  set_bound_buffer(9);
  std::cout << read_from_string(0)<<endl;
  for(size_t i = 0; i <= 10; ++i) {
    add_to_buffer(i);
  }
  unbound_buffer();
  add_to_buffer(11);
  std::cout<< buffer.size() <<endl;
  set_bound_buffer(5);
  std::cout<< remove_from_buffer() <<endl;
  set_bound_buffer(-1);
  
	return 0;
  
 return 0;
}