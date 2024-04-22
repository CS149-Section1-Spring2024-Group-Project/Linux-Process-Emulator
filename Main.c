#include <stdio.h>

int main(int argc, char* argv[]){

    /*
        Command Prompt
    */

    printf("\nCurrent command:\n\n");
    printf("    'Q'             The process manager executes the next instruction of the currently running simulated process, \n");
    printf("                    increments program counter value (except for F or R instructions), increments TIME, and then performs.\n");
    printf("                    and then performs scheduling. Note that scheduling may involve performing context switching\n");
    printf("\n");
    printf("    'U'             The process manager moves the first simulated process in the blocked queue to the ready state queue array \n");
    printf("\n");
    printf("    'P'             The process manager spawns a new reporter process. \n");
    printf("\n");
    printf("    'T'             The process manager first spawns a reporter process and then terminates after termination of the reporter process. \n");
    printf("                    The process manager ensures that no more than one reporter process is running at any moment. \n");
    printf("                    and then performs scheduling. Note that scheduling may involve performing context switching\n");
    printf("\n");

    

    return 0;
}