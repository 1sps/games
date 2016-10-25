##
 # Makefile
 #
 # St: 2016-10-13 Thu 04:50 AM
 # Up: 2016-10-25 Tue 06:42 PM
 #
 # Author: SPS
 ##

CC=gcc
CFLAGS=-Wall -pedantic -g

aquarium: aquarium.o mnlib.o
	$(CC) $(CFLAGS) -o $@ $^ -lncurses

aquarium.o: aquarium.c 
	$(CC) $(CFLAGS) -c -o $@ $< -lncurses

mnlib.o: mnlib.c
	$(CC) $(CFLAGS) -c -o $@ $< -lncurses

clean:
	-$(RM) ./*.o
	-$(RM) ./aquarium

