#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "simdimacs.h"

static bool FILE_based = false;
static bool quiet = false;

void
simdimacs_add(void* userdata, int lit) {
  (void)userdata;
  if(quiet)
    return;
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
  if(quiet)
    return;
  printf("p cnf %d %d\n", vars, clauses);
}

static void
help() {
  printf("simdimacs driver - drives the simdimacs accelerated dimacs parser\n");
  printf("OPTIONS:\n");
  printf("  -F\tuse FILE* based parsing (synchronous)\n");
  printf("  -q\tdon't print the same CNF as output (quiet mode)\n");
  printf("  -h\tthis help message\n");
}

int
main(int argc, char* argv[]) {
  int opt;
  while((opt = getopt(argc, argv, "Fqh")) != -1) {
    switch(opt) {
      case 'F':
        FILE_based = true;
        break;
      case 'q':
        quiet = true;
        break;
      case 'h':
        help();
        return EXIT_SUCCESS;
    }
  }

  if(optind >= argc) {
    fprintf(stderr, "Error: Expected file to read after options!\n");
    return EXIT_FAILURE;
  }

  const char* path = argv[optind];
  const char* err = NULL;

  if(FILE_based) {
    FILE* file = fopen(path, "r");
    err = simdimacs_parse(file, NULL);
    fclose(file);
  } else {
    err = simdimacs_parse_path(path, NULL);
  }
  if(err) {
    fprintf(stderr, "Parse error: %s\n", err);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
