CC = gcc
CFLAGS = -Wall

all: user_proc

user_proc: user_proc.o
	$(CC) $(CFLAGS) user_proc.c -o user_proc

clean:
	rm -rf user_proc *.o

run:
	./user_proc
