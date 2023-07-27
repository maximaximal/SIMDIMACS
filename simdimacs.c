#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "simdimacs-internal.h"
#include "simdimacs.h"

#include <emmintrin.h>
#include <immintrin.h>

#ifdef SIMDIMACS_STATS
#define xstr(s) str(s)
#define str(s) #s

static size_t simdimacs_callsto_convert_1_digit = 0;
static size_t simdimacs_callsto_convert_2_digits = 0;
static size_t simdimacs_callsto_convert_3_digits = 0;
static size_t simdimacs_callsto_convert_4_digits = 0;
static size_t simdimacs_callsto_convert_8_digits = 0;

static size_t simdimacs_callsto_convert_2_digits_signed = 0;
static size_t simdimacs_callsto_convert_3_digits_signed = 0;
static size_t simdimacs_callsto_convert_4_digits_signed = 0;
static size_t simdimacs_callsto_convert_8_digits_signed = 0;

#define P(VAL) fprintf(stderr, str(VAL) ": %lu\n", simdimacs_##VAL)

void
simdimacs_print_stats() {
  P(callsto_convert_1_digit);
  P(callsto_convert_2_digits);
  P(callsto_convert_3_digits);
  P(callsto_convert_4_digits);
  P(callsto_convert_8_digits);

  P(callsto_convert_2_digits_signed);
  P(callsto_convert_3_digits_signed);
  P(callsto_convert_4_digits_signed);
  P(callsto_convert_8_digits_signed);
}

#undef P

#undef xstr
#undef str

#define INCSTAT(VAL) ++simdimacs_##VAL

#else

#define INCSTAT(VAL)

#endif

#define SSE_ALIGN __attribute__((aligned(16)))

static __m128i
decimal_digits_mask(const __m128i input) {
  const __m128i ascii0 = _mm_set1_epi8('0');
  const __m128i after_ascii9 = _mm_set1_epi8('9' + 1);

  const __m128i t0 = _mm_cmplt_epi8(input, ascii0);      // t1 = (x < '0')
  const __m128i t1 = _mm_cmplt_epi8(input, after_ascii9);// t0 = (x <= '9')

  return _mm_andnot_si128(t0, t1);// x <= '9' and x >= '0'
}

static inline void
convert_1digit(void* userdata, const __m128i input, int count) {
  INCSTAT(callsto_convert_1_digit);
  const __m128i ascii0 = _mm_set1_epi8('0');

  const __m128i t0 = _mm_subs_epu8(input, ascii0);

  uint8_t tmp[16] SSE_ALIGN;

  _mm_store_si128((__m128i*)tmp, t0);
  for(int i = 0; i < count; i++)
    SIMDIMACS_ADD(userdata, tmp[i]);
}

static inline void
convert_2digits(void* userdata, const __m128i input, int count) {
  INCSTAT(callsto_convert_2_digits);
  const __m128i ascii0 = _mm_set1_epi8('0');
  const __m128i mul_1_10 =
    _mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);

  const __m128i t0 = _mm_subs_epu8(input, ascii0);
  const __m128i t1 = _mm_maddubs_epi16(t0, mul_1_10);

  uint16_t tmp[8] SSE_ALIGN;

  _mm_store_si128((__m128i*)tmp, t1);
  for(int i = 0; i < count; i++)
    SIMDIMACS_ADD(userdata, tmp[i]);
}

static inline void
convert_3digits(void* userdata, const __m128i input, int count) {
  INCSTAT(callsto_convert_3_digits);

  const __m128i ascii0 = _mm_set1_epi8('0');
  const __m128i mul_all =
    _mm_setr_epi8(0, 100, 10, 1, 0, 100, 10, 1, 0, 100, 10, 1, 0, 100, 10, 1);

  // =--------------

  const __m128i t0 = _mm_subs_epu8(input, ascii0);
  const __m128i t1 = _mm_maddubs_epi16(t0, mul_all);
  const __m128i t2 = _mm_hadd_epi16(t1, t1);

  uint16_t tmp[8] SSE_ALIGN;

  _mm_store_si128((__m128i*)tmp, t2);
  for(int i = 0; i < count; i++)
    SIMDIMACS_ADD(userdata, tmp[i]);
}

