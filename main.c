#include <error.h>
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
    printf("-> index: %d \n", (int) c.index);
    printf("-> user time: %d \n", (int) c.userTime);
    char* status;
    switch(c.exitStatus){
        case NORMAL:
            status = "NORMAL";
            break;
        case NOTCREATED:
            status = "NOTCREATED";
            break;
        case SIGNALLED:
            status = "SIGNALLED";
            break;
        default:
            status = "failed to obtain status";
    }
    printf("-> exitStatus: %s \n\n", status);
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
        main_loop_entry:

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
            //printf("Error %d\n", errno);
            exit(EXIT_SUCCESS);
        }

        input = str_replace(input, "\n", ";\n");
        input = removeDoubledEmptySpaces(input);
        input = str_replace(input, " ;", ";");


        //put input into peaces
        string part = strtok(input, ";");

        while(!strchr(part, '\n')){
            program.command[program.size].toString = part;
            part = strtok(NULL, ";");
            if (program.size < MAX_COMMAND) program.size++; else {
                printf("Max %d Commands allowed\n", MAX_COMMAND);
                goto main_loop_entry;
            }
        }

        //continue if input equals empty input
        if(program.size == 0) continue;

        for (i=0; i < program.size; i++){
            string tmp = strdup(program.command[i].toString);
            program.command[i].progName = strtok(tmp, " ");
            program.command[i].args[0] = program.command[i].progName;
            part = strtok(NULL, " ");
            j=1;
            while(part != NULL){
                program.command[i].args[j] = part;
                part = strtok(NULL, " ");
                if (j < MAX_ARGS) j++; else {
                    printf("Max %d args allowed\n", MAX_ARGS-1);
                    goto main_loop_entry;
                }
            }
        }

        ChildProcess childProcess[program.size];

        for (i=0; i < program.size; i++){
            pid_t cpid, w;
            int status;

            cpid = fork();
            if (cpid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (cpid == 0) {            /* Code executed by child */
                execvp(program.command[i].progName, program.command[i].args);
                _exit(EXIT_FAILURE);

            } else {                    /* Code executed by parent */
                do {
                    struct tms begin, end;

                    times(&begin);
                    w = waitpid(cpid, &status, WUNTRACED | WCONTINUED);
                    times(&end);

                    if (w == -1) {
                        perror("waitpid");
                        exit(EXIT_FAILURE);
                    }

                    if (WIFEXITED(status)) {
                        if (WEXITSTATUS(status) != 0) childProcess[i].exitStatus = EXECVPNOTZERO;
                        else {
                            childProcess[i].exitStatus = NORMAL;
                            childProcess[i].userTime = (int) (end.tms_cutime - begin.tms_cutime);
                        }
                    } else if (WIFSIGNALED(status)) {
                        childProcess[i].exitStatus = SIGNALLED;
                    }
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            }
        }

        //printout
        for(i=0; i<program.size; i++){
            string task = program.command[i].args[0];
            switch(childProcess[i].exitStatus){
                //case EXECVPNOTZERO:  //uncomment to allow non shell commands - will hide errors
                case NORMAL:
                    printf("%s: user time = %d\n", task, childProcess[i].userTime);
                    sum_of_usertime += childProcess[i].userTime;
                    break;
                default:
                    printf("%s: [execution error]\n", task);
            }
        }

       //print sum of usertimes
       printf("sum of user times = %d \n", sum_of_usertime);
    }

    //code after ctrl+C
    //....

    return 0;
}
