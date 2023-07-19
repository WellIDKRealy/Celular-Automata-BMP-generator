#include <stdio.h>
#include <stdlib.h>
#include<stdint.h>

int main() {
  FILE* fptr = fopen("test.bmp", "w");
  int32_t test = 15;
  fwrite(&test, 4, 1, fptr);
  fclose(fptr);
}
