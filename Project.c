#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include <sys/wait.h>

enum State {
    STATE_READY,
    STATE_RUNNING,
    STATE_BLOCKED
};

typedef struct {
    char operation;
    int intArg;
    char* stringArg;
} Instruction;

typedef struct {
    Instruction* pProgram;
    int programCounter;
    int value;
    int timeSlice;
    int timeSliceUsed;
} Cpu;

typedef struct {
    int processId;
    int parentProcessId;
    Instruction* program;
    unsigned int programCounter;
    int value;
    unsigned int priority;
    enum State state;
    unsigned int startTime;
    unsigned int timeUsed;
} PcbEntry;

void avgTTime();
bool createProgram();

PcbEntry* pcbTable = NULL;
double avgTurnaroundTime;
int programIndexCounter;

// delay execution
void delay(double seconds) {
    unsigned int microseconds = (unsigned int)(seconds * 1000000); // Convert seconds to microseconds
    usleep(microseconds);
}

// Function to erase a substring from the given string
void erase_substring(char* str, size_t pos, size_t len) {
    size_t str_len = strlen(str);

    // If the string is NULL or pos is out of bounds or len is 0, do nothing
    if (str == NULL || pos >= str_len || len == 0) {
        return;
    }

    // If the length to erase goes beyond the end of the string, adjust len
    if (pos + len > str_len) {
        len = str_len - pos;
    }

    // Shift characters to the left to remove the substring
    memmove(str + pos, str + pos + len, str_len - pos - len + 1);
}

// adjust the size of array
void adjustArraySize(int **arr, int newSize) {
    // Reallocate memory for the array
    int *newArr = (int *)realloc(*arr, newSize * sizeof(int));
    
    if (newArr == NULL) {
        printf("Memory reallocation failed\n");
        exit(EXIT_FAILURE);
    }

    *arr = newArr;
}

// return length of array
int getSize(int *arr) {
    // Calculate the size of the array
    return sizeof(arr) / sizeof(arr[0]);
}

// return length of instruction array
int getInstructionSize(Instruction *arr) {
    // Calculate the size of the array
    return sizeof(arr) / sizeof(arr[0]);
}

// return length of pcb array
int getPcbEntrySize(PcbEntry *arr) {
    // Calculate the size of the array
    return sizeof(arr) / sizeof(arr[0]);
}

// removes element from array
void removeElement(int *arr, int index) {
    int size = getSize(arr);

    if (index < 0 || index >= size) {
        printf("Invalid index\n");
        return;
    }

    // Shift elements to the left
    for (int i = index; i < size - 1; i++) {
        arr[i] = arr[i + 1];
    }

    adjustArraySize(&arr, size - 1);
}

// adds element to array
int addElement(int *arr, int index, int element) {
    int size = getSize(arr);

    if (index < 0 || index > size) {
        printf("Invalid index\n");
        return size;
    }

    // Reallocate memory to increase the size of the array
    adjustArraySize(&arr, size + 1);

    // Shift elements to the right to make space for the new element
    for (int i = size; i > index; i--) {
        arr[i] = arr[i - 1];
    }

    // Insert the new element at the specified index
    arr[index] = element;
}

// adds instruction to array
int addInstruction(Instruction *arr, int index, Instruction element) {
    int size = getInstructionSize(arr);

    if (index < 0 || index > size) {
        printf("Invalid index\n");
        return size;
    }

    // Reallocate memory to increase the size of the array
    adjustArraySize((int **)arr, size + 1);

    // Shift elements to the right to make space for the new element
    for (int i = size; i > index; i--) {
        arr[i] = arr[i - 1];
    }

    // Insert the new element at the specified index
    arr[index] = element;
}

PcbEntry pcbEntry[10];
unsigned int timestamp = 0;
Cpu cpu;

// For the states below, -1 indicates empty (since it is an invalid index).
int runningState = -1;
int readyState[0];
int blockedState[0];
// In this implementation, we'll never explicitly clear PCB entries and the index in
// the table will always be the process ID. These choices waste memory, but since this
// program is just a simulation it the easiest approach. Additionally, debugging is
// simpler since table slots and process IDs are never re-used.
double cumulativeTimeDiff = 0;
int numTerminatedProcesses = 0;

