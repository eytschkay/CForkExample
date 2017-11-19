#include <sys/times.h>

#define string char*

#define MAX_INPUT_LENGTH 500    //chars/input line
#define MAX_COMMAND 10          //commands/input line
#define MAX_ARGS 20             //args/input line (arg[0] = progName, arg[1..MAX_ARGS-1])

//exit codes for child processes
#define NORMAL 100
#define FORK_ERR 1
#define EXECVP_ERR 2
#define INTERRUPTED 3
#define UNKNOWN -1;

struct Command{
    string toString;
    string progName;
    string args[MAX_ARGS];
    pid_t pid;
    int userTime;
    int status;
    bool hasExited;
    int exitStatus;
    struct tms begin, end;
};

#define Program struct Prog
struct Prog{
    struct Command command[MAX_COMMAND];
    int size;
};