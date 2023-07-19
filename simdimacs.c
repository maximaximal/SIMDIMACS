#include <stdio.h>

#include "simdimacs.h"
#include "simdimacs-internal.h"

const char* simdimacs_parse(FILE* f, void* userdata) {
  if(simdimacs_blocks[100].element_count > 0) {
    return ">0";
  }
  return "0";
}
