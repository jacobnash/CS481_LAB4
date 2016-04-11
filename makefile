EXECUTABLES = elevator_null elevator_part_1 elevator_part_2 reorder double-check
#EXECUTABLES = elevator_null reorder double-check

CC = gcc
LIBS = libfdr.a
CFLAGS = -O2

LIBFDROBJS = dllist.o fields.o jval.o jrb.o

all: $(EXECUTABLES)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -w -c $*.c

double-check: double-check.o 
	$(CC) $(CFLAGS) -o double-check double-check.o $(LIBS) -lpthread -lm

reorder: reorder.o 
	$(CC) $(CFLAGS) -o reorder reorder.o $(LIBS) -lpthread -lm

elevator_null: elevator_skeleton.o elevator_null.o finesleep.o libfdr.a
	$(CC) $(CFLAGS) -o elevator_null elevator_skeleton.o elevator_null.o finesleep.o $(LIBS) -lpthread -lm

elevator_part_1: elevator_skeleton.o elevator_part_1.o finesleep.o libfdr.a
	$(CC) $(CFLAGS) -o elevator_part_1 elevator_skeleton.o elevator_part_1.o finesleep.o $(LIBS) -lpthread -lm

elevator_part_2: elevator_skeleton.o elevator_part_2.o finesleep.o libfdr.a
	$(CC) $(CFLAGS) -o elevator_part_2 elevator_skeleton.o elevator_part_2.o finesleep.o $(LIBS) -lpthread -lm

elevator_skeleton.o: elevator.h names.h
elevator.o: elevator.h

libfdr.a: $(LIBFDROBJS)
	ar ru libfdr.a $(LIBFDROBJS)
	ranlib libfdr.a 

clean:
	rm -f core *.o $(EXECUTABLES) *~ libfdr.a
test: elevator_part_2
	touch test1.txt
	rm test*
	./elevator_part_2 10 1 .1 .1 .1 12 0 >> test1.txt
	./elevator_part_2 10 10 .01 .1 .1 12 0 >> test2.txt 
	./elevator_part_2 10 10 .1 .01 .01 12 0 >> test3.txt
	./elevator_part_2 10 10 .05 .1 .1 12 0 >> test4.txt
	./elevator_part_2 10 10 .02 .2 .01 12 0 >> test5.txt
	./elevator_part_2 10 6 .02 .02 .02 12 0 >> test6.txt
	./elevator_part_2 100 25 .02 .2 .01 12 0 >> test7.txt
	./elevator_part_2 100 4 .01 .01 .01 12 0 >> test8.txt
	cat test1.txt | ./reorder | ./double-check
	cat test2.txt | ./reorder | ./double-check
	cat test3.txt | ./reorder | ./double-check
	cat test4.txt | ./reorder | ./double-check
	cat test5.txt | ./reorder | ./double-check
	cat test6.txt | ./reorder | ./double-check
	cat test7.txt | ./reorder | ./double-check
	cat test8.txt | ./reorder | ./double-check