// Implements the S operation.
void set(int value)
{
    // TODO: Implement
    // 1. Set the CPU value to the passed-in value.
    cpu.value = value;
}

// Implements the A operation.
void add(int value)
{
    // TODO: Implement
    // 1. Add the passed-in value to the CPU value.
    cpu.value += value;
}

// Implements the D operation.
void decrement(int value)
{
    // TODO: Implement
    // 1. Subtract the integer value from the CPU value.
    cpu.value -= value;
}

// Performs scheduling.
void schedule()
{
    // TODO: Implement
    // 1. Return if there is still a processing running (runningState != -1). 
    //There is no need to schedule if a process is already running (at least until iLab 3)
    if (runningState != -1)
    {
        return;
    }
    // 2. Get a new process to run, if possible, from the ready queue.


    if (getSize(readyState) > 0)
    {
        int pcbIndex = readyState[0];
        removeElement(readyState, 0);
    // 3. If we were able to get a new process to run:
    // a. Mark the processing as running (update the new process's PCB state)
 
        runningState = pcbIndex;
        pcbEntry[pcbIndex].state = STATE_RUNNING;
    // b. Update the CPU structure with the PCB entry details(program, program counter, // value, etc.)

        cpu.pProgram = pcbEntry[pcbIndex].program;
        cpu.programCounter = pcbEntry[pcbIndex].programCounter;
        cpu.value = pcbEntry[pcbIndex].value;
        
    }
}
// Implements the B operation.
void block()
{
    // TODO: Implement
    
    // 1. Add the PCB index of the running process (stored in runningState) to the blocked queue.
    addElement(blockedState, getSize(blockedState) - 1, runningState);

    // 2. Update the process's PCB entry
    int pcbIndex = runningState;    
    
    // a. Change the PCB's state to blocked.
    pcbEntry[pcbIndex].state = STATE_BLOCKED;
    // b. Store the CPU program counter in the PCB's program counter.
    pcbEntry[pcbIndex].programCounter = cpu.programCounter;
    // c. Store the CPU's value in the PCB's value.
    pcbEntry[pcbIndex].value = cpu.value;
    // 3. Update the running state to -1 (basically mark no process as running). Note that a new process will be chosen to run later
    runningState = -1;
    // (via theQ command code calling the schedule() function).
}

// Implements the E operation.
void end()
{
    // TODO: Implement
    // 1. Get the PCB entry of the running process.
    int pcbIndex = runningState;
    PcbEntry process = pcbEntry[runningState]; 
    // 2. Update the cumulative time difference (increment it by
    // timestamp + 1 - start time of the process).
    cumulativeTimeDiff += timestamp + 1- process.startTime;
    // 3. Increment the number of terminated processes.
    numTerminatedProcesses++;
    // 4. Update the running state to -1 (basically mark no process as running).
    runningState = -1;

    // Note that a new process will be chosen to run later (via the Q command code calling the schedule function).
}

bool createProgram(const char* filename, Instruction* program)
{
    /*
    ifstream file;
    int lineNum = 0;
    file.open(filename.c_str());
    if (!file.is_open())
    {
        cout << "Error opening file " << filename << endl;
        return false;
    }
    while (file.good()) 
    {
        char* line;
        getline(file, line);
        
        trim(line);
        if (strlen(line) > 0)
        {
            Instruction instruction;
            instruction.operation = toupper(line[0]);
            instruction.stringArg = trim(line.erase(0, 1));
            line.erase(0, 1);
            trim(line);

            stringstream argStream(instruction.stringArg);
            switch (instruction.operation)
            {
            case 'S': 
                set(instruction.intArg);
                break;// Integer argument.  
            case 'A': 
                add(instruction.intArg);// Integer argument.
                break;
            case 'D': 
                decrement(instruction.intArg);// Integer argument.
                break;
            case 'F': 
                 // Integer argument.
                if (!(argStream >> instruction.intArg))
                {
                    cout << filename << ":" << lineNum
                         << " - Invalid integer argument "
                         << instruction.stringArg << " for "
                         << instruction.operation << " operation"
                         << endl;
                    file.close();
                    return false;
                }
                break;
            case 'B': // No argument.
            case 'E': // No argument.
                break;
            case 'R': // String argument.
                      // Note that since the string is trimmed on both ends, filenames
                      // with leading or trailing whitespace (unlikely) will not work.
                if (strlen(instruction.stringArg) == 0)
                {
                    cout << filename << ":" << lineNum << " - Missing string argument" << endl;
                    file.close();
                    return false;
                }
                break;
            default:
                cout << filename << ":" << lineNum << " - Invalid operation, "
                     << instruction.operation << endl;
                file.close();
                return false;
            }
            addInstruction(program, getSize(program), instruction);
        }
        lineNum++;
    }
    file.close();
    */
    return true;
}

