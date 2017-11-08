#include "refs.h"

bool debugging = true;

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

int printChild(ChildProcess c){
    printf("Child Process <%d> \n", c.pid);
    printf("-> task: %s \n", c.task);
    printf("-> status: %d \n", c.status);
    printf("-> startTime: %d \n", c.startTime);
    printf("-> endTime: %d \n", c.endTime);
    printf("-> elipsed time: %d \n", c.endTime - c.startTime);
    printf("-> failed: %d \n\n", c.exitedWithError);
}

int main() {

    // varsmk
    Program program;
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

        ChildProcess childs[program.size];

        //fork
        for (i = 0; i < program.size; i++) {
            if ((childs[i].pid=fork()) == 0) {

                execvp(program.command[i].progName, program.command[i].args);

                //perror("Execvp error");
                _exit(errno);
            }
            if (childs[i].pid < 0) {
                perror("Fork error");
            }
        }

        //wait for execution
        for (i = 0; i < program.size; i++) {
            if (childs[i].pid > 0) {
                int status;
                childs[i].task = program.command[i].toString; //todo -> change to program.command[i].args[0]

                //measure time
                childs[i].startTime = clock();
                waitpid(childs[i].pid, &status, 0);
                childs[i].endTime = clock();

                childs[i].status = getChildStatus(&status);
                if (status > 0) { //error
                    childs[i].exitedWithError = true;
                }
            } else {
                //process never started
                childs[i].exitedWithError = true;
            }
        }


        debug(foreach(ChildProcess *c, childs) printChild(*c);)

        foreach (ChildProcess *c, childs) {
            if (c->exitedWithError) {
                printf("%s: [execution error]\n", c->task);
                continue;
            }
            printf("%s: user time = %d\n", c->task, c->endTime - c->startTime);

        }

        /*print
        for (i = 0; i < program.size; i++) {
            if (childs[i].exitedWithError != 0) {
                printf("%s: [execution error]");
                continue;
            }
            printf("%s: user time = %d \n", program.command[i].progName, childs[i].endTime - childs[i].startTime);
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
                /* childs process

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
        printf("\n --> run completed.\n");
    }

    return 0;
}
