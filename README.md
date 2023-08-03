# SIMDIMACS

Fast DIMACS parsing using SIMD instructions!

## SSE

## AVX512

This mode is only seldomly available. Tested using [Intel's
SDE](https://www.intel.com/content/www/us/en/developer/articles/tool/software-development-emulator.html).

## LRAT and DRAT

The most difficult detail about parsing these proofs is the 'a' and 'd'
separation on-top of processing numbers. They may be represented using INT_MAX
and INT_MIN during SIMDIMACS_ADD calls, but they still have to be detected
efficiently.

Possible digit (d), separator (s), and operator (o) Patterns:

    dddddddddddddddd
    sdsdsdsdsdsdsdsd
    dsdsdsdsdsdsdsds

    ddsosddddddddddd
    ddsssddddddddddd

If separators can never occur in triples, there has to be an operator there! Use
the lookup table to do this stuff, as this area was not used anyway!

The observation of the number pattern in DIMACS changes everything: Only one
separator is possible between literals, otherwise it must be an operation. The
table does a lookup and if there occurs a three separator spot, less numbers are
read, with the operator being pushed as a tail. This can be done in any spot, as
it is just one more (unlikely) if during conversion.