//Implements F
void F(int value)
{
    // TODO: Implement
    // 1. Get a free PCB index (pcbTable.size())
    int pcbIndex = sizeof(pcbTable) / sizeof(pcbTable[0]);
 
    // 2. Get the PCB entry for the current running process.
    int runningIndex = runningState;
    PcbEntry parentProcess = pcbEntry[pcbIndex];
    // 3. Ensure the passed-in value is not out of bounds.
    if (value < 0 || value >= getPcbEntrySize(pcbTable))
    {
        printf("Invalid value for fork operation");
        return;
    } else {
    // 4. Populate the PCB entry obtained in #1

    // a. Set the process ID to the PCB index obtained in #1.
    pcbEntry[pcbIndex].processId = pcbIndex;
    // b. Set the parent process ID to the process ID of the
    pcbEntry[pcbIndex].parentProcessId = pcbEntry[runningIndex].processId;

    // running process (use the running process's PCB entry to get this).
    pcbEntry[pcbIndex].program = pcbEntry[runningIndex].program;
    // c. Set the program counter to the cpu program counter.
    pcbEntry[pcbIndex].programCounter = cpu.programCounter;
    // d. Set the value to the cpu value.
    pcbEntry[pcbIndex].value = cpu.value;
    // e. Set the priority to the same as the parent process's priority.
    pcbEntry[pcbIndex].priority = pcbEntry[runningIndex].priority;
    // f. Set the state to the ready state.
    pcbEntry[pcbIndex].state = STATE_READY;
    // g. Set the start time to the current timestamp
    pcbEntry[pcbIndex].startTime = timestamp;
    pcbEntry[pcbIndex].timeUsed = 0;
    // 5. Add the pcb index to the ready queue.
    addElement(readyState, getSize(readyState) - 1, pcbIndex);
    // 6. Increment the cpu's program counter by the value read in #3
    cpu.programCounter += value;
    }
}
// Implements the R operation.
void replace(char* argument)
{
    // TODO: Implement
    // 1. Clear the CPU's program (cpu.pProgram->clear()).
    printf("Error reading program file %s", argument);
    // 2. Use createProgram() to read in the filename specified by
    // argument into the CPU (*cpu.pProgram)
    // a. Consider what to do if createProgram fails. I printed an
    // error, incremented the cpu program counter and then returned. Note
    // that createProgram can fail if the file could not be opened or did not exist.
    if (!createProgram(argument, cpu.pProgram))
    {
        printf("Error reading program file %s", argument);
        cpu.programCounter++;
        return;
    }
    // 3. Set the program counter to 0.
    cpu.programCounter = 0;

}
// Implements the Q command.
void quantum()
{
    Instruction instruction;
    printf("In quantum");
    if (runningState == -1) {
        printf("No processes are running");
        ++timestamp;
        return;
    }

    if (cpu.programCounter < getInstructionSize(cpu.pProgram)) {
        // instruction = (*cpu.pProgram)[cpu.programCounter]; original
        instruction = cpu.pProgram[cpu.programCounter];
        ++cpu.programCounter;
    } else {
        printf("End of program reached without E operation");
        instruction.operation = 'E';
    }
    switch (instruction.operation) {
    case 'S':
        set(instruction.intArg);
        printf("instruction S %d", instruction.intArg);
        break;

    case 'A':
        add(instruction.intArg);
        printf("instruction A %d", instruction.intArg);
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
        F(instruction.intArg);
        break;
        
    case 'R':
        replace(instruction.stringArg);
        break;
    }
    ++timestamp;
    schedule();
}

