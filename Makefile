TARGET=test
FLAGS=-Wall -ggdb
CC=gcc

all: $(TARGET)

%.o: %.c
	$(CC) $(FLAGS) -c $^

test: test.o hash.o memory.o
	$(CC) $(FLAGS) -o $@ $^

clean:
	rm -f *.o $(TARGET)