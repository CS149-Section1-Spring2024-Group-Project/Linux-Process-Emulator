#include <ctype.h>
#include <stdio.h>
#include <string.h> // Include string.h library for string operations

#define MAX_STATEMENTS 100 // Define the maximum number of statements

struct statements {
  char type[100];
  char stringValue[255]; // Change type of stringValue to char array for storing
                         // strings
  int number;
};

struct statements instructionStatementsArr[MAX_STATEMENTS];
int count = 0;

char *trim(char *str) {
  while (isspace((unsigned char)*str))
    str++;
  if (*str == 0)
    return str;
  char *end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end))
    end--;
  end[1] = '\0';
  return str;
}

int readInstructions(char fileName[]) {
  FILE *ptr = fopen(fileName, "r");
  if (ptr == NULL) {
    printf("no such file. %s\n", fileName);
    return 1;
  }

  // Array to store instruction statements
  count = 0;

  // Read data line by line using fscanf and store in the array

  while (count < MAX_STATEMENTS &&
         fscanf(ptr, "%s", instructionStatementsArr[count].type) == 1) {
    if (strcmp(instructionStatementsArr[count].type, "R") == 0) {
      fgets(instructionStatementsArr[count].stringValue,
            sizeof(instructionStatementsArr[count].stringValue), ptr);
      instructionStatementsArr[count].stringValue[strcspn(
          instructionStatementsArr[count].stringValue, "\n")] =
          0; // Remove newline character if present
    }

    else {
      fscanf(ptr, "%d", &instructionStatementsArr[count].number);
    }
    // printf("%s\n", instructionStatementsArr[count].type);

    switch (instructionStatementsArr[count].type[0]) {

    case 'S':
      // set(instruction.intArg);
      printf("instruction S \n");
      break;
    case 'A':
      // add(instruction.intArg);
      printf("instruction A \n");
      break;
    case 'D':
      // decrement(instruction.intArg);
      printf("instruction D \n");
      break;
    case 'B':
      printf("instruction B \n");
      // block();
      break;
    case 'E':

      printf("instruction E \n");
      // end();
      break;
    case 'F':

      printf("instruction F \n");
      // fork(instruction.intArg);
      break;

    case 'R':
      printf("Handling type R instruction:%s\n",
             instructionStatementsArr[count].stringValue);

      char argv[100]; // Assuming the maximum length of the file name is 100
      sprintf(argv, "%s.txt", instructionStatementsArr[count].stringValue);
      // Read the instructions from the new file only once without a recursive
      // call

      char trimmedFileName[100];
      strcpy(trimmedFileName, trim(argv));
      readInstructions(trimmedFileName);
      printf("This is the value of file name:%s\n", trimmedFileName);

      // readInstructions(argv);

      break;
    case 'T':
      printf("Handling type T instruction: %d\n",
             instructionStatementsArr[count].number);
      // Add logic for handling type T instruction
      break;
    default:
      printf("Unknown instruction type\n");
      break;
    }

    count++;
  }

  fclose(ptr);

  return 0;
}

int main() {

  char fileName[] = "abc.txt";

  readInstructions(fileName);
  // Printing the values from the array

  for (int i = 0; i < count; i++) {
    if (strcmp(instructionStatementsArr[i].type, "R") == 0) {
      printf("%s %s\n", instructionStatementsArr[i].type,
             instructionStatementsArr[i].stringValue);
    } else {
      printf("%s %d\n", instructionStatementsArr[i].type,
             instructionStatementsArr[i].number);
    }
  }

  // Switch case to handle different types of instructions

  return 0;
}
