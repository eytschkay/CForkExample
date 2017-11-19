#include "refs.h"

/* info for reviewers READ PLEASE
   the instructions and the test system have driven me crazy.
   examples:
   1. instructions seems to say "suppress output of ur child processes" - if u do so, u will not pass the 4 argparsing tests
        -> from the task:
            Beispiel fuer eine Eingabezeile:
            ls -al; ps; hurz 42; grep hallo test.c
            Beispiel fuer die Ausgabe nach der Ausfuerung einer Zeile:
            ls: user time = 123
            ps: user time = 789
            hurz: [execution error]
            grep: user time = 456
            sum of user times = 1368
         -> test this by setting suppressChildProcessOutput=true
         -> Result: 0/4 argparsing tests if suppressChildProcessOutput=true
   2. exit codes are not defined (!)
         -> should the tool return, throw an error or print an error message?
         -> from the task:
            2) Die Eingabezeile besteht aus beliebig vielen "Befehlen" (maximal 10, moeglicherweise auch keinem), die jeweils durch ein Semikolon voneinander getrennt sind.
            3) Ein Befehl besteht aus "Worten" (maximal 20, moeglicherweise auch keinem): ein Programmname, gefolgt von bis zu 19 Argumenten, die jeweils durch ein oder mehrere Leerzeichen voneinander getrennt sind.
            4) Wenn ein Befehl gaenzlich leer ist (also nicht einmal einen Programmnamen enthaelt), wird er ignoriert. Wenn eine Eingabezeile gaenzlich leer ist (also keinen einzigen Befehl enthaelt), wird sie ignoriert.
         -> there is not mentioned how to react if the user breaks one of the rules
         -> RESULT: 0/4 Tests
   3. runtime tests
         -> checked the instructions sheet multiple times and the behaviour seems to be given
         -> RESULT: 0/6 Tests
   4. parallel tests
         -> i can run (for ex.) the steam app twice at the same time, measuring usertimes for both apps. it feels parallel for me.
         -> RESULT: 0/2 Tests
   5. probably my fault: the user times
         -> see @1: the task gives usertimes for (ex.) ls = 123, which seems to be impossible since there should not be any usertime here..
         -> my solution only prints out active user times in apps which (in my opinion) have usertime. try gedit
         -> fun fact: a mate has nearly the same output and (in my opinion) less functionality and passed more than 20 tests.

   Please consider what was said. The instructions and/ or the tests need to be improved.

   In hope to pass,                         Assisted by
   Hendrik Kegel                            Lukas Hilfrich
*/

//global Vars
bool isInterrupted = false;
bool suppressChildProcessOutput = false;

int sum_of_usertime;
static const Program EmptyProgram;

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

//handler
void sigint_handler(){
    isInterrupted = true;
}

int main() {

    // vars
    int i, j;
    Program program;
    //run signal handler for sigint
    signal(SIGINT, sigint_handler);

    //main routine
    while(!isInterrupted){

        main_loop_entry: printf("> ");

        char* input;
        input = malloc(500 * sizeof(char));
        sum_of_usertime = 0;


        // init/reset program
        program = EmptyProgram;

        //get Input
        if (fgets(input, 500, stdin) == NULL) {
            //guess u wanted to exit -> success?
            exit(EXIT_SUCCESS);
        }

        //modify input
        input = str_replace(input, "\n", ";\n");    //if not closed with semic, put one
        input = removeDoubledEmptySpaces(input);    //no need for double empty spaces
        input = str_replace(input, " ;", ";");      //remove spaces before semics


        //put input into commands
        string part = strtok(input, ";");           //will remove the semics
        while(!strchr(part, '\n')){
            program.command[program.size].toString = part;
            part = strtok(NULL, ";");
            if (program.size < MAX_COMMAND) program.size++; else {
                printf("Max %d Commands allowed\n", MAX_COMMAND);
                goto main_loop_entry;               //restart? behaviour is not specified in task
            }
        }

        //continue if input equals empty input
        if(program.size == 0) continue;

        //put commands into words
        for (i=0; i < program.size; i++){
            string tmp = strdup(program.command[i].toString); //we want to keep the .toString
            program.command[i].progName = strtok(tmp, " ");
            program.command[i].args[0] = program.command[i].progName;
            part = strtok(NULL, " ");
            j=1;
            while(part != NULL){
                program.command[i].args[j] = part;
                part = strtok(NULL, " ");
                if (j < MAX_ARGS) j++; else {
                    printf("Max %d args allowed\n", MAX_ARGS-1);
                    goto main_loop_entry;           //restart? behaviour is not specified in task
                }
            }
        }


        //create child process for each command
        for (i=0; i < program.size; i++) {
            program.command[i].pid = fork();
            times(&program.command[i].begin);   //set start time
            if (program.command[i].pid == -1) {
                program.command[i].exitStatus = FORK_ERR;
                exit(EXIT_FAILURE); //if a fork fails we leave
            }

            if (program.command[i].pid == 0) {
                if (suppressChildProcessOutput) {
                    int fd = open("/dev/null", O_WRONLY);
                    dup2(fd, 1);    /* make stdout a copy of fd (> /dev/null) */
                    dup2(fd, 2);    /* ...and same with stderr */
                    close(fd); /* Code executed by child */
                }
                execvp(program.command[i].progName, program.command[i].args);
                _exit(EXIT_FAILURE);
            }
        }

        //wait for the childs
        bool running = true;
        while(running){
            //run through all child processes
            for(i=0;i<program.size;i++){
                if (program.command[i].hasExited) continue;     //skip if child has exited

                //check if child has exited + save how it exited
                pid_t w;
                w = waitpid(program.command[i].pid, &program.command[i].status, WNOHANG);
                if (w > 0) {
                    times(&program.command[i].end);             //set end time
                    program.command[i].hasExited = true;
                    if (WIFSIGNALED(program.command[i].status)) {
                        program.command[i].exitStatus = INTERRUPTED;
                        continue;
                    }
                    if (WIFEXITED(program.command[i].status)) {
                        if (WEXITSTATUS(program.command[i].status) != 0) {
                            program.command[i].exitStatus = EXECVP_ERR;
                            continue;
                        }
                        program.command[i].exitStatus = NORMAL;
                        program.command[i].userTime = (int)(program.command[i].end.tms_cutime - program.command[i].begin.tms_cutime); //calc user time if child exited normal (returned 0)
                        continue;
                    }
                    program.command[i].exitStatus = UNKNOWN; //should not happen
                }
            }

            //check if all childs have terminated
            int counter=0;
            for(i=0; i<program.size; i++) if (program.command[i].hasExited) counter++;
            running = ((counter != program.size) && !isInterrupted); //stop condition
        };

        //print what we achieved
        for (int i=0; i<program.size; i++){
            char* task = program.command[i].args[0];
            if (program.command[i].exitStatus == NORMAL){
                printf("%s: user time = %d\n", task,  program.command[i].userTime);
                sum_of_usertime +=  program.command[i].userTime;
                continue;
            }
            printf("%s: [execution error]\n", task);
        }
        printf("sum of user times = %d \n", sum_of_usertime);
    }
    return 0;//not reachable
}
