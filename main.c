#include <unistd.h>
#include <sys/wait.h>
#include <time.h>
#include "refs.h"


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

int getChildStatus (int status) {
    if (WIFEXITED (status)) {
        //printf ("Kind normal beendet mit Rückgabewert %d\n", WEXITSTATUS(status));
        return 0;
    }
    else if (WIFSIGNALED (status)) {
        /* printf ("Kind mit Signalnummer %d beendet\n", WTERMSIG(status));
        /* WCOREDUMP könnte hier stehen */
        return WTERMSIG(status);
    }
    else if (WIFSTOPPED (status)) {
        /*printf ("Kind wurde angehalten mit Signalnummer %d\n",
                WSTOPSIG(status));
        /* I.d.R. wird dies SIGSTOP sein, aber es gibt */
        /* ja auch noch das ptrace()-Interface.        */
        return -1;
    }
}

int main() {

    // vars
    struct Program program;
    string input;
    int pId;
    int i, j;
    int sum_of_usertime;

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

        struct ChildProcess child[program.size];

        //fork
        for (i = 0; i < program.size; i++) {
            if ((child[i].pid=fork()) == 0) {

                execvp(program.command[i].progName, program.command[i].args);

                perror("Execvp error");
                _exit(1);
            }
            if (child[i].pid < 0) {
                perror("Fork error");
            }
        }

        //wait for execution
        for (i = 0; i < program.size; i++) {
            if (child[i].pid > 0) {
                int status;
                child[i].startTime = clock();
                waitpid(child[i].pid, &status, 0);
                child[i].endTime = clock();
                child[i].status = getChildStatus(status);
                if (status > 0) { //error
                    child[i].exitedWithError = true;
                }
            } else {
                //process did never started
                child[i].exitedWithError = true;
            }
        }

        /*
        for (i = 0; i < program.size; i++) {
            printf("Command: %d: %s \n", i, program.command[i].toString);
            printf("pid: %d \n", child[i].pid);
            printf("status: %d \n", child[i].status);
            printf("startTime: %d \n", child[i].startTime);
            printf("endTime: %d \n", child[i].endTime);
            printf("elipsed time: %d \n", child[i].endTime - child[i].startTime);
            printf("failed: %d \n\n", child[i].exitedWithError);
        }
         */

        //print
        for (i = 0; i < program.size; i++) {
            if (child[i].exitedWithError != 0) {
                printf("%s: [execution error]");
                continue;
            }
            printf("%s: user time = %d \n", program.command[i].progName, child[i].endTime - child[i].startTime);
        }

        //end test



        /*
        for(i=0; i < program.size; i++){
            int childStatus;
            clock_t before, difference;

            pId = fork(); /* Hier wird eine Kopie des laufenden Prozesses erzeugt!
            if (pId < 0) {
                perror("fork failed");
                exit(EXIT_FAILURE);
            }
            else if (pId == 0){
                /* child process

                childStatus = execvp(program.command[i].progName, program.command[i].args);
                perror(1);
            }
            else {
                /* Vaterprozess; n = Prozess-ID des Kindes
                before = clock();
                pId = wait(&childStatus);
                difference = clock() - before;

                switch(getChildStatus(childStatus))
                {
                    case 0:
                        printf("%s: user time = %d \n", program.command[i].progName, difference);
                        sum_of_usertime += difference;
                        break;
                    default:
                        printf("%s: %s\n", program.command[i].progName, "[execution error]");
                }

                /* In "&status" erscheint der mit "return ...;" aus main() zurückgegebene Wert!
            }
        }

        printf("sum of user times = %d \n", sum_of_usertime); */
        printf("run completed.\n");
    }

    return 0;
}
