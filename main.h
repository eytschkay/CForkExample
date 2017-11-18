#include <sys/times.h>

#define debug(task) if (debugging) task

#define string char*

#define MAX_COMMAND 10
#define MAX_ARGS 21

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
#define NORMAL 0
#define SIGNALLED 1
#define NOTCREATED 2
#define EXECVPNOTZERO 3

struct CProcess{
    pid_t pid;
    int userTime;
    int exitStatus;
    int index;
    bool wasEntered;
};

string readInput(void);