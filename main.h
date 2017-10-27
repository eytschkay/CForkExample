
#define string char*

#define MAX_COMMAND 10
#define MAX_ARGS 20

struct Command{
    string toString;
    string progName;
    string args[MAX_ARGS];
}command;

struct Program{
    struct Command command[MAX_COMMAND];
    int size;
}program;


string readInput(void);