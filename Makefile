TARGET=test_hash test_mem test_cpu test_cpu2 test_cpu3 test_parser
FLAGS=-Wall -ggdb
CC=gcc

all: $(TARGET)

%.o: %.c
	$(CC) $(FLAGS) -c $^

test_hash: test_hash.o hash.o
	$(CC) $(FLAGS) -o $@ $^

test_mem: test_mem.o hash.o memory.o
	$(CC) $(FLAGS) -o $@ $^

test_cpu: test_cpu.o hash.o memory.o cpu.o parser.o
	$(CC) $(FLAGS) -o $@ $^

test_cpu2: test_cpu2.o hash.o memory.o cpu.o
	$(CC) $(FLAGS) -o $@ $^

test_cpu3: test_cpu3.o hash.o memory.o cpu.o parser.o
	$(CC) $(FLAGS) -o $@ $^

test_parser: test_parser.o hash.o parser.o
	$(CC) $(FLAGS) -o $@ $^

clean:
	rm -f *.o $(TARGET)
	rm vgcore.*
