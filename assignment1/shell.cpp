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
  for (int i = 0; c_args[i] != nullptr; ++i) {
        std::cout << "c_args[0][" << i <<"]: " << c_args[0][i] << std::endl;
    }
  // replace current process with new process as specified 102 'f'  of 112 'p' 108 'l' 
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
// First, divide the input into the distinct commands (as they can be chained, separated by `|`).
// Next, these commands are parsed separately. The first command is checked for the `<` operator, and the last command for the `>` operator.
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
    //std::cout << args.at(0) << "\n";

  }
  //std::cout << expression.commands[0].parts.at(0) << "\n";
   std::cout << expression.inputFromFile + "input:" << "\n";
   std::cout << expression.outputToFile + "output:"<< "\n";
  return expression;
}

int execute_expression(Expression& expression) {
  // Check for empty expression
  if (expression.commands.size() == 0)
    return EINVAL;

  // Handle intern commands (like 'cd' and 'exit')
  if( expression.commands[0].parts.at(0) == std::string("exit")){
        std::cout << "exit code run" << "\n";
    exit(10);
  }

  if(expression.commands[0].parts.size() == 2 && expression.commands[0].parts.at(0) == std::string("cd")) {
    std::cout << "cd command" << "\n";   
    std::cout << expression.commands[0].parts.at(1) << "\n";   
    string s = expression.commands[0].parts.at(1);
    const char* path = s.c_str();
    std::cout << path << "\n";  
    chdir(path);
    return 0;
  }
  int waitall = 0;
  pid_t child_pid, wpid;
  // External commands, executed with fork():
  // Loop over all commandos, and connect the output and input of the forked processes
  
    // for(int i = 0; i < expression.commands.size(); i++){
    //     if((child_pid = fork()) == 0){
    //       execute_command(expression.commands[i]);
    //   }
    // }
    // while((wpid = wait(&waitall)) > 0); 
  // For now, we just execute the first command in the expression. Disable.
  // if(fork() == 0){
    execute_command(expression.commands[0]);
  // }
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
    //std::cout << expression.commands[0].parts.at(0) + "dasfjkasdl;fd"<< "\n";
    int rc = execute_expression(expression);
    if (rc != 0)
      cerr << strerror(rc) << endl;
  }
  return step1(showPrompt);
}
