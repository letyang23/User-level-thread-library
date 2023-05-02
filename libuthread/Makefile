## Target library
#lib := libuthread.a
#
#all: $(lib)

## TODO: Phase 1
all: queue_tester

queue_tester: queue_tester.o queue.o
	gcc -Wall -Wextra -Werror -o queue_tester queue_tester.o queue.o

queue_tester.o: ../apps/queue_tester.c queue.h
	gcc -Wall -Wextra -Werror -c -o queue_tester.o ../apps/queue_tester.c

queue.o: queue.c queue.h
	gcc -Wall -Wextra -Werror -c -o queue.o queue.c

clean:
	rm -f queue_tester queue_tester.o queue.o