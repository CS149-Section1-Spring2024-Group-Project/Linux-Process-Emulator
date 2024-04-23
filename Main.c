#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){

    /*
        Command Prompt
    */

    printf("\nCurrent command:\n\n");
    printf("    'Q'             End of one unit of time.\n\n");
    printf("The process manager executes the next instruction of the currently running simulated process, \n");
    printf("increments program counter value (except for F or R instructions), increments TIME, and then performs.\n");
    printf("and then performs scheduling. Note that scheduling may involve performing context switching.\n");
    printf("\n");
    printf("    'U'             Unblock the first simulated process in blocked queue.\n\n");
    printf("The process manager moves the first simulated process in the blocked queue to the ready state queue array.\n");
    printf("\n");
    printf("    'P'             Print the current state of the system.\n\n");
    printf("The process manager spawns a new reporter process.\n");
    printf("\n");
    printf("    'T'             Print the average turnaround time and terminate the system.\n\n");
    printf("The process manager first spawns a reporter process and then terminates after termination of the reporter process. \n");
    printf("The process manager ensures that no more than one reporter process is running at any moment. \n");
    printf("and then performs scheduling. Note that scheduling may involve performing context switching\n");
    printf("\n");
    printf("    'Exit'          Abruptly ends the program.\n\n");
    printf("The program will no longer listen to any user input, ending the program. \n");

    while (true) {
        char UserInputCommand[] = "";
        printf("\nEnter Command: ");
        scanf("%s", UserInputCommand);

        if ((strcmp(UserInputCommand, "Q")) == 0) {
            /*
                Q - End of one unit of time
            */
            printf("Q command\n");
        } else if ((strcmp(UserInputCommand, "U")) == 0) {
            /*
                U - Unblock the first simulated process in blocked queue
            */
            printf("U command\n");
        } else if ((strcmp(UserInputCommand, "P")) == 0) {
            /*
                P - Print the current state of the system
            */
            printf("P command\n");
        } else if ((strcmp(UserInputCommand, "T")) == 0) {
            /*
                T - Print the average turnaround time and terminate the system
            */
            printf("T command\n");
        } else if ((strcmp(UserInputCommand, "Exit")) == 0) {
            /*
                Exit - Abruptly ends the program
            */
            printf("Terminated Program\n");
            break;
        } else {
            printf("Invalid command\n");
        }
    }

    return 0;
}