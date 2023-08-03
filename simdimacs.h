/* SIMDIMACS - Fast DIMACS parser using SIMD instructions
 *
 * Copyright (c) 2023 Maximilian Heisinger <mail@maxheisinger.at>
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 *
 * USAGE
 * =====
 *
 * Implement the following functions (names may be changed):
 *
 * simdimacs_problem(void* userdata, int variables, int clauses):
 *   initiate the problem with given vars and clause counts.
 *
 * simdimacs_add(void* userdata, int lit):
 *   add another litaral. Clauses are ended by the 0 literal.
 */

#ifndef SIMDIMACS_H
#define SIMDIMACS_H

#ifndef SIMDIMACS_ADD
#define SIMDIMACS_ADD simdimacs_add
#endif

#ifndef SIMDIMACS_PROBLEM
#define SIMDIMACS_PROBLEM simdimacs_problem
#endif

#define SIMDIMACS_OP_D INT_MAX
#define SIMDIMACS_OP_A INT_MIN
#define SIMDIMACS_OP_E (INT_MIN + 1)

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Parse the file into a CNF, using SIMDIMACS_ADD(userdata,
 *         SIMDIMACS_LIT).
 *
 * Returns: NULL on success, error code on failure.
 *
 * Doesn't use the heap, but allocates 2MB on the stack for the two-part read
 * buffer.
 */
const char*
simdimacs_parse(FILE*, void*);

/** @brief Parse the file into a CNF, using SIMDIMACS_ADD(userdata,
 *         SIMDIMACS_LIT).
 *
 * Returns: NULL on success, error code on failure.
 *
 * Doesn't use the heap, but allocates 2MB on the stack for the two-part read
 * buffer.
 *
 * Processes DRAT and LRAT files by representing 'd' with INT_MIN and 'a' as
 * INT_MAX in calls to SIMDIMACS_ADD. This variant does not process the header.
 */
const char*
simdimacs_parse_lrat_or_drat(FILE*, void*);

/** @brief Parse the file into a CNF, using SIMDIMACS_ADD(userdata,
 *         SIMDIMACS_LIT).
 *
 * Returns: NULL on success, error code on failure.
 *
 * Doesn't use the heap, but allocates 2MB on the stack for the two-part read
 * buffer.
 */
const char*
simdimacs_parse_path(const char* path, void*);

void
SIMDIMACS_PROBLEM(void* userdata, int variables, int clauses);
void
SIMDIMACS_ADD(void* userdata, int lit);

#ifdef SIMDIMACS_STATS
/* Parsing statistics embedded in SIMDIMACS. Enabled by defining
 * SIMDIMACS_STATS. Print them using simdimacs_print_stats() */
void
simdimacs_print_stats();
#endif

#ifdef __cplusplus
}
#endif

#endif// SIMDIMACS_H
