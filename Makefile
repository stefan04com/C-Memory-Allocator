
CC = gcc
CFLAGS = -Wall -Wextra -g


build: sfl.o
	$(CC) $(CFLAGS) sfl.o -o sfl -fsanitize=address,undefined -g -Og


run_sfl: build
	./sfl


clean:
	rm -f sfl *.o
