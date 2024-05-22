#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/wait.h>

// delay execution
void delay(double seconds) {
    unsigned int microseconds = (unsigned int)(seconds * 1000000); // Convert seconds to microseconds
    usleep(microseconds);
}

// Function that implements the process manager.
int runProcessManager(int fileDescriptor)
{
    // vector<PcbEntry> pcbTable;
    // Attempt to create the init process.
    // if (!createProgram("init", pcbEntry[0].program)) {
    //     return EXIT_FAILURE;
    // }

    // pcbEntry[0].processId = 0;
    // pcbEntry[0].parentProcessId = -1;
    // pcbEntry[0].programCounter = 0;
    // pcbEntry[0].value = 0;
    // pcbEntry[0].priority = 0;
    // pcbEntry[0].state = STATE_RUNNING;
    // pcbEntry[0].startTime = 0;
    // pcbEntry[0].timeUsed = 0;
    // runningState = 0;
    
    // cpu.pProgram = &(pcbEntry[0].program);
    // cpu.programCounter = pcbEntry[0].programCounter;
    // cpu.value = pcbEntry[0].value;
    // timestamp = 0;
    // double avgTurnaroundTime = 0;

    // Loop until a 'T' is read, then terminate.
    char userInput;
    
    do {
        // Read a command character from the pipe.
        if (read(fileDescriptor, &userInput, sizeof(userInput)) != sizeof(userInput)) {
            // Assume the parent process exited, breaking the pipe.
            break;
        }

        //TODO: Write a switch statement
        switch (userInput) {
            case 'Q': 
                // quantum();
                printf("Option Q selected\n");
                break;

            case 'U':
                printf("Option U selected\n");
                break;

            case 'P':
                printf("Option P selected\n");
                break;

            case 'T':
                printf("Terminated\n");
                break;

            default:
                printf("Character entered is invalid\n");
        }
    } while (userInput != 'T');

    return EXIT_SUCCESS;
}

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

    int pipeDescriptors[2];
    pid_t processMgrPid;
    int result;

    if(pipe(pipeDescriptors) == -1) {exit(1);} // Check for errors with pipe
    if((processMgrPid = fork()) == -1) {exit(1);} // check for errors with fork

    if (processMgrPid == 0) {
        // The process manager process is running.
        // Close the unused write end of the pipe for the process manager process.
        close(pipeDescriptors[1]);
        
        // Run the process manager.
        result = runProcessManager(pipeDescriptors[0]);

        // Close the read end of the pipe for the process manager process (for cleanup purposes).
        close(pipeDescriptors[0]);
        _exit(result);
    } else {
        // The commander process is running.
        
        // Close the unused read end of the pipe for the commander process.
        close(pipeDescriptors[0]);
        
        // Variable for user inputs
        char userInput;

        // Loop until a 'T' is written or until the pipe is broken.
        do {
            delay(0.1);
            printf("\n$ ");
            scanf(" %c", &userInput); 
            
            // Pass commands to the process manager process via the pipe.
            if (write(pipeDescriptors[1], &userInput, sizeof(userInput)) != sizeof(userInput)) 
            {
                // Assume the child process exited, breaking the pipe.
                break;
            }
        } while (userInput != 'T');

        write(pipeDescriptors[1], &userInput, sizeof(userInput));
        
        // Close the write end of the pipe for the commander process (for cleanup purposes).
        close(pipeDescriptors[1]);
        
        // Wait for the process manager to exit.
        wait(&result);
    }

    return result;
}