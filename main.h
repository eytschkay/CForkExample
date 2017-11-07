
#define string char*

#define MAX_COMMAND 10
#define MAX_ARGS 20

struct Command{
    string toString;
    string progName;
    string args[MAX_ARGS];
};

struct Program{
    struct Command command[MAX_COMMAND];
    int size;
};

struct ChildProcess{
    clock_t startTime;
    clock_t endTime;
    pid_t pid;
    int status;
    bool exitedWithError;
};

string readInput(void);