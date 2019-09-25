#include <iostream>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <vector>

#include "perfect/aslr.hpp"

using namespace perfect;

int main(int argc, char **argv) {

  using namespace perfect;

  PERFECT(init());

  pid_t pid;
  int status;
  pid = fork();
  if (pid == -1) {
    // pid == -1 means error occured
    std::cerr << "can't fork, error occured\n";
    exit(EXIT_FAILURE);
  } else if (pid == 0) {
    // in the child process

    // skip the first argument, which is this program
    std::vector<char*> args;
    for (int i = 1; i < argc; ++i) {
      args.push_back(argv[i]);
    }
    assert(args.size() > 0);
    args.push_back(nullptr);

    PERFECT(disable_aslr());

    // the execv() only return if error occured.
    // The return value is -1
    return execvp(args[0], args.data());
  } else {
    // parent process

    if (waitpid(pid, &status, 0) > 0) {

      if (WIFEXITED(status) && !WEXITSTATUS(status)) {
        // success
        exit(status);
      }

      else if (WIFEXITED(status) && WEXITSTATUS(status)) {
        if (WEXITSTATUS(status) == 127) {

          // execv failed
          std::cerr << "execv failed\n";
          exit(status);
        } else {
          std::cerr << "program terminated normally, but returned a non-zero status\n";
          exit(status);
        }
      } else {
        printf("program didn't terminate normally\n");
        exit(status);
      }
    } else {
      // waitpid() failed
      printf("waitpid() failed\n");
      exit(EXIT_FAILURE);
    }
    exit(0);
  }
  return 0;
}