static inline void
convert_4digits(void* userdata, const __m128i input, int count) {
  INCSTAT(callsto_convert_4_digits);

  const __m128i ascii0 = _mm_set1_epi8('0');
  const __m128i mul_1_10 =
    _mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);
  const __m128i mul_1_100 = _mm_setr_epi16(100, 1, 100, 1, 100, 1, 100, 1);

  // =--------------

  const __m128i t0 = _mm_subs_epu8(input, ascii0);
  const __m128i t1 = _mm_maddubs_epi16(t0, mul_1_10);
  const __m128i t2 = _mm_madd_epi16(t1, mul_1_100);

  uint32_t tmp[4] SSE_ALIGN;

  _mm_store_si128((__m128i*)tmp, t2);
  for(int i = 0; i < count; i++)
    SIMDIMACS_ADD(userdata, tmp[i]);
}

static inline void
convert_8digits(void* userdata, const __m128i input, int count) {
  INCSTAT(callsto_convert_8_digits);

  const __m128i ascii0 = _mm_set1_epi8('0');
  const __m128i mul_1_10 =
    _mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);
  const __m128i mul_1_100 = _mm_setr_epi16(100, 1, 100, 1, 100, 1, 100, 1);
  const __m128i mul_1_10000 =
    _mm_setr_epi16(10000, 1, 10000, 1, 10000, 1, 10000, 1);

  // =--------------

  const __m128i t0 = _mm_subs_epu8(input, ascii0);
  const __m128i t1 = _mm_maddubs_epi16(t0, mul_1_10);
  const __m128i t2 = _mm_madd_epi16(t1, mul_1_100);
  const __m128i t3 = _mm_packus_epi32(t2, t2);
  const __m128i t4 = _mm_madd_epi16(t3, mul_1_10000);

  uint32_t tmp[4] SSE_ALIGN;

  _mm_store_si128((__m128i*)tmp, t4);
  for(int i = 0; i < count; i++)
    SIMDIMACS_ADD(userdata, tmp[i]);
}

static inline void
convert_2digits_signed(void* userdata,
                       const __m128i input,
                       const __m128i negate_mask,
                       int count) {
  INCSTAT(callsto_convert_2_digits_signed);
  const __m128i ascii0 = _mm_set1_epi8('0');
  const __m128i mul_1_10 =
    _mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);

  const __m128i t0 = _mm_subs_epu8(input, ascii0);
  const __m128i t1 = _mm_maddubs_epi16(t0, mul_1_10);

  const __m128i s0 = _mm_xor_si128(t1, negate_mask);
  const __m128i s1 = _mm_sub_epi16(s0, negate_mask);

  int16_t tmp[8] SSE_ALIGN;

  _mm_store_si128((__m128i*)tmp, s1);
  for(int i = 0; i < count; i++)
    SIMDIMACS_ADD(userdata, tmp[i]);
}

static inline void
convert_3digits_signed(void* userdata,
                       const __m128i input,
                       const __m128i negate_mask,
                       int count) {
  INCSTAT(callsto_convert_3_digits_signed);

  const __m128i ascii0 = _mm_set1_epi8('0');
  const __m128i mul_all =
    _mm_setr_epi8(0, 100, 10, 1, 0, 100, 10, 1, 0, 100, 10, 1, 0, 100, 10, 1);

  const __m128i s0 = _mm_xor_si128(mul_all, negate_mask);
  const __m128i s1 = _mm_sub_epi8(s0, negate_mask);

  // =--------------

  const __m128i t0 = _mm_subs_epu8(input, ascii0);
  const __m128i t1 = _mm_maddubs_epi16(t0, s1);
  const __m128i t2 = _mm_hadd_epi16(t1, t1);

  int16_t tmp[8] SSE_ALIGN;

  _mm_store_si128((__m128i*)tmp, t2);
  for(int i = 0; i < count; i++)
    SIMDIMACS_ADD(userdata, tmp[i]);
}

static inline void
convert_4digits_signed(void* userdata,
                       const __m128i input,
                       const __m128i negate_mask,
                       int count) {
  INCSTAT(callsto_convert_4_digits_signed);

  const __m128i ascii0 = _mm_set1_epi8('0');
  const __m128i mul_1_10 =
    _mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);
  const __m128i mul_1_100 = _mm_setr_epi16(100, 1, 100, 1, 100, 1, 100, 1);

  // =--------------

  const __m128i t0 = _mm_subs_epu8(input, ascii0);
  const __m128i t1 = _mm_maddubs_epi16(t0, mul_1_10);
  const __m128i t2 = _mm_madd_epi16(t1, mul_1_100);

  const __m128i s0 = _mm_xor_si128(t2, negate_mask);
  const __m128i s1 = _mm_sub_epi32(s0, negate_mask);

  int32_t tmp[4] SSE_ALIGN;

  _mm_store_si128((__m128i*)tmp, s1);
  for(int i = 0; i < count; i++)
    SIMDIMACS_ADD(userdata, tmp[i]);
}

