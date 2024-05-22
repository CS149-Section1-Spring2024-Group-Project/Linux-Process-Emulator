#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/wait.h>

// Constants for process states
#define STATE_RUNNING 3
#define STATE_READY 1
#define STATE_BLOCKED 2
#define STATE_TERMINATED 0

// Instruction structure
typedef struct {
    char operation;
    int intArg;
    char stringArg[50];
} Instruction;

// PCB Entry structure
typedef struct {
    int processId;
    int value;
    int programCounter;
    int priority;
    int state;
    int startTime;
    int timeUsed;
} PCBEntry;

Instruction program[100];
PCBEntry pcbEntry[10];
int runningState = -1;
int timestamp = 0;
int programSize = 0;

struct {
    int programCounter;
    Instruction *pProgram;
} cpu;

int readyState[100];
int readyStateSize = 0;
int blockedState[100];
int blockedStateSize = 0;

// Delay execution
void delay(double seconds) {
    unsigned int microseconds = (unsigned int)(seconds * 1000000); // Convert seconds to microseconds
    usleep(microseconds);
}

// Function to set a value
void set(int arg) {
    if (runningState != -1) {
        pcbEntry[runningState].value = arg;
    }
}

// Function to add a value
void add(int arg) {
    if (runningState != -1) {
        pcbEntry[runningState].value += arg;
    }
}

// Function to decrement a value
void decrement(int arg) {
    if (runningState != -1) {
        pcbEntry[runningState].value -= arg;
    }
}

// Function to block a process
void block() {
    if (runningState != -1) {
        pcbEntry[runningState].state = STATE_BLOCKED;
        blockedState[blockedStateSize++] = runningState;
        runningState = -1;
    }
}

// Function to end a process
void end() {
    if (runningState != -1) {
        pcbEntry[runningState].state = STATE_TERMINATED;
        runningState = -1;
    }
}

// Function to fork a process
void fork(int arg) {
    for (int i = 0; i < 10; ++i) {
        if (pcbEntry[i].processId == -1) {  // Find an empty PCB entry
            pcbEntry[i] = pcbEntry[runningState];  // Copy current process
            pcbEntry[i].processId = i;  // Assign new process ID
            pcbEntry[i].programCounter += arg;  // Modify PC as per fork logic
            readyState[readyStateSize++] = i;  // Add to ready queue
            break;
        }
    }
}

// Function to replace a string argument
void replace(char* arg) {
    if (runningState != -1) {
        strcpy(cpu.pProgram[cpu.programCounter].stringArg, arg);
    }
}

// Function to schedule processes
void schedule() {
    if (runningState == -1 && readyStateSize > 0) {
        runningState = readyState[0];
        for (int i = 1; i < readyStateSize; ++i) {
            readyState[i - 1] = readyState[i];
        }
        --readyStateSize;
        pcbEntry[runningState].state = STATE_RUNNING;
        cpu.pProgram = &(program[0]);  // Assuming the program array is used as a placeholder. Adjust as needed.
        cpu.programCounter = pcbEntry[runningState].programCounter;
        cpu.value = pcbEntry[runningState].value;
    }
}

// Function to execute a quantum of processing time
void quantum() {
    Instruction instruction;
    printf("In quantum\n");
    if (runningState == -1) {
        printf("No processes are running\n");
        ++timestamp;
        return;
    }
    if (cpu.programCounter < programSize) {
        instruction = cpu.pProgram[cpu.programCounter];
        ++cpu.programCounter;
    } else {
        printf("End of program reached without E operation\n");
        instruction.operation = 'E';
    }
    switch (instruction.operation) {
        case 'S':
            set(instruction.intArg);
            printf("instruction S %d\n", instruction.intArg);
            break;
        case 'A':
            add(instruction.intArg);
            printf("instruction A %d\n", instruction.intArg);
            break;
        case 'D':
            decrement(instruction.intArg);
            break;
        case 'B':
            block();
            break;
        case 'E':
            end();
            break;
        case 'F':
            fork(instruction.intArg);
            break;
        case 'R':
            replace(instruction.stringArg);
            break;
    }
    ++timestamp;
    schedule();
}

// Function to unblock a process
void unblock() {
    if (blockedStateSize > 0) {
        int pcbIndex = blockedState[0];
        for (int i = 1; i < blockedStateSize; ++i) {
            blockedState[i - 1] = blockedState[i];
        }
        --blockedStateSize;
        readyState[readyStateSize++] = pcbIndex;
        pcbEntry[pcbIndex].state = STATE_READY;
    }
    schedule();
}

