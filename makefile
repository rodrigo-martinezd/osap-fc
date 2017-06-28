CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

main: main.o
	$(CC) $(LFLAGS) -o $@ $^

main.o: main.cpp
	$(CC) $(CFLAGS) main.cpp

clean:
	rm -f main *.o
