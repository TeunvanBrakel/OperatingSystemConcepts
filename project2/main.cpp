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

// although it is good habit, you don't have to type 'std::' before many objects by including this line
using namespace std;

enum BufferState {
  Unbounded = static_cast<std::size_t>(-1)
};

std::stack<std::string> logs;
std::vector<int32_t> buffer; 
std::size_t BUFFER_SIZE = BufferState::Unbounded;


void write_to_log(std::string& s) {
  logs.push(s);
}

std::string read_from_string(std::size_t element) {
  if(logs.empty()) {
    std::cout << "No logs available" << std::endl;
    return "";
  }
  return logs.top();
}

void add_to_buffer(int32_t element) {
  if(BUFFER_SIZE != BufferState::Unbounded && BUFFER_SIZE <= buffer.size()) {
    std::cout << "Buffer is full" << std::endl;
    return; 
  }
  buffer.emplace_back(element);
}

int32_t remove_from_buffer() {
  if(buffer.empty()) {
    std::cout << "Can not read from an empty buffer" << std::endl;
    return -1;
  }
  int32_t r = buffer.front();
  buffer.erase(buffer.begin());
  return r; 
}

void set_bound_buffer(size_t bound) {
  if(bound < 0) {
    std::cout << "Negative buffer not allowed" << std::endl;
    return;
  }

  if(bound < buffer.size()) {
    std::cout << "There are more elements in the buffer than the requested size" << std::endl; 
    return;
  }

  BUFFER_SIZE = bound;
  buffer.reserve(bound);
}

void unbound_buffer() {
  BUFFER_SIZE = -1;
}


int main(int argc, char* argv[]) {
  set_bound_buffer(10);

  for(size_t i = 0; i <= 10; ++i) {
    add_to_buffer(i);
  }
  
	return 0;
}