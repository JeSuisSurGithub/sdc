TARGET=test_hash_mem
FLAGS=-Wall -ggdb
CC=gcc

all: $(TARGET)

%.o: %.c
	$(CC) $(FLAGS) -c $^

test_hash_mem: test_hash_mem.o hash.o memory.o
	$(CC) $(FLAGS) -o $@ $^

clean:
	rm -f *.o $(TARGET)