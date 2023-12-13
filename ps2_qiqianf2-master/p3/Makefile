CC = gcc
CFLAGS = -Wall -pthread

all: lab_occupancy

lab_occupancy: main.o solution.o spinlock_ece391.o
	$(CC) $(CFLAGS) -o lab_occupancy main.o solution.o spinlock_ece391.o

solution.o: solution.c solution.h spinlock_ece391.h
	$(CC) $(CFLAGS) -c solution.c

spinlock_ece391.o: spinlock_ece391.c spinlock_ece391.h
	$(CC) $(CFLAGS) -c spinlock_ece391.c

clean:
	rm -f solution.o lab_occupancy
