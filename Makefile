main: *.c headers/*.h
	cc *.c -o main -lncurses -ggdb2