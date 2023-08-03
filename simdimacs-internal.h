#ifndef SIMDIMACS_INTERNAL_H
#define SIMDIMACS_INTERNAL_H

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

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <immintrin.h>

typedef enum simdimacs_conversion {
  Empty = 0,
  SSE1Digit,
  SSE2Digits,
  SSE3Digits,
  SSE4Digits,
  SSE8Digits,
  Scalar
} simdimacs_conversion;

typedef struct simdimacs_blockinfo {
  uint8_t first_skip;
  uint8_t total_skip;
  uint8_t element_count;
  bool trailing_op : 1;
  simdimacs_conversion conversion_routine : 7;
  uint16_t invalid_sign_mask;
  uint8_t shuffle_digits[16];
  uint8_t shuffle_signs[16];

} simdimacs_blockinfo;

void
simdimacs_dump(FILE* file, simdimacs_blockinfo* b);

extern simdimacs_blockinfo simdimacs_blocks[65536];

#endif