static inline void
convert_8digits_signed(void* userdata,
                       const __m128i input,
                       const __m128i negate_mask,
                       int count) {
  INCSTAT(callsto_convert_8_digits_signed);

  const __m128i ascii0 = _mm_set1_epi8('0');
  const __m128i mul_1_10 =
    _mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1);
  const __m128i mul_1_100 = _mm_setr_epi16(100, 1, 100, 1, 100, 1, 100, 1);
  const __m128i mul_1_10000 =
    _mm_setr_epi16(10000, 1, 10000, 1, 10000, 1, 10000, 1);

  // =--------------

  const __m128i t0 = _mm_subs_epu8(input, ascii0);
  const __m128i t1 = _mm_maddubs_epi16(t0, mul_1_10);
  const __m128i t2 = _mm_madd_epi16(t1, mul_1_100);

  const __m128i s0 = _mm_xor_si128(t2, negate_mask);
  const __m128i s1 = _mm_sub_epi32(s0, negate_mask);

  const __m128i t3 = _mm_packs_epi32(s1, s1);
  const __m128i t4 = _mm_madd_epi16(t3, mul_1_10000);

  int32_t tmp[4] SSE_ALIGN;

  _mm_store_si128((__m128i*)tmp, t4);
  for(int i = 0; i < count; i++)
    SIMDIMACS_ADD(userdata, tmp[i]);
}

static inline void
mul10_add_digit(uint32_t* number, char c) {
  assert(number);
  // number * 10 + 9 <= MAX  <=>  number <= (MAX - 9)/10
  if(*number < (INT_MAX - 9) / 10) {
    // no overflow is possible, use the faster path
    *number = 10 * (*number) + c - '0';
  } else {
    // check for overflow
    if(__builtin_umul_overflow(*number, 10, number)) {
      exit(1);
    }

    if(__builtin_uadd_overflow(*number, c - '0', number)) {
      exit(1);
    }
  }
}

static inline const char*
parse_unsigned(void* userdata,
               const simdimacs_blockinfo* bi,
               const __m128i input,
               const char* data,
               const char* end) {
  const __m128i shuffle_digits =
    _mm_loadu_si128((const __m128i*)bi->shuffle_digits);
  const __m128i shuffled = _mm_shuffle_epi8(input, shuffle_digits);

  if(bi->conversion_routine == SSE1Digit) {
    convert_1digit(userdata, shuffled, bi->element_count);
  } else if(bi->conversion_routine == SSE2Digits) {
    convert_2digits(userdata, shuffled, bi->element_count);
  } else if(bi->conversion_routine == SSE3Digits) {
    convert_3digits(userdata, shuffled, bi->element_count);
  } else if(bi->conversion_routine == SSE4Digits) {
    convert_4digits(userdata, shuffled, bi->element_count);
  } else if(bi->conversion_routine == SSE8Digits) {
    convert_8digits(userdata, shuffled, bi->element_count);
  } else {
    uint32_t result = 0;
    bool converted = false;

    data += bi->first_skip;
    while(data < end && *data >= '0' && *data <= '9') {
      mul10_add_digit(&result, *data);
      data += 1;
      converted = true;
    }

    if(converted) {
      if(result > INT_MAX) {
        exit(1);
      }
      SIMDIMACS_ADD(userdata, result);
    }

    return data;
  }

  return data + bi->total_skip;
}

