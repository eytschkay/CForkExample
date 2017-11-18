#include "refs.h"

//debug mode
bool debugging = false;

//global Vars
bool isInterrupted = false;

//input management
string removeDoubledEmptySpaces(string str){
    char *from, *to;
    int spc=0;
    to=from=str;
    while(1){
        if(spc && *from == ' ' && to[-1] == ' ')
            ++from;
        else {
            spc = (*from==' ')? 1 : 0;
            *to++ = *from++;
            if(!to[-1])break;
        }
    }
    return str;
}
string *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; tmp = strstr(ins, rep); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}


//ChildProcess Functions
void printChild(ChildProcess c){
    printf("Child Process <%d> \n", c.pid);
    printf("-> task: %s \n", c.task);
    printf("-> startTime: %d \n", (int) c.startTime);
    printf("-> endTime: %d \n", (int) c.endTime);
    printf("-> elipsed time: %d \n", (int) (c.endTime - c.startTime));
    printf("-> failed: %d \n\n", (int) c.exitedWithError);
}

//handler
void sigint_handler(){
    isInterrupted = true;
}

int main() {

    // vars
    Program program;
    int i, j;
    int sum_of_usertime;

    //run signal handler for sigint
    signal(SIGINT, sigint_handler);

    //main routine
    while(!isInterrupted){

        printf("> ");
        sum_of_usertime = 0;

        char* input;
        input = malloc(500 * sizeof(char));


        // init/reset program
        program.size = 0;
        for(i=0; i < MAX_COMMAND; i++){
            program.command[i].progName = NULL;
            program.command[i].toString = NULL;
            for (j = 0; j < MAX_ARGS; j++){
                program.command[i].args[j] = NULL;
            }
        }

        //get Input
        if (fgets(input, 500, stdin) == NULL) {
            printf("Error %d\n", errno);
            exit(EXIT_FAILURE);
        }

        input = str_replace(input, "\n", ";\n");
        input = removeDoubledEmptySpaces(input);
        input = str_replace(input, " ;", ";");


        //put input into peaces
        string part = strtok(input, ";");

        while(!strchr(part, '\n')){
            program.command[program.size].toString = part;
            part = strtok(NULL, ";");
            if (program.size < MAX_COMMAND) program.size++; else perror("IndexOutOfBound: too many commands");
        }
        for (i=0; i < program.size; i++){
            string tmp = strdup(program.command[i].toString);
            program.command[i].progName = strtok(tmp, " ");
            program.command[i].args[0] = program.command[i].progName;
            part = strtok(NULL, " ");
            j=1;
            while(part != NULL){
                program.command[i].args[j] = part;
                part = strtok(NULL, " ");
                if (j < MAX_COMMAND) j++; else perror("IndexOutOfBound: too many arguments");
            }
        }

        ChildProcess childProcess[program.size];

        //fork
        for (i = 0; i < program.size; i++) {
            if ((childProcess[i].pid=fork()) == 0) {
                execvp(program.command[i].progName, program.command[i].args);
                _exit(errno); //only reached if execvp fails
            }
            if (childProcess[i].pid < 0) perror("Fork error");
        }

        //wait for execution
        for (i = 0; i < program.size; i++) {
            if (childProcess[i].pid > 0) {
                int status;
                childProcess[i].task = program.command[i].progName;

                //measure time
                childProcess[i].startTime = times(&childProcess[i].init_tms);
                wait(&status);
                //waitpid(childProcess[i].pid, &status, 0);
                childProcess[i].endTime = times(&childProcess[i].end_tms);

                if (status > 0) childProcess[i].exitedWithError = true; //error in childProcess
            }
            else {
                childProcess[i].exitedWithError = true; //child did not start, error
            }
        }

        for(i=0; i<program.size; i++){
            if (childProcess[i].exitedWithError) {
                printf("%s: [execution error]\n", childProcess[i].task);
                childProcess[i].userTime = -1;
                continue;
            }
            childProcess[i].userTime = childProcess[i].endTime - childProcess[i].startTime;
            printf("%s: user time = %d\n", childProcess[i].task, childProcess[i].userTime);
            sum_of_usertime += childProcess[i].userTime;
        }

        //print sum of usertimes
        if (program.size != 0) printf("sum of user times = %d \n", sum_of_usertime);
    }

    //code after ctrl+C
    //....

    return 0;
}
