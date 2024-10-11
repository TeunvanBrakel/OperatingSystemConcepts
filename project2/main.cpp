/**
  * Assignment: synchronization
  * Operating Systems
  */

/**
  Hint: F2 (or Control-klik) on a functionname to jump to the definition
  Hint: Ctrl-space to auto complete a functionname/variable.
  */


 /*
  TODO 
    Fix buffer and bound semaphores 
    Talk about the problem with multiple iterations of readers and writers 
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
#include <variant>
#include <chrono>
#include <atomic>
#include <cassert>
#include <unordered_set>

// although it is good habit, you don't have to type 'std::' before many objects by including this line
using namespace std;

template <typename T>
struct Ok {
  public:
    explicit constexpr Ok(T value) : value(std::move(value)) {}

    constexpr T&& take_value() { return std::move(value); }

    T value;
};

template <typename T>
struct Err {
  public:
    explicit constexpr Err(T value) : value(std::move(value)) {}

    constexpr T&& take_value() { return std::move(value); }

    T value;
};

template <typename OkT, typename ErrT>
struct Result {
  public:
    using VariantT = std::variant<Ok<OkT>, Err<ErrT>>;

    constexpr Result(Ok<OkT> value) : variant(std::move(value)) {}
    constexpr Result(Err<ErrT> value) : variant(std::move(value)) {}

    constexpr bool is_ok() const { return std::holds_alternative<Ok<OkT>>(variant); }
    constexpr bool is_err() const { return std::holds_alternative<Err<ErrT>>(variant); }

    constexpr OkT ok_value() const { return std::get<Ok<OkT>>(variant).value; }
    constexpr ErrT err_value() const { return std::get<Err<ErrT>>(variant).value; }

    constexpr OkT&& unwrap() { return std::get<Ok<OkT>>(variant).take_value(); }
    constexpr ErrT&& err() { return std::get<Err<ErrT>>(variant).take_value(); }

    VariantT variant;
};

inline static std::chrono::steady_clock::time_point get_5_seconds_timeout() {
    return std::chrono::steady_clock::now() + std::chrono::seconds(5);
}

enum BufferState {
  Unbounded = static_cast<std::size_t>(-1)
};

//Log mutexes
std::mutex write_to_log_allowed_mtx;
std::mutex read_from_log_allowed_mtx;
std::atomic<int32_t> n_readers {static_cast<int32_t>(0)};

//Buffer mutexes
std::mutex read_from_buffer_mtx;
std::mutex write_to_buffer_mtx;
std::mutex bound_semaphore_mtx;
std::atomic<int32_t> n_write_to_buffer;
std::atomic<int32_t> n_read_from_buffer;

//General data 
std::stack<std::string> logs;
std::vector<int32_t> buffer; 
std::atomic<size_t> BUFFER_SIZE = BufferState::Unbounded;

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
    {SET_BOUND, [] (const int item) { return FormatSuccesMessage(item, " set as bound"); }},
    {UNBOUND, [] (const int _) { return FormatSuccesMessage(_, "Unbounded buffer");}}
};

void write_to_log(const std::string& s) {
  const std::lock_guard<std::mutex> l(read_from_log_allowed_mtx);
  while(n_readers > 0); //Could take infinite time //Critical section because of n_readers

  const std::lock_guard<std::mutex> l2(write_to_log_allowed_mtx);
  logs.push(s); //Critical section
}

Result<std::string, std::string> read_from_log() {
    read_from_log_allowed_mtx.lock();
    ++n_readers; //Critical section
    read_from_log_allowed_mtx.unlock();
    while(n_readers > 0) { //Critical section because of n_readers
      if(logs.empty()) { //Critical section
          write_to_log(ErrorMessages[NO_LOGS_AVAILABLE]);
          --n_readers; //Critical section
          return Err<std::string>("Log is empty");
      }
      string result = logs.top(); //Critical section
      --n_readers; //Critical section
      return Ok(result);
    }
}

void add_to_buffer(int32_t element) {
  const std::lock_guard<std::mutex> l(bound_semaphore_mtx); 
  const std::lock_guard<std::mutex> lock2(read_from_buffer_mtx);
  if(BUFFER_SIZE != BufferState::Unbounded && BUFFER_SIZE <= buffer.size()) { //Critical section
    write_to_log(ErrorMessages[FULL_BUFFER]);
    return; 
  }

  while(n_read_from_buffer > 0); //Critical section because of n_read_from_buffer

  ++n_write_to_buffer; //Critical section
  const std::lock_guard<std::mutex> lock(write_to_buffer_mtx);
  write_to_log(SuccesMessages[ADDED_TO_BUFFER](element));
  buffer.emplace_back(element); //Critical section
  --n_write_to_buffer; //Critical section
}

Result<int32_t, string> remove_from_buffer() {
  const std::lock_guard<std::mutex> l(read_from_buffer_mtx); 
  ++n_read_from_buffer; //Critical section
 
  if(buffer.empty()) { //Critical section
    write_to_log(ErrorMessages[EMPTY_BUFFER]);
    --n_read_from_buffer; //Critical section
    return Err<std::string>(ErrorMessages[EMPTY_BUFFER]);
  }

  int32_t r = buffer.front(); //Critical section
  buffer.erase(buffer.begin()); //Critical section
  write_to_log(SuccesMessages[REMOVED_FROM_BUFFER](r));
  --n_read_from_buffer; //Critical section
  return Ok<int32_t>(r); 
}

void set_bound_buffer(size_t bound) {
  {
    const std::lock_guard<std::mutex> l(bound_semaphore_mtx); 
    const std::lock_guard<std::mutex> l2(read_from_buffer_mtx);
    if(bound == BufferState::Unbounded) { //Critical section
      write_to_log(ErrorMessages[NEGATIVE_BUFFER_BOUND]);
      return;
    }

    if(bound < buffer.size()) { //Critical section
      write_to_log(ErrorMessages[REQUESTED_BOUND_TO_LOW]);
      return;
    }
    
    BUFFER_SIZE = bound; //Critical section
    buffer.reserve(bound); //Critical section
  }
  write_to_log(SuccesMessages[SET_BOUND](bound));
}

void unbound_buffer() {
  BUFFER_SIZE = -1; //Critical section
  write_to_log(SuccesMessages[UNBOUND](-1));
}


/* TESTING FUNCTIONS */
void test_buffer_integrity(std::size_t expected_size) {
    assert(buffer.size() == expected_size);

    std::unordered_set<int> unique_values(buffer.begin(), buffer.end());
    assert(unique_values.size() == expected_size);

    for (std::size_t i = 0; i < expected_size; ++i) {
        assert(unique_values.find(i) != unique_values.end()); 
    }

    for(std::size_t i = 0; i < expected_size; ++i) {
      std::cout << buffer[i] << "\n";
    }
    std::cout << endl;
    
    std::cout << "Buffer integrity test passed!" << std::endl;
}

std::size_t run_threads_for_testing(std::size_t n_threads) {
    std::vector<std::thread> threads;
    for (std::size_t i = 0; i < n_threads; ++i) {
        threads.push_back(std::thread(add_to_buffer, i));
    }

    for (auto& th : threads) {
        th.join();
    }
    return n_threads;
}


int main(int argc, char* argv[]) {
  auto n = run_threads_for_testing(100);
  test_buffer_integrity(n);
	return 0;
}