// Function to print the current system state
void print() {
    printf("______________________________________________________________\n");
    printf("The current system state is as follows:\n");
    printf("______________________________________________________________\n");
    printf("Time: %d\n", timestamp);
    printf("Running Process: %d\n", runningState);
    
    printf("Ready Queue: ");
    for (int i = 0; i < readyStateSize; i++) {
        printf("%d ", readyState[i]);
    }
    printf("\n");
    
    printf("Blocked Queue: ");
    for (int i = 0; i < blockedStateSize; i++) {
        printf("%d ", blockedState[i]);
    }
    printf("\n");
    
    for (int i = 0; i < 10; i++) {
        if (pcbEntry[i].processId != -1) {
            printf("Process %d: Value = %d, PC = %d, Priority = %d, State = %d, Start Time = %d, CPU Time = %d\n",
                   pcbEntry[i].processId, pcbEntry[i].value, pcbEntry[i].programCounter,
                   pcbEntry[i].priority, pcbEntry[i].state, pcbEntry[i].startTime,
                   pcbEntry[i].timeUsed);
        }
    }
    printf("\n");
}

// Function that implements the process manager
int runProcessManager(int fileDescriptor) {
    // Assuming the initial PCB setup is done here
    
    // Loop until a 'T' is read, then terminate
    char userInput;
    
    do {
        // Read a command character from the pipe
        if (read(fileDescriptor, &userInput, sizeof(userInput)) != sizeof(userInput)) {
            // Assume the parent process exited, breaking the pipe
            break;
        }

        // Handle the command
        switch (userInput) {
            case 'Q': 
                quantum();
                printf("Option Q selected\n");
                break;

            case 'U':
                unblock();
                printf("Option U selected\n");
                break;

            case 'P':
                print();
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

// Main function to start the process manager
int main(int argc, char* argv[]) {
    printf("\nCurrent command:\n\n");
    printf("    'Q'             End of one unit of time.\n\n");
    printf("The process manager executes the next instruction of the currently running simulated process,\n");
    printf("increments program counter value (except for F or R instructions), increments TIME, and then performs\n");
    printf("scheduling. Note that scheduling may involve performing context switching.\n");
    printf("\n");
    printf("    'U'             Unblock the first simulated process in blocked queue.\n\n");
    printf("The process manager moves the first simulated process in the blocked queue to the ready state queue array.\n");
    printf("\n");
    printf("    'P'             Print the current state of the system.\n\n");
    printf("The process manager spawns a new reporter process.\n");
    printf("\n");
    printf("    'T'             Print the average turnaround time and terminate the system.\n\n");
    printf("The process manager first spawns a reporter process and then terminates after termination of the reporter process.\n");
    printf("The process manager ensures that no more than one reporter process is running at any moment.\n");

    int pipeDescriptors[2];
    pid_t processMgrPid;
    int result;

    if (pipe(pipeDescriptors) == -1) {
        exit(1); // Check for errors with pipe
    }
    if ((processMgrPid = fork()) == -1) {
        exit(1); // check for errors with fork
    }

    if (processMgrPid == 0) {
        // The process manager process is running
        // Close the unused write end of the pipe for the process manager process
        close(pipeDescriptors[1]);
        
        // Run the process manager
        result = runProcessManager(pipeDescriptors[0]);

        // Close the read end of the pipe for the process manager process (for cleanup purposes)
        close(pipeDescriptors[0]);
        _exit(result);
    } else {
        // The commander process is running
        
        // Close the unused read end of the pipe for the commander process
        close(pipeDescriptors[0]);
        
        // Variable for user inputs
        char userInput;

        // Loop until a 'T' is written or until the pipe is broken
        do {
            delay(0.1);
            printf("\n$ ");
            scanf(" %c", &userInput); 
            
            // Pass commands to the process manager process via the pipe
            if (write(pipeDescriptors[1], &userInput, sizeof(userInput)) != sizeof(userInput)) {
                // Assume the child process exited, breaking the pipe
                break;
            }
        } while (userInput != 'T');

        write(pipeDescriptors[1], &userInput, sizeof(userInput));
        
        // Close the write end of the pipe for the commander process (for cleanup purposes)
        close(pipeDescriptors[1]);
        
        // Wait for the process manager to exit
        wait(&result);
    }

    return result;
}
