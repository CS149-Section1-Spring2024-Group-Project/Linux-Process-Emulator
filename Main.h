#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char operation;
    int intArg;
    char stringArg[1000];
} Instruction;

typedef struct {
    Instruction pProgram[0];
    int programCounter;
    int value;
    int timeSlice;
    int timeSliceUsed;
} Cpu;

typedef enum {
    STATE_READY,
    STATE_RUNNING,
    STATE_BLOCKED
} State;

typedef struct {
    int processId;
    int parentProcessId;
    Instruction program[0];
    unsigned int programCounter;
    int value;
    unsigned int priority;
    State state;
    unsigned int startTime;
    unsigned int timeUsed;
} PcbEntry;

PcbEntry pcbTable[0];
PcbEntry pcbEntry[10];
double avgTurnaroundTime;
Cpu cpu;

int runningState = -1;
int readyState[0];
int blockedState[0];
int pcbIndex = 0;