CC:= gcc
CFLAGS = -g -pg -std=c99 -fprofile-arcs -ftest-coverage -Wall# -g for debug, -O2 for optimise and -Wall additional messages
LFLAGS = -pthread -lm --coverage
SOURCES =client.c server.c
OBJECTS1 =client.o
OBJECTS2 =server.o

EXECUTABLE1 = client
EXECUTABLE2 = server

.PHONY: clean

all: build link1 clean build link2 clean

build: $(SOURCES)

link1: $(EXECUTABLE1)

link2: $(EXECUTABLE2)

$(EXECUTABLE1): $(OBJECTS1)
	$(CC) -o $@ $(OBJECTS1) $(LFLAGS)

$(EXECUTABLE2): $(OBJECTS2)
	$(CC) -o $@ $(OBJECTS2) $(LFLAGS)

$(SOURCES):
	$(CC) $(CFLAGS) -o $@ -c $< -MD

clean:
	rm -rf *.d *.o

-include *.d
