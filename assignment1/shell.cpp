/**
	* Shell framework
	* course Operating Systems
	* Radboud University
	* v22.09.05

	Student names:
	- ...
	- ...
*/

/**
 * Hint: in most IDEs (Visual Studio Code, Qt Creator, neovim) you can:
 * - Control-click on a function name to go to the definition
 * - Ctrl-space to auto complete functions and variables
 */

// function/class definitions you are going to use
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/param.h>
#include <signal.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <vector>
#include <list>
#include <optional>

//Custom
#include <map>
#include <cerrno>
#include <functional>
#include <unordered_map>
#include <memory>

// although it is good habit, you don't have to type 'std' before many objects by including this line
using namespace std;

struct Command {
  vector<string> parts = {};
};

struct Expression {
  vector<Command> commands;
  string inputFromFile;
  string outputToFile;
  bool background = false;
};

//Enums and mappers
//Enums 
enum InternalCommands { 
   NORMAL_EXIT_COMMAND,
   FUNC_EXIT_COMMAND,
   CD_COMMAND 
};

//Make it lazily loaded if time is available
std::map<InternalCommands, std::string> internal_command_to_string = { 
  {NORMAL_EXIT_COMMAND, std::string("exit")},
  {FUNC_EXIT_COMMAND, std::string("exit()")},
  {CD_COMMAND, std::string("cd")}
 };

std::vector<std::pair<std::function<bool(const std::string&)>, 
                           std::function<void(const Command&)>>> internal_commands = {
        {
            // Predicate for "exit" command
            [](const std::string& s) { return (s == internal_command_to_string[FUNC_EXIT_COMMAND] || s == internal_command_to_string[NORMAL_EXIT_COMMAND]); },
            [](const Command& cmd) {
                std::cout << "Goodbye! Exiting..." << std::endl;
                exit(0);
            }
        },
        {
            // Predicate for "cd" command
            [](const std::string& s) { return s == internal_command_to_string[CD_COMMAND]; },
            [](const Command& cmd) {
            if (cmd.parts.size() == 1) {
                std::cout << "No path argument provided" << std::endl;
                return;
            }

            if (cmd.parts.size() != 2) {
                std::cout << "Invalid amount of arguments provided" << std::endl;
                return;
            }

            int err_code = chdir(cmd.parts.at(1).c_str());
            if (err_code != 0) {
                std::perror("chdir()");
                return;
            }

            std::cout << "Directory changed to " << cmd.parts.at(1) << std::endl;
            }
        }
    };

// Parses a string to form a vector of arguments. The separator is a space char (' ').
vector<string> split_string(const string& str, char delimiter = ' ') {
  vector<string> retval;
  for (size_t pos = 0; pos < str.length(); ) {
    // look for the next space
    size_t found = str.find(delimiter, pos);
    // if no space was found, this is the last word
    if (found == string::npos) {
      retval.push_back(str.substr(pos));
      break;
    }
    // filter out consequetive spaces
    if (found != pos)
      retval.push_back(str.substr(pos, found-pos));
    pos = found+1;
  }
  return retval;
}

// wrapper around the C execvp so it can be called with C++ strings (easier to work with)
// always start with the command itself
// DO NOT CHANGE THIS FUNCTION UNDER ANY CIRCUMSTANCE
int execvp(const vector<string>& args) {
  // build argument list
  const char** c_args = new const char*[args.size()+1];
  for (size_t i = 0; i < args.size(); ++i) {
    c_args[i] = args[i].c_str();
  }
  c_args[args.size()] = nullptr;
  // replace current process with new process as specified
  int rc = ::execvp(c_args[0], const_cast<char**>(c_args));
  // if we got this far, there must be an error
  int error = errno;
  // in case of failure, clean up memory (this won't overwrite errno normally, but let's be sure)
  delete[] c_args;
  errno = error;
  return rc;
}

// Executes a command with arguments. In case of failure, returns error code.
int execute_command(const Command& cmd) {
  auto& parts = cmd.parts;
  if (parts.size() == 0)
    return EINVAL;

  // execute external commands
  int retval = execvp(parts);
  return retval ? errno : 0;
}

void display_prompt() {
  char buffer[512];
  char* dir = getcwd(buffer, sizeof(buffer));
  if (dir) {
    cout << "\e[32m" << dir << "\e[39m"; // the strings starting with '\e' are escape codes, that the terminal application interpets in this case as "set color to green"/"set color to default"
  }
  cout << "$ ";
  flush(cout);
}

string request_command_line(bool showPrompt) {
  if (showPrompt) {
    display_prompt();
  }
  string retval;
  getline(cin, retval);
  return retval;
}

// note: For such a simple shell, there is little need for a full-blown parser (as in an LL or LR capable parser).
// Here, the user input can be parsed using the following approach.
// First, divide the input into the distinct commands (as they can be chained, separated by |).
// Next, these commands are parsed separately. The first command is checked for the < operator, and the last command for the > operator.
Expression parse_command_line(string commandLine) {
  Expression expression;
  vector<string> commands = split_string(commandLine, '|');
  for (size_t i = 0; i < commands.size(); ++i) {
    string& line = commands[i];
    vector<string> args = split_string(line, ' ');
    if (i == commands.size() - 1 && args.size() > 1 && args[args.size()-1] == "&") {
      expression.background = true;
      args.resize(args.size()-1);
    }
    if (i == commands.size() - 1 && args.size() > 2 && args[args.size()-2] == ">") {
      expression.outputToFile = args[args.size()-1];
      args.resize(args.size()-2);
    }
    if (i == 0 && args.size() > 2 && args[args.size()-2] == "<") {
      expression.inputFromFile = args[args.size()-1];
      args.resize(args.size()-2);
    }
    expression.commands.push_back({args});
  }
  return expression;
}

