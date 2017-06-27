CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

main: main.o fc.o
	$(CC) $(LFLAGS) -o $@ $^

main.o: main.cpp fc.h
	$(CC) $(CFLAGS) main.cpp

fc.o: fc.cpp fc.h
	$(CC) $(CFLAGS) fc.cpp

clean:
	rm -f main *.o
