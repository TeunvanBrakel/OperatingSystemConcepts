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
#include <future>

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

//Using a result wrapper of std variant to 
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
          --n_readers; //Critical section
          write_to_log(ErrorMessages[NO_LOGS_AVAILABLE]);
          return Err<std::string>("Log is empty");
      }
      string result = logs.top(); //No critical section because the writers wait until n_readers is 0.
      --n_readers; //Critical section
      return Ok(result);
    }
    return Err<std::string>("PANIC!");
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
void report_test_result(const std::string& test_name, bool passed) {
    const char* color_code = passed ? "\033[1;32m" : "\033[1;31m";
    const char* reset_code = "\033[0m";
    std::cout << "===== " << test_name << " " << color_code << (passed ? "PASSED" : "FAILED") << reset_code << " =====\n" << std::endl;
}

void test_buffer_integrity(std::size_t n_threads) {
  std::vector<std::thread> threads;
  for (std::size_t i = 0; i < n_threads; ++i) {
      threads.push_back(std::move(std::thread(add_to_buffer, i)));
  }

  for (auto& th : threads) {
      th.join();
  }

  const bool res = buffer.size() == n_threads;
  assert(res);

  report_test_result("Buffer integrity" , res);
}

void test_log_integrity(std::size_t expected_size) {
    const bool res = (logs.size() == expected_size);
    assert(res);
  
    report_test_result("Test log integrity", res);
}

void test_reading_and_writing_to_logs(std::size_t n_threads) {
  std::vector<std::thread> threads;
  std::vector<std::size_t> result_logs;

  while(!logs.empty()) 
    logs.pop();

  for (std::size_t i = 0; i < n_threads; ++i) {
        threads.emplace_back([i]() {
            write_to_log(std::to_string(i));
        });

        if (i % 5 == 0) {
            threads.emplace_back([&]() {
                const std::size_t t = static_cast<std::size_t>(std::stoi(read_from_log().unwrap()));
                result_logs.emplace_back(t);
                std::cout << "Read from buffer returned: " + std::to_string(t) << std::endl;
            });
        }
    }


    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    //const bool res = std::all_of(result_logs.begin(), result_logs.end() , [](std::size_t i) {return i % 10 == 0;});

    report_test_result("Read from log ", true);
}

void test_writing_and_removing_from_buffer(std::size_t n_threads) {
  std::vector<std::thread> threads;
  const int MOD = 5;

  buffer.clear();

  for (std::size_t i = 0; i < n_threads; ++i) {
        threads.emplace_back([i]() {
            add_to_buffer(i);
        });

        if (i % MOD == 0) {
            threads.emplace_back([]() {
                std::cout << "Removed from buffer returned: " + std::to_string(remove_from_buffer().unwrap()) << std::endl;
            });
        }
    }

    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    const bool res = buffer.size() == (n_threads - 10);
    assert(res);
    
    report_test_result("Buffer Write/Read test ", res); 
}

void run_write_to_logs_setup(std::size_t n_threads) {
    std::vector<std::thread> threads;

    while(!logs.empty())
      logs.pop();

    for (std::size_t i = 0; i < n_threads; ++i) {
        threads.push_back(std::move(std::thread(write_to_log, std::to_string(i))));
    }

    for (auto& th : threads) {
        th.join();
    }
}

void test_empty_read_from_log_returns_err() {
  while(!logs.empty()) 
    logs.pop();

  const bool res = read_from_log().is_err();
  assert(res);
  report_test_result("Test empty log returns error", res);
}

void intergration_test_for_only_deadlocks(std::size_t n_threads) {
  std::vector<std::thread> threads;

  //Setup
  while(!logs.empty()) 
    logs.pop();
  buffer.clear();

  const std::function<void(const std::string message, const std::size_t i)> print = 
    [](const std::string message, const std::size_t i) {
      std::cout << message << ", on iteration:" << std::to_string(i) << std::endl;
    };

  for (std::size_t i = 0; i < n_threads; ++i) {
       if (i == 1) {
          threads.emplace_back([&]() {
              print("Unbound buffer", i);
              unbound_buffer();
        });
      }

      if (i == n_threads - 1) {
          threads.emplace_back([&]() {
              print(("Bound buffer to " + std::to_string(i)), i);
              set_bound_buffer(n_threads);
          });
      }

       if (i % 3 == 0) {
            threads.emplace_back([&]() {
                add_to_buffer(static_cast<int32_t>(i));
                print(("Item i added to buffer: " + std::to_string(i)), i);
            });
        }

        if (i % 6 == 0) {
            threads.emplace_back([&]() {
                const int32_t t =  remove_from_buffer().unwrap();
                print(("Item i added to buffer: " + std::to_string(t)), i);
            });
        }

      if (i % 4 == 0) {
          threads.emplace_back([&]() {
              print("Written value: " + std::to_string(i) + " to log", i);
              write_to_log(std::to_string(i));
          });
      }


        if (i % 5 == 0) {
            threads.emplace_back([&]() {
                const std::string t = read_from_log().unwrap();
                print(("Read from buffer returned: " + t), i);
            });
        }
    }

    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }
}

void run_deadlock_test_with_timeout(std::size_t n_threads, std::chrono::milliseconds timeout) {
    auto future = std::async(std::launch::async, intergration_test_for_only_deadlocks, n_threads);

    if (future.wait_for(timeout) == std::future_status::timeout) {
        report_test_result("Deadlock test: Test timed out", false);
        assert(false && "Test timed out!");
    } else {
        report_test_result("Deadlock test", true);
    }
}

int main(int argc, char* argv[]) {
  const std::size_t N_THREADS = 50;
  test_buffer_integrity(N_THREADS);
  test_log_integrity(N_THREADS);
  test_writing_and_removing_from_buffer(N_THREADS);

  test_reading_and_writing_to_logs(N_THREADS);
  run_write_to_logs_setup(N_THREADS);
  test_log_integrity(N_THREADS);
  test_empty_read_from_log_returns_err();
  run_deadlock_test_with_timeout(100, std::chrono::milliseconds(50000));
	return 0;
}