void handle_internal_command(const std::vector<Command>& command) {
  for(const Command& command : command) {
    const std::string s_command = command.parts[0];
    for(auto& [predicate, action] : internal_commands) {
      if(predicate(s_command))
        action(command);
    }
  }
}

int read_file(const std::string& file_path) {
  auto file_exists = [&]() {return (access(file_path.c_str(), F_OK) != 1);};

  if(!file_exists()) {
    return -1;
  }

  return open(file_path.c_str(), O_RDONLY);
}

int open_file(std::string& filepath) {
  return open(filepath.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
}

int close_file_descriptor(int* fd, optional<bool> predicate) {
  if(!predicate.has_value() && fd && *fd >= 0) {
    return close(*fd);
  }
  if(predicate.has_value() && predicate.value() && fd && *fd >= 0) {
    return close(*fd);
  } 
  return 0;
}

int execute_expression(Expression& expression) {
  // Check for empty expression
  if (expression.commands.size() == 0)
    return EINVAL;

  // Handle intern commands (like 'cd' and 'exit')
  handle_internal_command(expression.commands);
  
  // External commands, executed with fork():
  // Loop over all commandos, and connect the output and input of the forked processes
  int pipefds[2];
  pid_t pid;
  int prev_pipefd = -1;
  std::vector<std::reference_wrapper<const pid_t>> child_pids;

  const bool write_to_file = !expression.outputToFile.empty();
  const bool read_from_file = !expression.inputFromFile.empty();

  int output_fd;
  if(write_to_file && (output_fd = open_file(expression.outputToFile)) == -1) {
    std::perror("acces");
  };

  int input_fd;
  if(read_from_file && (input_fd = read_file(expression.inputFromFile)) == -1) {
    std::perror("acces");
  }

  auto is_last_element = [&](size_t& i) { return i >= (expression.commands.size() - 1);};
  auto is_first_command = [](size_t& i) { return i == 0 ;};

   for (size_t i = 0; i < expression.commands.size(); ++i) {
        const Command& command = expression.commands[i];

        if (!is_last_element(i)) {
            if (pipe(pipefds) == -1) {
                std::perror("pipe");
                return -1;
            }
        }

        //Fork parent
        if ((pid = fork()) == -1) {
            std::perror("fork");
            return -1;
        } else if (pid == 0) { //Child process
            if (!is_first_command(i)) {
                dup2(prev_pipefd, STDIN_FILENO); //Read the output from the previous command and use it as input
                close(prev_pipefd);
            } else if (is_first_command(i) && read_from_file) {
                dup2(input_fd, STDIN_FILENO); //From file to new input
                close(input_fd);
            }

            if (!is_last_element(i)) { //Not the last
                close(pipefds[0]); 
                dup2(pipefds[1], STDOUT_FILENO); //Write output to the current pipe 
                close(pipefds[1]);
            } else if (write_to_file) {
                dup2(output_fd, STDOUT_FILENO); //We are the last element and write to file is true
                close(output_fd);
            }

            /*if(expression.background)
                sleep(2); */
            
            execute_command(command); //Uses STDIN as input
            abort();
        } else { //Parent
            child_pids.push_back(std::ref(pid));

            if (!is_first_command(i)) {
                close(prev_pipefd);
            }

            if (!is_last_element(i)) { 
                close(pipefds[1]);
                prev_pipefd = pipefds[0]; //Read the previous command its output and put it in prev_pipefd setting it up for the next iteration
            }
        }
    }

  close_file_descriptor(&output_fd, write_to_file); //Close the output file descriptor
  close_file_descriptor(&input_fd, read_from_file); //Close the read descriptor
          
  if(expression.background) {
    for(auto& pid : child_pids) {
      waitpid(pid, nullptr, WNOHANG);  // Dont wait  
    }
  } else {
     for(auto& pid : child_pids) {
      waitpid(pid, nullptr, 0);  // Wait for each specific child process    
    }
  }

  return 0;
}

// framework for executing "date | tail -c 5" using raw commands
// two processes are created, and connected to each other
int step1(bool showPrompt) {
  // create communication channel shared between the two processes
  // ...

  pid_t child1 = fork();
  if (child1 == 0) {
    // redirect standard output (STDOUT_FILENO) to the input of the shared communication channel
    // free non used resources (why?)
    Command cmd = {{string("date")}};
    execute_command(cmd);
    // display nice warning that the executable could not be found
    abort(); // if the executable is not found, we should abort. (why?)
  }

  pid_t child2 = fork();
  if (child2 == 0) {
    // redirect the output of the shared communication channel to the standard input (STDIN_FILENO).
    // free non used resources (why?)
    Command cmd = {{string("tail"), string("-c"), string("5")}};
    execute_command(cmd);
    abort(); // if the executable is not found, we should abort. (why?)
  }

  // free non used resources (why?)
  // wait on child processes to finish (why both?)
  waitpid(child1, nullptr, 0);
  waitpid(child2, nullptr, 0);
  return 0;
}

int shell(bool showPrompt) {
  //* <- remove one '/' in front of the other '/' to switch from the normal code to step1 code
  while (cin.good()) {
    string commandLine = request_command_line(showPrompt);
    Expression expression = parse_command_line(commandLine);
    int rc = execute_expression(expression);
    if (rc != 0)
      cerr << strerror(rc) << endl;
  }
  return 0;
  /*/
  return step1(showPrompt);
  //*/
}