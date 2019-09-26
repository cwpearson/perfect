#include <cassert>
#include <cerrno>
#include <iostream>
#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "clipp/clipp.h"

#include "perfect/aslr.hpp"
#include "perfect/cpu_set.hpp"
#include "perfect/cpu_turbo.hpp"
#include "perfect/os_perf.hpp"
#include "perfect/detail/os/linux.hpp"
#include "perfect/drop_caches.hpp"

// argv should be null-terminated
int fork_child(char *const *argv) {

  pid_t pid;
  int status;
  pid = fork();
  if (pid == -1) {
    // pid == -1 means error occured
    std::cerr << "can't fork, error occured\n";
    return EXIT_FAILURE;
  } else if (pid == 0) {
    // in the child process

    // skip the first argument, which is this program

    // the execv() only return if error occured.
    // The return value is -1
    return execvp(argv[0], argv);
  } else {
    // parent process
    if (waitpid(pid, &status, 0) > 0) {

      if (WIFEXITED(status) && !WEXITSTATUS(status)) {
        // success
        return status;
      }

      else if (WIFEXITED(status) && WEXITSTATUS(status)) {
        if (WEXITSTATUS(status) == 127) {
          std::cerr << "execv failed\n";
          return status;
        } else {
          std::cerr << "program terminated normally, but returned a non-zero "
                       "status\n";
          return status;
        }
      } else {
        printf("program didn't terminate normally\n");
        return status;
      }
    } else {
      printf("waitpid() failed\n");
      return EXIT_FAILURE;
    }
    return 0;
  }
}

int main(int argc, char **argv) {
  using namespace clipp;

  size_t numUnshielded = 0;
  size_t numShielded = 0;
  bool aslr = false;
  bool cpuTurbo = false;
  bool maxOsPerf = true;
  bool dropCaches = true;

  std::vector<std::string> program;

  auto shieldGroup =
      ((option("-u") &
        value("UNSH", numUnshielded).doc("number of unshielded CPUs")) |
       (option("-s") &
        value("SH", numShielded).doc("number of shielded CPUs")));


  auto cli = (shieldGroup, 
              option("--no-drop-cache").set(dropCaches, false).doc("do not drop filesystem caches"),
              option("--no-max-perf").set(maxOsPerf, false).doc("do not max os perf"),
              option("--no-aslr").set(aslr, false).doc("disable ASLR"),
              option("--cpu-turbo").set(cpuTurbo, true).doc("enable CPU turbo"),
              // run everything after "--"
              required("--") & greedy(values("cmd", program))

  );

  if (!parse(argc, argv, cli)) {
    std::cout << make_man_page(cli, argv[0]);
    return -1;
  }

  // exec the rest of the options
  std::vector<char *> args;
  for (auto &c : program) {
    args.push_back((char *)c.c_str());
  }
  args.push_back(nullptr);

  PERFECT(perfect::init());

  auto cpus = perfect::cpus();
  if (0 < numShielded) {
    numUnshielded = cpus.size() - numShielded;
  } else if (0 < numUnshielded) {
    numShielded = cpus.size() - numUnshielded;
  }

  // handle CPU shielding
  perfect::CpuSet shielded, unshielded;
  if (numShielded) {
    std::cerr << "shielding " << numShielded << " cpus\n";

    perfect::CpuSet root;
    PERFECT(perfect::CpuSet::get_root(root));
    PERFECT(root.make_child(shielded, "shielded"));
    PERFECT(root.make_child(unshielded, "unshielded"));

    std::cerr << "enable memory\n";
    PERFECT(shielded.enable_mem(0));
    PERFECT(shielded.enable_mem(0));

    std::cerr << "enable cpus\n";
    size_t i = 0;
    for (; i < numShielded; ++i) {
      std::cerr << "shield cpu " << cpus[i] << "\n";
      shielded.enable_cpu(cpus[i]);
    }
    for (; i < cpus.size(); ++i) {
      std::cerr << "unshield cpu " << cpus[i] << "\n";
      unshielded.enable_cpu(cpus[i]);
    }

    std::cerr << "migrate self\n";
    PERFECT(root.migrate_self_to(shielded));
    std::cerr << "migrate other\n";
    PERFECT(root.migrate_tasks_to(unshielded));
  }

  // handle aslr
  if (!aslr) {
    std::cerr << "disable ASLR for this process\n";
    PERFECT(perfect::disable_aslr());
  }

  // handle CPU turbo
  perfect::CpuTurboState cpuTurboState;
  if (!cpuTurbo) {
    std::cerr << "disabling cpu turbo\n";
    PERFECT(perfect::get_cpu_turbo_state(&cpuTurboState));
    PERFECT(perfect::disable_cpu_turbo());
  }

  // handle governor
  perfect::OsPerfState osPerfState;
  if (maxOsPerf) {
    std::cerr << "set max performance state\n";
    PERFECT(perfect::get_os_perf_state(osPerfState));
    for (auto cpu : perfect::cpus()) {
      PERFECT(perfect::os_perf_state_maximum(cpu));
    } 
  }

  // handle file system caches
  if (dropCaches) {
    std::cerr << "clearing file system cache\n";
    PERFECT(perfect::drop_caches());
  }

  // parent should return
  std::cerr << "exec ";
  for (size_t i = 0; i < args.size() - 1; ++i) {
    std::cerr << args[i] << " ";
  }
  std::cerr << "\n";
  int status = fork_child(args.data());
  std::cerr << "finished execution\n";

  // clean up CpuSets (if needed)
  if (numShielded) {
    std::cerr << "clean up cpu sets\n";
    shielded.destroy();
    unshielded.destroy();
  }

  // restore original turbo state
  if (!cpuTurbo) {
    std::cerr << "restore CPU turbo\n";
    PERFECT(perfect::set_cpu_turbo_state(cpuTurboState));
  }

  if (maxOsPerf) {
    std::cerr << "restore os performance state\n";
    PERFECT(perfect::set_os_perf_state(osPerfState));
  }

  return status;
}