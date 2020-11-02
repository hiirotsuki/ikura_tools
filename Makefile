CC ?= gcc
CFLAGS ?= -std=c89 -pedantic -Wall -Wextra -Wconversion -g3 -O0
LDFLAGS ?=

all: sm2mpx10_unpack ggp_decrypt

sm2mpx10_unpack: sm2mpx10_unpack.c
	$(CC) $(CFLAGS) $? -o $@ $(LDFLAGS)

ggp_decrypt: ggp_decrypt.c
	$(CC) $(CFLAGS) $? -o $@ $(LDFLAGS)

clean:
	rm -f sm2mpx10_unpack sm2mpx10_unpack.exe ggp_decrypt ggp_decrypt.exe *.o


.PHONY: clean
