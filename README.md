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

## Benchmark against Kissat Parser

Below is a benchmark against the Kissat parser implemented in pure C:

``` text
time ./kissat /tmp/test3.dimacs > /dev/null

________________________________________________________
Executed in    5.02 secs    fish           external
   usr time    4.81 secs  313.00 micros    4.81 secs
   sys time    0.21 secs  105.00 micros    0.21 secs

maximaximal@MaxDebianDesk ~/s/k/build (master) [10]> time ~/sai/simdimacs/driver -q /tmp/test3.dimacs > /dev/null
    /tmp/test3.dimacs : parse-time: 2.381017 s

________________________________________________________
Executed in    2.38 secs    fish           external
   usr time    2.09 secs  329.00 micros    2.09 secs
   sys time    0.29 secs  111.00 micros    0.29 secs

maximaximal@MaxDebianDesk ~/s/k/build (master)> ~/sai/simdimacs/driver /tmp/test3.dimacs | diff /dev/stdin /tmp/test3.dimacs
    /tmp/test3.dimacs : parse-time: 24.516724 s
maximaximal@MaxDebianDesk ~/s/k/build (master)> du -h /tmp/test3.dimacs
2.5G	/tmp/test3.dimacs
maximaximal@MaxDebianDesk ~/s/k/build (master)> git diff
diff --git a/src/parse.c b/src/parse.c
index 573d996..a9030b0 100644
--- a/src/parse.c
+++ b/src/parse.c
@@ -307,7 +307,7 @@ parse_dimacs (kissat * solver, file * file,
   kissat_message (solver,
                  "parsed 'p cnf %d %" PRIu64 "' header", variables, clauses);
   *max_var_ptr = variables;
-  kissat_reserve (solver, variables);
+  //kissat_reserve (solver, variables);
   uint64_t parsed = 0;
   int lit = 0;
   for (;;)
@@ -414,7 +414,7 @@ parse_dimacs (kissat * solver, file * file,
          parsed++;
          lit = 0;
        }
-      kissat_add (solver, lit);
+      //kissat_add (solver, lit);
     }
   if (lit)
     return "trailing zero missing";
```