static inline const char*
parse_signed(void* userdata,
             const simdimacs_blockinfo* bi,
             const __m128i input,
             const char* data,
             const char* end) {
  const __m128i ascii_minus = _mm_set1_epi8('-');

  const __m128i shuffle_digits =
    _mm_loadu_si128((const __m128i*)bi->shuffle_digits);
  const __m128i shuffle_signs =
    _mm_loadu_si128((const __m128i*)bi->shuffle_signs);

  // Note: there is not need to mask '+' or '-' in the input prior
  // shuffling. This is possible because ASCII codes of '+' and '-'
  // are smaller than '0' (43 < 48 && 45 < 48). These character will
  // be zeroed during subtraction of '0'.
  const __m128i shuffled = _mm_shuffle_epi8(input, shuffle_digits);
  const __m128i shuffled_signs = _mm_shuffle_epi8(input, shuffle_signs);
  const __m128i negate_mask = _mm_cmpeq_epi8(shuffled_signs, ascii_minus);

  if(bi->conversion_routine == SSE1Digit) {
    convert_1digit(userdata, shuffled, bi->element_count);
  } else if(bi->conversion_routine == SSE2Digits) {
    convert_2digits_signed(userdata, shuffled, negate_mask, bi->element_count);
  } else if(bi->conversion_routine == SSE3Digits) {
    convert_3digits_signed(userdata, shuffled, negate_mask, bi->element_count);
  } else if(bi->conversion_routine == SSE4Digits) {
    convert_4digits_signed(userdata, shuffled, negate_mask, bi->element_count);
  } else if(bi->conversion_routine == SSE8Digits) {
    convert_8digits_signed(userdata, shuffled, negate_mask, bi->element_count);
  } else {
    bool converted = false;
    uint32_t result;
    bool negative;

    data += bi->first_skip;

    if(*data == '-') {
      data++;
      negative = true;
      result = 0;
    } else {
      result = *data++ - '0';
      negative = false;
      converted = true;
    }

    while(data < end && *data >= '0' && *data <= '9') {
      mul10_add_digit(&result, *data);
      data += 1;
      converted = true;
    }

    if(converted) {
      if(negative) {
        const int64_t tmp = INT_MAX;
        const uint32_t absmin = -tmp;
        if(result > absmin) {
          exit(1);
        }
        SIMDIMACS_ADD(userdata, -result);
      } else {
        const uint32_t max = INT_MAX;
        if(result > max) {
          exit(1);
        }
        SIMDIMACS_ADD(userdata, result);
      }
    }

    return data;
  }

  return data + bi->total_skip;
}

// Initiated by sse_init
static size_t separator_set_size;

// Initiated by sse_init
static __m128i separator_set;

#define GET_MASK(INPUT, INITIAL)     \
  _mm_or_si128(                      \
    INITIAL,                         \
    _mm_cmpestrm(separator_set,      \
                 separator_set_size, \
                 INPUT,              \
                 16,                 \
                 _SIDD_UBYTE_OPS | _SIDD_CMP_EQUAL_ANY | _SIDD_UNIT_MASK))

static const char*
process_chunk(void* userdata,
              const char* data,
              const char* end,
              const char** error) {
  assert(end - data >= 16);

  // Load a chunk into memory
  __m128i input = _mm_loadu_si128((__m128i*)data);

  const __m128i ascii_minus = _mm_set1_epi8('-');
  const __m128i bytemask_digit = decimal_digits_mask(input);

  const __m128i bytemask_minus = _mm_cmpeq_epi8(input, ascii_minus);
  const __m128i bytemask_span = _mm_or_si128(bytemask_digit, bytemask_minus);

  const uint16_t span_mask = _mm_movemask_epi8(bytemask_span);
  const uint16_t sign_mask = _mm_movemask_epi8(bytemask_minus);

  simdimacs_blockinfo* bi = &simdimacs_blocks[span_mask];
  if(sign_mask & bi->invalid_sign_mask) {
    *error = "character '-' at invalid position!";
    return NULL;
  }

  if(span_mask == 0) {
    return data + 16;
  }

  if(sign_mask == 0 || bi->conversion_routine == SSE1Digit) {
    return parse_unsigned(userdata, bi, input, data, end);
  } else {
    return parse_signed(userdata, bi, input, data, end);
  }
}

static void
sse_init() {
  separator_set_size = 2;
  // All the separators have to be listed, otherwise this is a buffer overflow!
  separator_set = _mm_loadu_si128((__m128i*)" \n                          ");
}

#define NEXT_C  \
  ++*c;         \
  if(*c == end) \
  return "end of file reached early"

