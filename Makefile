CFLAGS ?= -mavx -g -Wall -Wextra -pedantic -mtune=native -march=native -DSIMDIMACS_STATS

all: driver driver-dbg driver-avx512

driver: Makefile driver.c simdimacs.c simdimacs.h simdimacs-blockinfo.o
	$(CC) $(CFLAGS) -O3 driver.c simdimacs.c simdimacs-blockinfo.o -o driver

driver-avx512: Makefile driver.c simdimacs.c simdimacs.h simdimacs-blockinfo.o
	$(CC) $(CFLAGS) -O3 -mavx512bw -mavx512vbmi -DAVX512 driver.c simdimacs.c simdimacs-blockinfo.o -o driver-avx512

simdimacs-blockinfo.o: Makefile simdimacs-blockinfo.c simdimacs-internal.h simdimacs-blockinfo.inc.c
	$(CC) $(CFLAGS) -O3 -c simdimacs-blockinfo.c

driver-dbg: Makefile driver.c simdimacs.c simdimacs.h simdimacs-blockinfo-dbg.o
	$(CC) $(CFLAGS) -fsanitize=address -fsanitize=undefined  driver.c simdimacs.c simdimacs-blockinfo-dbg.o -o driver-dbg

simdimacs-blockinfo-dbg.o: Makefile simdimacs-blockinfo.c simdimacs-internal.h simdimacs-blockinfo.inc.c
	$(CC) $(CFLAGS) -fsanitize=address -fsanitize=undefined -c simdimacs-blockinfo.c -o simdimacs-blockinfo-dbg.o

clear:
	rm -f *.o
