##
 # Makefile for entry
 #
 # St: 2016-10-21 Fri 04:30 PM
 # Up: 2016-10-23 Sun 09:58 PM
 #
 # Author: SPS
 ##

##
 # Note: Dependency on header files is not taken into account 
 ##

CC=gcc
CFLAGS=-Wall -pedantic -g

snake: snake.o mnlib.o deque.o
	$(CC) $(CFLAGS) -o $@ $^ -lncurses

test: snake
	./snake

snake.o: snake.c
	$(CC) $(CFLAGS) -c -o $@ $< -lncurses

mnlib.o: mnlib.c
	$(CC) $(CFLAGS) -c -o $@ $< -lncurses

deque.o: deque.c
	$(CC) $(CFLAGS) -c -o $@ $< -lncurses

clean:
	-$(RM) snake
	-$(RM) *.o

PHONY: clean

