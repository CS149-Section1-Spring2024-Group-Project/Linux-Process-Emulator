#include <stdio.h>

#define MAX_STATEMENTS 100 // Define the maximum number of statements

struct statements {
  char type[100];
  int number;
};

int main() {
  FILE *ptr = fopen("abc.txt", "r");
  if (ptr == NULL) {
    printf("no such file.");
    return 1;
  }

  // Array to store instruction statements
  struct statements instructionStatementsArr[MAX_STATEMENTS];
  int count = 0;

  // Read data line by line using fscanf and store in the array
  while (count < MAX_STATEMENTS &&
         fscanf(ptr, "%s %d", instructionStatementsArr[count].type,
                &instructionStatementsArr[count].number) == 2) {
    count++;
  }

  fclose(ptr);

  // Printing the values from the array
  for (int i = 0; i < count; i++) {
    printf("%s %d\n", instructionStatementsArr[i].type,
           instructionStatementsArr[i].number);
  }

  return 0;
}
