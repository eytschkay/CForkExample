#include "refs.h"

//debug mode
bool debugging = true;

//global Vars
bool isInterrupted = false;

//input management
string readInput(void) {
    char * line = malloc(100), * linep = line;
    size_t lenmax = 100, len = lenmax;
    int c;

    if(line == NULL)
        return NULL;

    for(;;) {
        c = fgetc(stdin);
        if(c == EOF)
            break;

        if(--len == 0) {
            len = lenmax;
            char * linen = realloc(linep, lenmax *= 2);

            if(linen == NULL) {
                free(linep);
                return NULL;
            }
            line = linen + (line - linep);
            linep = linen;
        }

        if((*line++ = c) == '\n')
            break;
    }
    *line = '\0';
    return linep;
}
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
string trimwhitespace(string str){
    char *end;

    // Trim leading space
    while(isspace((unsigned char)*str)) str++;

    if(*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    *(end+1) = 0;

    return str;
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
    string input;
    int i, j;
    int sum_of_usertime;

    //run signal handler for sigint
    signal(SIGINT, sigint_handler);

    //main routine
    while(!isInterrupted){

        printf("> ");
        sum_of_usertime = 0;


        // init/reset program
        program.size = 0;
        for(i=0; i < MAX_COMMAND; i++){
            program.command[i].progName = NULL;
            program.command[i].toString = NULL;
            for (j = 0; j < MAX_ARGS; j++){
                program.command[i].args[j] = NULL;
            }
        }

        //read input
        input = readInput();
        if (strlen(input) > 500) perror("IndexOutOfBound: input too long");
        if (input == NULL) exit(0); //todo

        input = removeDoubledEmptySpaces(input);
        //todo: remove leading whitespace before semic

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
                waitpid(childProcess[i].pid, &status, 0);
                childProcess[i].endTime = times(&childProcess[i].end_tms);

                if (status > 0) childProcess[i].exitedWithError = true; //error in childProcess
            }
            else {
                childProcess[i].exitedWithError = true; //child did not start, error
            }
        }


        debug(foreach(ChildProcess *c, childProcess) printChild(*c);)

        //print user times
        foreach (ChildProcess *c, childProcess) {
            if (c->exitedWithError) {
                printf("%s: [execution error]\n", c->task);
                c->userTime = -1;
                continue;
            }
            c->userTime = c->endTime - c->startTime;
            printf("%s: user time = %d\n", c->task, c->userTime);
            sum_of_usertime += c->userTime;
        }

        //print sum of usertimes
        if (sum_of_usertime == 0) printf("Nothing to be done\n"); else printf("sum of user times = %d \n", sum_of_usertime);
    }

    //code after ctrl+C
    //....

    return 0;
}
