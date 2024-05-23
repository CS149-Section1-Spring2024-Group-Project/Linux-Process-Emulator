#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>

enum State {
    STATE_RUNNING,
    STATE_READY,
    STATE_BLOCKED,
    STATE_TERMINATED
};

struct Instruction {
    char operation;
    char stringArg[100];
    int intArg;
};

struct PcbEntry {
    struct Instruction program[100];
    int processId;
    int parentProcessId;
    int programCounter;
    int value;
    int priority;
    enum State state;
    unsigned int startTime;
    unsigned int timeUsed;
};

struct Cpu {
    struct Instruction *pProgram;
    int programCounter;
    int value;
};

void trim(char *str) {
    char *end;

    while (isspace((unsigned char)*str)) str++;

    if (*str == 0) return;

    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    *(end + 1) = 0;
}

struct PcbEntry pcbEntry[10];
unsigned int timestamp = 0;
struct Cpu cpu;

int runningState = -1;
int readyState[10];
int readyStateSize = 0;
int blockedState[10];
int blockedStateSize = 0;

double cumulativeTimeDiff = 0;
int numTerminatedProcesses = 0;

bool createProgram(const char *filename, struct Instruction *program) {
    FILE *file;
    int lineNum = 0;
    file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file %s\n", filename);
        return false;
    }
    char line[100];
    while (fgets(line, sizeof(line), file)) {
        trim(line);
        if (line[0] != '\0') {
            struct Instruction instruction;
            instruction.operation = toupper(line[0]);
            strcpy(instruction.stringArg, line + 1);
            sscanf(instruction.stringArg, "%d", &instruction.intArg);
            switch (instruction.operation) {
                case 'S':
                case 'A':
                case 'D':
                case 'F':
                    break;
                case 'B':
                case 'E':
                    break;
                case 'R':
                    if (instruction.stringArg[0] == '\0') {
                        printf("%s:%d - Missing string argument\n", filename, lineNum);
                        fclose(file);
                        return false;
                    }
                    break;
                default:
                    printf("%s:%d - Invalid operation, %c\n", filename, lineNum, instruction.operation);
                    fclose(file);
                    return false;
            }
            program[lineNum] = instruction;
        }
        lineNum++;
    }
    fclose(file);
    return true;
}

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
    if (runningState != -1) return;

    if (readyStateSize > 0) {
        int newProcess = readyState[0];
        for (int i = 1; i < readyStateSize; ++i) {
            readyState[i - 1] = readyState[i];
        }
        readyStateSize--;
        runningState = newProcess;
        cpu.pProgram = pcbEntry[newProcess].program;
        cpu.programCounter = pcbEntry[newProcess].programCounter;
        cpu.value = pcbEntry[newProcess].value;
    }
}

void block() {
    if (runningState == -1) return;

    blockedState[blockedStateSize++] = runningState;
    pcbEntry[runningState].state = STATE_BLOCKED;
    pcbEntry[runningState].programCounter = cpu.programCounter;
    pcbEntry[runningState].value = cpu.value;

    runningState = -1;
}

void end() {
    if (runningState == -1) return;

    struct PcbEntry *runningProcess = &pcbEntry[runningState];
    cumulativeTimeDiff += timestamp + 1 - runningProcess->startTime;
    ++numTerminatedProcesses;
    runningState = -1;
}

void custom_fork(int value) {
    int newProcessId = readyStateSize;
    struct PcbEntry *parentProcess = &pcbEntry[runningState];
    if (value < 0 || value >= 100) return;

    pcbEntry[newProcessId].processId = newProcessId;
    pcbEntry[newProcessId].parentProcessId = runningState;
    pcbEntry[newProcessId].programCounter = cpu.programCounter;
    pcbEntry[newProcessId].value = cpu.value;
    pcbEntry[newProcessId].priority = parentProcess->priority;
    pcbEntry[newProcessId].state = STATE_READY;
    pcbEntry[newProcessId].startTime = timestamp;

    readyState[readyStateSize++] = newProcessId;
    cpu.programCounter += value;
}

void replace(const char *argument, struct Instruction *pProgram) {
    memset(pProgram, 0, 100 * sizeof(struct Instruction));
    if (!createProgram(argument, pProgram)) {
        printf("Failed to load program file: %s\n", argument);
        ++cpu.programCounter;
        return;
    }
    cpu.programCounter = 0;
}

void quantum() {
    struct Instruction instruction;
    printf("In quantum\n");
    if (runningState == -1) {
        printf("No processes are running\n");
        ++timestamp;
        return;
    }
    if (cpu.programCounter < 100 && cpu.pProgram[cpu.programCounter].operation != '\0') {
        instruction = cpu.pProgram[cpu.programCounter++];
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
            custom_fork(instruction.intArg);
            break;
        case 'R':
            replace(instruction.stringArg, cpu.pProgram);
            break;
    }
    ++timestamp;
    schedule();
}

void unblock() {
    if (blockedStateSize > 0) {
        int unblockedProcess = blockedState[0];
        for (int i = 1; i < blockedStateSize; ++i) {
            blockedState[i - 1] = blockedState[i];
        }
        blockedStateSize--;
        readyState[readyStateSize++] = unblockedProcess;
        pcbEntry[unblockedProcess].state = STATE_READY;
    }
    schedule();
}

int runProcessManager(int fileDescriptor) {
    if (!createProgram("init", pcbEntry[0].program)) {
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
    double avgTurnaroundTime = 0;

    char ch;
    do {
        if (read(fileDescriptor, &ch, sizeof(ch)) != sizeof(ch)) {
            break;
        }
        switch (ch) {
            case 'Q':
                quantum();
                break;
            case 'U':
                unblock();
                break;
            case 'P':
                //print();
                break;
            default:
                printf("You entered an invalid character!\n");
        }
    } while (ch != 'T');
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    int pipeDescriptors[2];
    pid_t processMgrPid;
    char ch;
    int result;
    pipe(pipeDescriptors);
    if ((processMgrPid = fork()) == -1) exit(1); /* FORK FAILED */
    if (processMgrPid == 0) {
        close(pipeDescriptors[1]);
        result = runProcessManager(pipeDescriptors[0]);
        close(pipeDescriptors[0]);
        _exit(result);
    } else {
        close(pipeDescriptors[0]);
        do {
            printf("Enter Q, P, U or T\n");
            printf("$ ");
            scanf(" %c", &ch);
            if (write(pipeDescriptors[1], &ch, sizeof(ch)) != sizeof(ch)) {
                break;
            }
        } while (ch != 'T');
        write(pipeDescriptors[1], &ch, sizeof(ch));
        close(pipeDescriptors[1]);
        wait(&result);
    }
    return result;
}