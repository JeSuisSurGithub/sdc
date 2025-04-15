TARGET=test_hash test_mem test_cpu test_cpu2 test_cpu3 test_parser test_perf_parser test_perf_cpu test_es_segment sdc_main
FLAGS=-Wall -Wextra -Wpedantic -ggdb
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

test_perf_parser: test_perf_parser.o hash.o parser.o
	$(CC) $(FLAGS) -o $@ $^

test_perf_cpu: test_perf_cpu.o hash.o memory.o cpu.o parser.o
	$(CC) $(FLAGS) -o $@ $^

test_es_segment: test_es_segment.o hash.o memory.o cpu.o parser.o
	$(CC) $(FLAGS) -o $@ $^

sdc_main: main.o hash.o memory.o cpu.o parser.o
	$(CC) $(FLAGS) -o $@ $^

clean:
	rm -f *.o $(TARGET)
	rm vgcore.*