static const char*
parse_header(const char** c, const char* end, void* userdata) {
  if(**c != 'p')
    return "first char not 'p'";
  NEXT_C;
  if(**c != ' ')
    return "second char not ' '";
  NEXT_C;
  if(**c != 'c')
    return "third char not 'c'";
  NEXT_C;
  if(**c != 'n')
    return "fourth char not 'n'";
  NEXT_C;
  if(**c != 'f')
    return "fifth char not 'f'";
  NEXT_C;
  if(**c != ' ')
    return "sixth char not ' '";
  NEXT_C;

  uint32_t variables = 0;
  uint32_t clauses = 0;

  if(!(**c >= '0' && **c <= '9'))
    return "no digit encountered for first char of variables in problem line";
  while(**c >= '0' && **c <= '9') {
    mul10_add_digit(&variables, **c);
    NEXT_C;
  }

  if(**c != ' ')
    return "variables and claues must be separated by ' ' in problem line";

  NEXT_C;

  if(!(**c >= '0' && **c <= '9'))
    return "no digit encountered for first char of clauses in problem line";
  while(**c >= '0' && **c <= '9') {
    mul10_add_digit(&clauses, **c);
    NEXT_C;
  }

  if(**c != '\n')
    return "problem line must end with a '\n'";

  if(variables > INT_MAX)
    return "variables overflow";
  if(clauses > INT_MAX)
    return "clauses overflow";

  SIMDIMACS_PROBLEM(userdata, variables, clauses);

  return NULL;
}

static inline const char*
scalar_parse_signed(void* userdata, const char* data, size_t size) {

  typedef enum state { separator, minus, digit } state;

  state s = separator;
  state prev = separator;
  bool negative = false;
  uint32_t number = 0;

  for(size_t i = 0; i < size; i++) {
    const char c = data[i];
    if(c == '-') {
      s = minus;
    } else if(c >= '0' && c <= '9') {
      s = digit;
    } else if(c == ' ' || c == '\n') {
      s = separator;
    } else {
      return "wrong character (scalar)";
    }

    switch(s) {
      case minus:
        if(prev != separator) {
          return "invalid syntax ('-' follows a non-separator character)";
        }
        number = 0;
        negative = true;
        break;

      case digit:
        if(prev == separator) {
          number = c - '0';
          negative = false;
        } else {
          mul10_add_digit(&number, c);
        }
        break;

      case separator:
        if(prev == digit) {
          if(negative) {
            const int64_t tmp = INT_MAX;
            const uint32_t absmin = -tmp;
            if(number > absmin) {
              return "signed overflow";
            }
            SIMDIMACS_ADD(userdata, -number);
          } else {
            if(number > INT_MAX) {
              return "signed overflow";
            }

            SIMDIMACS_ADD(userdata, number);
          }
        } else if(prev != separator) {
          return "Invalid syntax ('-' or '+' not followed by any digit)";
        }
        number = 0;
        break;
    }

    prev = s;
  }

  if(s == separator) {
    if(prev == digit) {
      if(negative) {
        SIMDIMACS_ADD(userdata, -number);
      } else {
        SIMDIMACS_ADD(userdata, number);
      }
    } else if(prev != separator) {
      return "Invalid syntax ('-' or '+' not followed by any digit)";
    }
  }

  return NULL;
}

#define BUFSIZE ((1u << 21) + 16)

/* Read the next part. Also sets correct functions and copies old remaining
   data. */
static inline size_t
next_read_step(char* buf,
               FILE* f,
               size_t step,
               const char** data,
               const char** end,
               bool* eof) {
  assert(!*eof);
  assert(*end - *data <= 16);

  char* tgt = buf + 16;
  if(step % 2 == 0) {
    tgt += (1u << 20);
  }
  const size_t expected = 1u << 20;
  size_t len = fread(tgt, 1, expected, f);
  if(len != expected) {
    *eof = true;
  } else {
    *eof = false;
  }
  if(*data != *end) {
    // There was some data left! Copy this in front of the read data. The buffer
    // is large enough for this.
    size_t tail_len = *end - *data;

    // Only required half of the time!
    if(tgt - tail_len != *data) {
      memcpy(tgt - tail_len, *data, tail_len);
    }
    *data = tgt - tail_len;
  } else {
    *data = tgt;
  }
  *end = tgt + len;
  return step + 1;
}

#define AVX512

#ifdef AVX512
enum lookup {

  DIGIT = 0x80,
  SIGN = 0xc0,
  VALID = 1,
  INVALID = 0
};

static inline const char*
prepare_lookup(const char* separators, uint8_t result[128]) {
  uint8_t* c = (uint8_t*)(separators);

  memset(result, INVALID, 128);

  for(int i = '0'; i <= '9'; i++) {
    result[i] = DIGIT;
  }

  result['-'] = SIGN;
  result['+'] = SIGN;

  while(*c) {
    uint8_t x = *c++;
    if(x & 0x80) {
      return "extended ASCII is not supported";
    }

    switch(x) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '+':
      case '-':
        return "digits and sign chars are reserved";
    }

    result[x] = VALID;
  }

  return NULL;
}

