CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o 9cc $(OBJS) $(LDFLAGS)

$(OBJS): 9cc.h

test: 9cc
	./test.sh

self: 9cc
	./self.sh

self_test: 9cc_self
	./self_test.sh

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: test clean self self_test
