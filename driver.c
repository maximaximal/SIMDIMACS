#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "simdimacs.h"

void
simdimacs_add(void* userdata, int lit) {
  (void)userdata;
  static bool first = true;
  if(first) {
    printf("%d", lit);
    first = false;
  } else {
    printf(" %d", lit);
  }
  if(lit == 0) {
    printf("\n");
    first = true;
  }
}

void
simdimacs_problem(void* userdata, int vars, int clauses) {
  (void)userdata;
  printf("p cnf %d %d\n", vars, clauses);
}

int
main(int argc, char* argv[]) {
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