inline static const char*
parse_matrix_chunk(void* userdata,
                   const char** data,
                   const char* end,
                   const __m512i *class_lo,
                   const __m512i *class_hi) {
  while(*data + 64 < end) {
    const __m512i input = _mm512_loadu_si512((__m512i*)(*data));

    const __m512i classes = _mm512_permutex2var_epi8(*class_lo, input, *class_hi);

    if(_mm512_test_epi8_mask(classes, classes) != (uint64_t)-1) {
      return "invalid character";
    }

    uint64_t span_mask64 = _mm512_movepi8_mask(classes);
    uint64_t sign_mask64 =
      _mm512_test_epi8_mask(classes, _mm512_set1_epi8((int8_t)0x40));

    const char* bufend = *data + 64;
    while(*data + 16 <= bufend) {
      const uint16_t span_mask = span_mask64 & 0xffff;
      const uint16_t sign_mask = sign_mask64 & 0xffff;

      const simdimacs_blockinfo* bi = &simdimacs_blocks[span_mask];
      if(sign_mask & bi->invalid_sign_mask) {
        return "'+' or '-' at invalid position";
      }

      const __m128i chunk = _mm_loadu_si128((__m128i*)(*data));

      const __m128i shuffle_digits =
        _mm_loadu_si128((const __m128i*)bi->shuffle_digits);
      const __m128i shuffle_signs =
        _mm_loadu_si128((const __m128i*)bi->shuffle_signs);

      const __m128i shuffled = _mm_shuffle_epi8(chunk, shuffle_digits);
      const __m128i negate_mask = _mm_cmpeq_epi8(
        _mm_shuffle_epi8(chunk, shuffle_signs), _mm_set1_epi8('-'));
      if(bi->conversion_routine == SSE1Digit) {
        convert_1digit(userdata, shuffled, bi->element_count);
      } else if(bi->conversion_routine == SSE2Digits) {
        convert_2digits_signed(
          userdata, shuffled, negate_mask, bi->element_count);
      } else if(bi->conversion_routine == SSE4Digits) {
        convert_4digits_signed(
          userdata, shuffled, negate_mask, bi->element_count);
      } else if(bi->conversion_routine == SSE8Digits) {
        convert_8digits_signed(
          userdata, shuffled, negate_mask, bi->element_count);
      } else {

        printf("case %04x not handled yet\n", span_mask);
        assert(false);
      }

      *data += bi->total_skip;

      span_mask64 >>= bi->total_skip;
      sign_mask64 >>= bi->total_skip;
    }
  }

  return NULL;
}
#else

inline static const char*
parse_matrix_chunk(void* userdata, const char** data, const char* end) {
  const char* err = NULL;
  while(*data + 16 < end) {
    *data = process_chunk(userdata, *data, end, &err);
    if(err)
      return err;
  }
  return err;
}
#endif

const char*
simdimacs_parse(FILE* f, void* userdata) {
  sse_init();

  /*
    Memory Layout: Two buffer areas, one is used to actively read into, while
    the other is read from by the DIMACS parser.
   */

  char buf[BUFSIZE] SSE_ALIGN;

  size_t step = 0;

  const char* data = NULL;
  const char* end = NULL;
  const char* err = NULL;

#ifdef AVX512
  uint8_t classes_lookup[128];

  prepare_lookup("\n ", classes_lookup);

  const __m512i class_lo = _mm512_loadu_si512((__m512i*)(&classes_lookup[0]));
  const __m512i class_hi = _mm512_loadu_si512((__m512i*)(&classes_lookup[64]));
#endif

  bool header_unparsed = true;

  bool eof = false;
  while(!eof) {
    step = next_read_step(buf, f, step, &data, &end, &eof);

    if(header_unparsed) {
      if((err = parse_header(&data, end, userdata))) {
        return err;
      }
      header_unparsed = false;
    }

    err = parse_matrix_chunk(userdata,
                             &data,
                             end
#ifdef AVX512
                             ,
                             &class_lo,
                             &class_hi
#endif
    );
    if(err)
      return err;
  }

  // Tail processing
  if((err = scalar_parse_signed(userdata, data, end - data))) {
    return err;
  }

  return NULL;
}

const char*
simdimacs_parse_path(const char* path, void* userdata) {
  FILE* f = fopen(path, "r");
  const char* err = simdimacs_parse(f, userdata);
  fclose(f);
  return err;
}
