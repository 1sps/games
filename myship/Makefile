##
 # Makefile
 # ========
 #
 # St: 2016-10-11 Tue 02:04 PM
 # Up: 2016-10-11 Tue 02:10 PM
 #
 # Author: SPS
 ##

CC=gcc
CFLAGS=-Wall -pedantic -g


playship: spaceship.o mnlib.o
	$(CC) $(CFLAGS) -o $@ $^ -lncurses

spaceship.o: spaceship.c 
	$(CC) $(CFLAGS) -c -o $@ $< -lncurses

mnlib.o: mnlib.c
	$(CC) $(CFLAGS) -c -o $@ $< -lncurses

clean:
	-$(RM) ./*.o
	-$(RM) ./playship

