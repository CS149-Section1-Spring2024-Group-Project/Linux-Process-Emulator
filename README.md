# Linux-Process-Emulator

> #### Contributers
>
> - Andy Nguyen
> - Yeng Her

>  Last Modified: April 11, 2024 8:06 PM PST





My name is Yeng Her and I worked on the simulation Process

Simulated Process Process management simulation manages the execution of simulated processes. Each simulated process is comprised of a program that manipulates (sets/updates) the value of a single integer variable. Thus the state of a simulated process at any instant is comprised of the value of its integer variable and the value of its program counter. A simulated process’ program consists of a sequence of instructions. There are seven types of instructions as follows:

S n: Set the value of the integer variable to n, where n is an integer.
A n: Add n to the value of the integer variable, where n is an integer.
D n: Subtract n from the value of the integer variable, where n is an integer. © Copyright 2023 Pearson Education, Inc. All Rights Reserved.
B: Block this simulated process.
E: Terminate this simulated process.
F n: Create a new simulated process. The new (simulated) process is an exact copy of the parent (simulated) process. The new (simulated) process executes from the instruction immediately after this (F) instruction, while the parent (simulated) process continues its execution n instructions after the next instruction.
R filename: Replace the program of the simulated process with the program in the file filename, and set program counter to the first instruction of this new program. An example of a program for a simulated is as follows: S 1000 A 19 A 20 D 53 A 55 F 1 R file_a F 1 R file_b F 1 R file_c F 1 R file_d F 1 R file_e E You may store the program of a simulated process in an array, with one array entry for each instruction. Process Manager Process The process manager process simulates five process management functions: creation of new (simulated) processes, replacing the current process image of a simulated process with a new process image, management of process state transitions, process scheduling, and context switching. In addition, it spawns a reporter process whenever it needs to print out the state of the system. The process manager creates the first simulated process (process id = 0). Program for this process is read from a file (filename: init). This is the only simulated process created by the process manager on its own. All other simulated processes are created in response to the execution of the F instruction. Process manager: Data structures © Copyright 2023 Pearson Education, Inc. All Rights Reserved. The process manager maintains six data structures: Time, Cpu, PcbTable, ReadyState, BlockedState, and RunningState. Time is an integer variable initialized to zero. Cpu is used to simulate the execution of a simulated process that is in running state. It should include data members to store a pointer to the program array, current program counter value, integer value, and time slice of that simulated process. In addition, it should store the number of time units used so far in the current time slice. PcbTable is an array with one entry for every simulated process that hasn't finished its execution yet. Each entry should include data members to store process id, parent process id, a pointer to program counter value (initially 0), integer value, priority, state, start time, and CPU time used so far. ReadyState stores all simulated processes (PcbTable indices) that are ready to run. This can be implemented using a queue or priority queue data structure. BlockedState stores all processes (PcbTable indices) that are currently blocked. This can be implemented using a queue data structure. Finally, RunningState stores the PcbTable index of the currently running simulated process. 

