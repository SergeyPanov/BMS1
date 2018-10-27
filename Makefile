CC=g++
CFLAGS=-std=c++11 -Wall -pedantic -O2
LIBS=-lm

all: bms2A bms2B


bms2A: bms1A.cpp
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@ /usr/local/lib/libsndfile.a

bms2B: bms1B.cpp
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@ /usr/local/lib/libsndfile.a

clean:
	rm -f *.o bms1A bms1B
