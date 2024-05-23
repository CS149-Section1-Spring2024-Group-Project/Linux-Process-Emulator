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
bool createProgram(const char* filename, Instruction* program);

PcbEntry* pcbTable = NULL;
double avgTurnaroundTime;
int programIndexCounter;

// delay execution
void delay(double seconds) {
    usleep((unsigned int)(seconds * 1000000)); // Convert seconds to microseconds
}

// Function to erase a substring from the given string
void erase_substring(char* str, size_t pos, size_t len) {
    size_t str_len = strlen(str);

    if (str == NULL || pos >= str_len || len == 0) {
        return;
    }

    if (pos + len > str_len) {
        len = str_len - pos;
    }

    memmove(str + pos, str + pos + len, str_len - pos - len + 1);
}

// adjust the size of array
void adjustArraySize(int** arr, int newSize) {
    int* newArr = (int*)realloc(*arr, newSize * sizeof(int));
    if (newArr == NULL) {
        printf("Memory reallocation failed\n");
        exit(EXIT_FAILURE);
    }
    *arr = newArr;
}

// return length of array
int getSize(int* arr) {
    // Placeholder, original logic is incorrect
    return 0;
}

// return length of instruction array
int getInstructionSize(Instruction* arr) {
    // Placeholder, original logic is incorrect
    return 0;
}

// return length of pcb array
int getPcbEntrySize(PcbEntry* arr) {
    // Placeholder, original logic is incorrect
    return 0;
}

// removes element from array
void removeElement(int* arr, int index) {
    int size = getSize(arr);

    if (index < 0 || index >= size) {
        printf("Invalid index\n");
        return;
    }

    for (int i = index; i < size - 1; i++) {
        arr[i] = arr[i + 1];
    }

    adjustArraySize(&arr, size - 1);
}

// adds element to array
int addElement(int* arr, int index, int element) {
    int size = getSize(arr);

    if (index < 0 || index > size) {
        printf("Invalid index\n");
        return size;
    }

    adjustArraySize(&arr, size + 1);

    for (int i = size; i > index; i--) {
        arr[i] = arr[i - 1];
    }

    arr[index] = element;
    return size + 1;
}

// adds instruction to array
int addInstruction(Instruction* arr, int index, Instruction element) {
    int size = getInstructionSize(arr);

    if (index < 0 || index > size) {
        printf("Invalid index\n");
        return size;
    }

    adjustArraySize((int**)&arr, size + 1);

    for (int i = size; i > index; i--) {
        arr[i] = arr[i - 1];
    }

    arr[index] = element;
    return size + 1;
}

PcbEntry pcbEntry[10];
unsigned int timestamp = 0;
Cpu cpu;
int runningState = -1;
int readyState[0];
int blockedState[0];
double cumulativeTimeDiff = 0;
int numTerminatedProcesses = 0;

void set(int value) {
    cpu.value = value;
}

void add(int value) {
    cpu.value += value;
}

void decrement(int value) {
    cpu.value -= value;
}

void schedule() {
    if (runningState != -1) {
        return;
    }

    if (getSize(readyState) > 0) {
        int pcbIndex = readyState[0];
        removeElement(readyState, 0);
        runningState = pcbIndex;
        pcbEntry[pcbIndex].state = STATE_RUNNING;
        cpu.pProgram = pcbEntry[pcbIndex].program;
        cpu.programCounter = pcbEntry[pcbIndex].programCounter;
        cpu.value = pcbEntry[pcbIndex].value;
    }
}

void block() {
    addElement(blockedState, getSize(blockedState), runningState);
    int pcbIndex = runningState;
    pcbEntry[pcbIndex].state = STATE_BLOCKED;
    pcbEntry[pcbIndex].programCounter = cpu.programCounter;
    pcbEntry[pcbIndex].value = cpu.value;
    runningState = -1;
}

void end() {
    int pcbIndex = runningState;
    PcbEntry process = pcbEntry[runningState];
    cumulativeTimeDiff += timestamp + 1 - process.startTime;
    numTerminatedProcesses++;
    runningState = -1;
}

bool createProgram(const char* filename, Instruction* program) {
    // TODO: Implement actual file reading logic
    return true;
}

