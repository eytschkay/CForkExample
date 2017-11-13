#include "refs.h"

bool debugging = false;

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
int printChild(ChildProcess c){
    printf("Child Process <%d> \n", c.pid);
    printf("-> task: %s \n", c.task);
    printf("-> startTime: %d \n", c.startTime);
    printf("-> endTime: %d \n", c.endTime);
    printf("-> elipsed time: %d \n", c.endTime - c.startTime);
    printf("-> failed: %d \n\n", c.exitedWithError);
}

void printAllChildProcesses(ChildProcess c[]){

}

void sigint_handler(){
    printf("\nSIGNAL INTERRUPT\n");
    exit(11);
}

int main() {

    // varsmk
    Program program;
    string input;
    int pId;
    int i, j;
    int sum_of_usertime;

    //run signal handler for sigint
    signal(SIGINT, sigint_handler);

    //main routine
    while(true){

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


        printf("sum of user times = %d \n", sum_of_usertime);
    }

    return 0;
}
