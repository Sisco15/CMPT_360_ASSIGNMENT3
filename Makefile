CC = gcc-13
CFLAGS = -std=c11 -Wall -Wextra -pedantic

all: vmsim

vimsim: vmsim.c vmsim.h
	$(CC) $(CFLAGS) vmsim.c -o vmsim

clean:
	rm -f vmsim vmsim.exe