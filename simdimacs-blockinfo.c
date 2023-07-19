#include "simdimacs-internal.h"

/*
Copyright (c) 2018, Wojciech Muła
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "simdimacs-blockinfo.inc.c"

static void
as_array(FILE* f, const uint8_t data[16]) {
  fprintf(f, "{");
  fprintf(f, "%02x", data[0]);
  for(int i = 1; i < 16; i++)
    fprintf(f, ", %02x", data[i]);
  fprintf(f, "}\n");
}

static const char*
to_string(simdimacs_conversion c) {
  switch(c) {
    case Scalar:
      return "Scalar";

    case SSE1Digit:
      return "SSE1Digit";

    case SSE2Digits:
      return "SSE2Digits";

    case SSE4Digits:
      return "SSE4Digits";

    case SSE8Digits:
      return "SSE8Digits";

    default:
      return "<unknown>";
  }
}

void
simdimacs_dump(FILE* f, simdimacs_blockinfo *b) {
  fprintf(f, "first_skip          : %d\n", b->first_skip);
  fprintf(f, "total_skip          : %d\n", b->total_skip);
  fprintf(f, "element_count       : %d\n", b->element_count);
  fprintf(f, "conversion          : %s\n", to_string(b->conversion_routine));
  fprintf(f, "invalid_sign_mask   : %04x\n", b->invalid_sign_mask);
  fprintf(f, "shuffle_digits      : ");
  as_array(f, b->shuffle_digits);
  fprintf(f, "shuffle_signs       : ");
  as_array(f, b->shuffle_signs);
}