void forkProcess(int value) {
    int pcbIndex = sizeof(pcbTable) / sizeof(pcbTable[0]);
    int runningIndex = runningState;
    PcbEntry parentProcess = pcbEntry[pcbIndex];

    if (value < 0 || value >= getPcbEntrySize(pcbTable)) {
        printf("Invalid value for fork operation");
        return;
    }

    pcbEntry[pcbIndex].processId = pcbIndex;
    pcbEntry[pcbIndex].parentProcessId = pcbEntry[runningIndex].processId;
    pcbEntry[pcbIndex].program = pcbEntry[runningIndex].program;
    pcbEntry[pcbIndex].programCounter = cpu.programCounter;
    pcbEntry[pcbIndex].value = cpu.value;
    pcbEntry[pcbIndex].priority = pcbEntry[runningIndex].priority;
    pcbEntry[pcbIndex].state = STATE_READY;
    pcbEntry[pcbIndex].startTime = timestamp;
    pcbEntry[pcbIndex].timeUsed = 0;
    addElement(readyState, getSize(readyState), pcbIndex);
    cpu.programCounter += value;
}

void replace(char* argument) {
    printf("Error reading program file %s", argument);
    if (!createProgram(argument, cpu.pProgram)) {
        printf("Error reading program file %s", argument);
        cpu.programCounter++;
        return;
    }
    cpu.programCounter = 0;
}

void quantum() {
    Instruction instruction;
    if (runningState == -1) {
        printf("No processes are running\n");
        ++timestamp;
        return;
    }

    if (cpu.programCounter < getInstructionSize(cpu.pProgram)) {
        instruction = cpu.pProgram[cpu.programCounter];
        ++cpu.programCounter;
    } else {
        printf("End of program reached without E operation\n");
        instruction.operation = 'E';
    }

    switch (instruction.operation) {
        case 'S':
            set(instruction.intArg);
            break;
        case 'A':
            add(instruction.intArg);
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
            forkProcess(instruction.intArg);
            break;
        case 'R':
            replace(instruction.stringArg);
            break;
    }

    ++timestamp;
    schedule();
}

void unblock() {
    if (getSize(blockedState) > 0) {
        int pcbIndex = blockedState[0];
        removeElement(blockedState, 0);
        addElement(readyState, getSize(readyState), pcbIndex);
        pcbEntry[pcbIndex].state = STATE_READY;
    }
    schedule();
}

void print() {
    printf("****************************************************************\n");
    printf("The current system state is as follows:\n");
    printf("****************************************************************\n\n");

    printf("CURRENT TIME: %d\n\n", timestamp);

    printf("RUNNING PROCESS:\n");
    printf("%d\n\n", runningState);

    printf("BLOCKED PROCESSES\n");
    int blockedStateLength = getSize(blockedState);
    for (int i = 0; i < blockedStateLength; i++) {
        printf("%d\n\n", blockedState[i]);
    }

    printf("PROCESSES READY TO EXECUTE:\n");
    int currentPriorityListing = -1;

    for (int i = 0; i < 10; i++) {
        if (pcbEntry[i].priority != currentPriorityListing) {
            currentPriorityListing++;
            printf("Queue of processes with priority %d:\n", currentPriorityListing);
        }

        printf("%d, %d, %d, %d, %d\n",
               pcbEntry[i].processId,
               pcbEntry[i].parentProcessId,
               pcbEntry[i].value,
               pcbEntry[i].startTime,
               pcbEntry[i].timeUsed);
    }

    printf("****************************************************************\n");
}

void avgTTime() {
    int totalTime = 0;

    for (int h = 0; h < 10; h++) {
        PcbEntry process = pcbEntry[h];
        totalTime += timestamp - process.startTime;
    }

    avgTurnaroundTime = (double)totalTime / 10;
    printf("\n\nThe average turnaround time for the system of processes is %.2f time units.\n\n", avgTurnaroundTime);
}

int main() {
    char* filename = "program.txt";
    int i;
    Instruction program[100];

    if (!createProgram(filename, program)) {
        printf("Error reading program file %s", filename);
        return 1;
    }

    Instruction initProgram[] = {{'S', 0, NULL}, {'A', 1, NULL}, {'D', 2, NULL}, {'B', 0, NULL}, {'E', 0, NULL}};

    for (i = 0; i < 10; i++) {
        pcbEntry[i].processId = i;
        pcbEntry[i].parentProcessId = -1;
        pcbEntry[i].program = initProgram;
        pcbEntry[i].programCounter = 0;
        pcbEntry[i].value = 0;
        pcbEntry[i].priority = i % 3;
        pcbEntry[i].state = STATE_READY;
        pcbEntry[i].startTime = 0;
        pcbEntry[i].timeUsed = 0;

        addElement(readyState, getSize(readyState), i);
    }

    while (timestamp < 100) {
        quantum();
        unblock();
        print();
        delay(1.0);
    }

    avgTTime();

    return 0;
}