// Implements the U command.
void unblock() {
    // 1. If the blocked queue contains any processes:
    // a. Remove a process form the front of the blocked queue.
    if (getSize(blockedState) > 0) {
        int pcbIndex = blockedState[0];
        removeElement(blockedState, 0);
    
    //b. Add the process to the ready queue.
        addElement(readyState, getSize(readyState) - 1, pcbIndex);
    // c. Change the state of the process to ready (update its PCB entry).
        pcbEntry[pcbIndex].state = STATE_READY;
    // 2. Call the schedule() function to give an unblocked process a
    // chance to run (if possible).
    }
    schedule();
}
// Implements the P command.
void print() 
{
    printf("****************************************************************\n");
    printf("The current system state is as follows:\n");
    printf("****************************************************************\n\n");

    printf("CURRENT TIME: %d\n\n", timestamp);

    printf("RUNNING PROCESS:\n");
    printf("%d\n\n", runningState);

    printf("BLOCKED PROCESSES\n");
    int blockedStateLength = sizeof(blockedState) / sizeof(blockedState[0]);
    for (int i = 0; i < blockedStateLength; i++) {
        printf("%d\n\n", blockedState[0]);
    }

    printf("PROCESSES READY TO EXECUTE:\n");
    int currentPriortyListing = -1;

    for (int i = 0; i < 10; i++) {
        if(pcbEntry[i].priority != currentPriortyListing) {
            currentPriortyListing++;
            printf("Queue of processes with priority %d:\n", currentPriortyListing);
        }

        char stringResult[100];
        char numToString[100];

        sprintf(numToString, "%d", pcbEntry[i].processId);
        strcat(stringResult, numToString);
        strcat(stringResult, ", ");
        sprintf(numToString, "%d", pcbEntry[i].parentProcessId);
        strcat(stringResult, numToString);
        strcat(stringResult, ", ");
        sprintf(numToString, "%d", pcbEntry[i].value);
        strcat(stringResult, numToString);
        strcat(stringResult, ", ");
        sprintf(numToString, "%d", pcbEntry[i].startTime);
        strcat(stringResult, numToString);
        strcat(stringResult, ", ");
        sprintf(numToString, "%d", pcbEntry[i].timeUsed);
        strcat(stringResult, numToString);

        printf("%s\n", stringResult);
    }

    printf("****************************************************************\n");
}

void avgTTime() {
    int totalTime = 0;

    for(int h = 0; h <= programIndexCounter; h++){
        totalTime += pcbTable[h].timeUsed;
    }

    avgTurnaroundTime = totalTime / (double) programIndexCounter;

    printf("Average turnaround time: %lf\n", avgTurnaroundTime);
}

// Function that implements the process manager.
int runProcessManager(int fileDescriptor) {
    //  Attempt to create the init process.
    if (!createProgram("init", pcbEntry[0].program))
    {
        return EXIT_FAILURE;
    }

    pcbEntry[0].processId = 0;
    pcbEntry[0].parentProcessId = -1;
    pcbEntry[0].programCounter = 0;
    pcbEntry[0].value = 0;
    pcbEntry[0].priority = 0;
    pcbEntry[0].state = STATE_RUNNING;
    pcbEntry[0].startTime = 0;
    pcbEntry[0].timeUsed = 0;
    runningState = 0;
    cpu.pProgram = pcbEntry[0].program;
    cpu.programCounter = pcbEntry[0].programCounter;
    cpu.value = pcbEntry[0].value;
    timestamp = 0;
    programIndexCounter = 0;
    avgTurnaroundTime = 0;
    
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
                printf("Option Q selected\n");
                // quantum();
                break;

            case 'U':
                printf("Option U selected\n");
                // unblock();
                break;

            case 'P':
                printf("Option P selected\n");
                // print();
                break;

            case 'T':
                printf("Terminated\n");
                break;

            default:
                printf("Character entered is invalid\n");
        }
    } while (userInput != 'T');

    // Print the average time
    avgTTime();

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {

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
            printf("$ ");
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