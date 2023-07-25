CFLAGS ?= -Wall -Wextra -pedantic -march=native

all: driver driver-dbg

driver: Makefile driver.c simdimacs.c simdimacs.h simdimacs-blockinfo.o
	$(CC) $(CFLAGS) -O3 driver.c simdimacs.c simdimacs-blockinfo.o -o driver

simdimacs-blockinfo.o: Makefile simdimacs-blockinfo.c simdimacs-internal.h simdimacs-blockinfo.inc.c
	$(CC) $(CFLAGS) -O3 -c simdimacs-blockinfo.c

driver-dbg: Makefile driver.c simdimacs.c simdimacs.h simdimacs-blockinfo-dbg.o
	$(CC) $(CFLAGS) -g -fsanitize=address -fsanitize=undefined  driver.c simdimacs.c simdimacs-blockinfo-dbg.o -o driver-dbg

simdimacs-blockinfo-dbg.o: Makefile simdimacs-blockinfo.c simdimacs-internal.h simdimacs-blockinfo.inc.c
	$(CC) $(CFLAGS) -g -fsanitize=address -fsanitize=undefined -c simdimacs-blockinfo.c -o simdimacs-blockinfo-dbg.o
