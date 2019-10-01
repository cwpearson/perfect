#include <cassert>
#include <cerrno>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#ifdef __linux__
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#else
#error "unsupported platform"
#endif

#include "clipp/clipp.h"
#include "nonstd/optional.hpp"

#include "perfect/aslr.hpp"
#include "perfect/cpu_set.hpp"
#include "perfect/cpu_turbo.hpp"
#include "perfect/detail/os/linux.hpp"
#include "perfect/drop_caches.hpp"
#include "perfect/os_perf.hpp"
#include "perfect/priority.hpp"

typedef std::function<perfect::Result()> CleanupFn;
std::vector<CleanupFn> cleanups;

// restore the system state to how we found it
void cleanup(int dummy) {
  (void)dummy;
  std::cerr << "caught ctrl-c\n";

  // unregister our handler
  signal(SIGINT, SIG_DFL);
  std::cerr << "cleaning up\n";
  std::cerr << "ctrl-c again to quit\n";

  for (auto f : cleanups) {
    perfect::Result result = f();
  }

  exit(EXIT_FAILURE);
}

// argv should be null-terminated
// outf and errf are file descriptors to where stdout and stderr should be
// redirected write stdout to out and stderr to err, if not null
int fork_child(char *const *argv, int outf, int errf) {

  pid_t pid;
  int status;
  pid = fork();
  if (pid == -1) {
    // pid == -1 means error occured
    std::cerr << "can't fork, error occured\n";
    return EXIT_FAILURE;
  } else if (pid == 0) {
    // in the child process

    if (outf > 0) {
      std::cerr << "redirecting child stdout to file\n";
      if (dup2(outf, 1)) {
        std::cerr << "dup2 error: " << strerror(errno) << "\n";
        /*

    EBADF
        oldfd isn't an open file descriptor, or newfd is out of the allowed
    range for file descriptors. EBUSY (Linux only) This may be returned by
    dup2() or dup3() during a race condition with open(2) and dup(). EINTR The
    dup2() or dup3() call was interrupted by a signal; see signal(7). EINVAL
        (dup3()) flags contain an invalid value. Or, oldfd was equal to newfd.
    EMFILE
        The process already has the maximum number of file descriptors open and
    tried to open a new one.
        */
      }

      if (close(outf)) {
        /*
        EBADF
            The fildes argument is not a valid file descriptor.
        EINTR
            The close() function was interrupted by a signal.

        The close() function may fail if:

        EIO
            An I/O error occurred while reading from or writing to the file
        system.
          */
      }
    }

    if (errf > 0) {
      std::cerr << "redirecting child stderr to file\n";
      if (dup2(errf, 2)) {
        std::cerr << "dup2 error: " << strerror(errno) << "\n";

        /*

    EBADF
        oldfd isn't an open file descriptor, or newfd is out of the allowed
    range for file descriptors. EBUSY (Linux only) This may be returned by
    dup2() or dup3() during a race condition with open(2) and dup(). EINTR The
    dup2() or dup3() call was interrupted by a signal; see signal(7). EINVAL
        (dup3()) flags contain an invalid value. Or, oldfd was equal to newfd.
    EMFILE
        The process already has the maximum number of file descriptors open and
    tried to open a new one.
        */
      }

      if (close(errf)) {
        /*
    EBADF
        The fildes argument is not a valid file descriptor.
    EINTR
        The close() function was interrupted by a signal.

    The close() function may fail if:

    EIO
        An I/O error occurred while reading from or writing to the file system.
      */
      }
    }

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

  signal(SIGINT, cleanup);

  using namespace clipp;

  size_t numUnshielded = 0;
  size_t numShielded = 0;
  bool aslr = false;
  nonstd::optional<bool> cpuTurbo = false;
  nonstd::optional<bool> maxOsPerf = true;
  bool dropCaches = true;
  bool highPriority = true;

  std::vector<std::string> program;
  std::string stdoutPath;
  std::string stderrPath;
  int iters = 1;
  int sleepMs = 1000;

  bool help = false;

  auto helpMode = option("-h", "--help").set(help).doc("show help");

  auto shieldGroup = ((option("-u").doc("number of unshielded CPUs") &
                       value("INT", numUnshielded)) |
                      (option("-s").doc("number of shielded CPUs") &
                       value("INT", numShielded)));

  auto noModMode = (option("--no-mod")
                        .doc("don't control performance")
                        .set(aslr, true)
                        .call([&]() { cpuTurbo = nonstd::nullopt; })
                        .call([&]() { maxOsPerf = nonstd::nullopt; })
                        .set(dropCaches, false)
                        .set(highPriority, false));

  auto modMode = (shieldGroup,
                  option("--no-drop-cache")
                      .set(dropCaches, false)
                      .doc("do not drop filesystem caches"),
                  option("--no-max-perf").doc("do not max os perf").call([&]() {
                    maxOsPerf = false;
                  }),
                  option("--aslr").set(aslr, true).doc("enable ASLR"),
                  option("--no-priority")
                      .set(highPriority, false)
                      .doc("don't set high priority"),
                  option("--cpu-turbo").doc("enable CPU turbo").call([&]() {
                    cpuTurbo = true;
                  }),
                  (option("--stdout").doc("redirect child stdout") &
                   value("PATH", stdoutPath)),
                  (option("--stderr").doc("redirect child stderr") &
                   value("PATH", stderrPath)));

  auto cli =
      helpMode |
      ((noModMode | modMode),
       (option("--sleep-ms").doc("sleep before run") & value("INT", sleepMs)),
       (option("-n").doc("run multiple times") & value("INT", iters)), helpMode,
       // run everything after "--"
       required("--") & greedy(values("cmd", program))

      );

  if (!parse(argc, argv, cli)) {
    auto fmt = doc_formatting{}.doc_column(31);
    std::cout << make_man_page(cli, argv[0], fmt);
    return -1;
  }

  if (help) {
    auto fmt = doc_formatting{}.doc_column(31);
    std::cout << make_man_page(cli, argv[0], fmt);
    return 0;
  }

  // open the redirect files, if needed
  int errf = 0;
  int outf = 0;
  if (!stderrPath.empty()) {
    std::cerr << "open " << stderrPath << "\n";
    errf = open(stderrPath.c_str(), O_WRONLY | O_CREAT,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (-1 == errf) {
      std::cerr << "error while opening " << stderrPath << ": "
                << strerror(errno) << "\n";
    }
  }
  if (!stdoutPath.empty()) {
    outf = open(stdoutPath.c_str(), O_WRONLY | O_CREAT,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (-1 == outf) {
      std::cerr << "error while opening " << stdoutPath << ": "
                << strerror(errno) << "\n";
    }
  }

  // if called with sudo, chown the files to whoever called sudo
  const char *sudoUser = std::getenv("SUDO_USER");
  if (sudoUser) {
    std::cerr << "called with sudo by " << sudoUser << "\n";
    uid_t uid;
    gid_t gid;
    struct passwd *pwd;

    pwd = getpwnam(sudoUser);
    if (pwd == NULL) {
      // die("Failed to get uid");
    }
    uid = pwd->pw_uid;
    gid = pwd->pw_gid;

    if (!stdoutPath.empty()) {
      if (chown(stdoutPath.c_str(), uid, gid) == -1) {
        // die("chown fail");
      }
    }
    if (!stderrPath.empty()) {
      if (chown(stderrPath.c_str(), uid, gid) == -1) {
        // die("chown fail");
      }
    }
  }

  // build the program arguments
  std::vector<char *> args;
  for (auto &c : program) {
    args.push_back((char *)c.c_str());
  }
  args.push_back(nullptr);

  // init the perfect library
  PERFECT(perfect::init());

  auto cpus = perfect::cpus();
  if (0 < numShielded) {
    numUnshielded = cpus.size() - numShielded;
  } else if (0 < numUnshielded) {
    numShielded = cpus.size() - numUnshielded;
  }

  // handle CPU shielding
  perfect::CpuSet root, shielded, unshielded;
  if (numShielded) {
    std::cerr << "shielding " << numShielded << " cpus\n";

    PERFECT(perfect::CpuSet::get_root(root));
    PERFECT(root.make_child(shielded, "shielded"));
    PERFECT(root.make_child(unshielded, "unshielded"));

    std::cerr << "enable memory\n";
    PERFECT(shielded.enable_mem(0));
    PERFECT(unshielded.enable_mem(0));

    std::cerr << "enable cpus\n";
    size_t i = 0;
    for (; i < cpus.size() - numShielded; ++i) {
      std::cerr << "unshield cpu " << cpus[i] << "\n";
      unshielded.enable_cpu(cpus[i]);
    }
    for (; i < cpus.size(); ++i) {
      std::cerr << "shield cpu " << cpus[i] << "\n";
      shielded.enable_cpu(cpus[i]);
    }

    std::cerr << "migrate self\n";
    PERFECT(root.migrate_self_to(shielded));
    std::cerr << "migrate other (1/2)\n";
    PERFECT(root.migrate_tasks_to(unshielded));
    // some tasks may have been spawned by unmigrated tasks while we migrated
    std::cerr << "migrate other (2/2)\n";
    PERFECT(root.migrate_tasks_to(unshielded));

    cleanups.push_back(CleanupFn([&] {
      std::cerr << "cleanup: shielded cpu set\n";
      shielded.destroy();
      std::cerr << "cleanup: unshielded cpu set\n";
      unshielded.destroy();
      return perfect::Result::SUCCESS;
    }));
  }

  // handle aslr
  if (!aslr) {
    std::cerr << "disable ASLR for this process\n";
    PERFECT(perfect::disable_aslr());
  }

  // handle CPU turbo
  perfect::CpuTurboState cpuTurboState;
  if (cpuTurbo.has_value()) {
    PERFECT(perfect::get_cpu_turbo_state(&cpuTurboState));
    if (false == cpuTurbo) {
      std::cerr << "disabling cpu turbo\n";
      PERFECT(perfect::disable_cpu_turbo());
    } else {
      std::cerr << "enabling cpu turbo\n";
      PERFECT(perfect::enable_cpu_turbo());
    }

    cleanups.push_back(CleanupFn([&] {
      std::cerr << "cleanup: restore CPU turbo state\n";
      return perfect::set_cpu_turbo_state(cpuTurboState);
    }));
  }

  // handle governor
  perfect::OsPerfState osPerfState;
  if (maxOsPerf.has_value()) {
    PERFECT(perfect::get_os_perf_state(osPerfState));
    if (true == maxOsPerf) {
      std::cerr << "set max performance state\n";
      for (auto cpu : perfect::cpus()) {
        PERFECT(perfect::os_perf_state_maximum(cpu));
      }
    }

    cleanups.push_back(CleanupFn([&] {
      std::cerr << "cleanup: os governor\n";
      return perfect::set_os_perf_state(osPerfState);
    }));
  }

  if (highPriority) {
    std::cerr << "set high priority\n";
    PERFECT(perfect::set_high_priority());
  }

  // parent should return
  for (int runIter = 0; runIter < iters; ++runIter) {

    // drop filesystem caches before each run
    if (dropCaches) {
      std::cerr << "clearing file system cache\n";
      PERFECT(perfect::drop_caches());
    }

    // sleep before each run
    if (sleepMs) {
      std::cerr << "sleep " << sleepMs << " ms before run\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
    }

    std::cerr << "exec ";
    for (size_t i = 0; i < args.size() - 1; ++i) {
      std::cerr << args[i] << " ";
    }
    std::cerr << "\n";

    int status = fork_child(args.data(), outf, errf);
    if (0 != status) {
      std::cerr << "did not terminate successfully\n";
    }
    std::cerr << "finished execution\n";
  }

  // clean up CpuSets (if needed)
  if (numShielded) {
    std::cerr << "clean up cpu sets\n";
    shielded.destroy();
    unshielded.destroy();
  }

  // restore original turbo state
  if (cpuTurbo.has_value()) {
    std::cerr << "restore CPU turbo\n";
    PERFECT(perfect::set_cpu_turbo_state(cpuTurboState));
  }

  if (maxOsPerf.has_value()) {
    std::cerr << "restore os performance state\n";
    PERFECT(perfect::set_os_perf_state(osPerfState));
  }

  return 0;
}