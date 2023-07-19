#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "simdimacs.h"

void simdimacs_add(void* userdata, int lit) {
  (void)userdata;
  printf(" %d", lit);
  if(lit == 0)
    printf("\n");
}

int main(int argc, char* argv[]) {
  if(argc != 2) {
    fprintf(stderr, "Error: Provide file to read!\n");
    return EXIT_FAILURE;
  }
  const char* path = argv[1];
  FILE* file = fopen(path, "r");
  const char* err = simdimacs_parse(file, NULL);
  if(err) {
    fprintf(stderr, "Parse error: %s\n", err);
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
