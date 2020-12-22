PREFIX := /usr/local/bin
CC := gcc -O2

.PHONY: all
all: 2in1screen

.PHONY: clean
clean:
	rm -f *.o 2in1screen

.PHONY: install
install:
	cp 2in1screen $(PREFIX)/

2in1screen: 2in1screen.c
	$(CC) -o $@ $^