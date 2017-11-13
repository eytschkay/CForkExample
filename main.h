#include <sys/times.h>

#define debug(task) if (debugging) task

#define string char*

#define MAX_COMMAND 10
#define MAX_ARGS 20

#define foreach(item, array) \
    for(int keep = 1, \
            count = 0,\
            size = sizeof (array) / sizeof *(array); \
        keep && count != size; \
        keep = !keep, count++) \
      for(item = (array) + count; keep; keep = !keep)

struct Command{
    string toString;
    string progName;
    string args[MAX_ARGS];
};

#define Program struct Prog
struct Prog{
    struct Command command[MAX_COMMAND];
    int size;
};

#define ChildProcess struct CProcess
struct CProcess{
    clock_t startTime;
    clock_t endTime;
    pid_t pid;
    int userTime;
    bool exitedWithError;
    string task;
    struct tms init_tms;
    struct tms end_tms;
};

string readInput(void);