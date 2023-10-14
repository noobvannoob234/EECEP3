#Myles Coleman and Andrew Roda
#EECE 446
#Fall 2023
EXE = peer
CFLAGS = -Wall
CXXFLAGS = -Wall
LDLIBS =
CC = gcc
CXX = g++

.PHONY: all
all: $(EXE)

peer: p2p.c
	$(CC) $(CFLAGS) p2p.c $(LDLIBS) -o peer

.PHONY: clean
clean:
	rm -f $(EXE)
