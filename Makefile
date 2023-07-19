CFLAGS ?= -Wall -Wextra -pedantic -march=native -O3

all: driver

driver: Makefile driver.c simdimacs.c simdimacs.h simdimacs-blockinfo.o
	$(CC) $(CFLAGS) driver.c simdimacs.c simdimacs-blockinfo.o -o driver

simdimacs-blockinfo.o: Makefile simdimacs-blockinfo.c simdimacs-internal.h simdimacs-blockinfo.inc.c
	$(CC) $(CFLAGS) -c simdimacs-blockinfo.c
