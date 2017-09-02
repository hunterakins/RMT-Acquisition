CFLAGS  = -g -std=gnu99 -Wall -Werror 
CFLAGS += -I/opt/redpitaya/include
LDFLAGS = -L/opt/redpitaya/lib
LDLIBS = -lm -lpthread -lrp -lgsl -lgslcblas -lpthread



SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=)

all: $(OBJS)

$(EXECS): $(OBJS)
	$(CC) $(CFLAGS) -c $(SRCS)

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c $(SRCS)

clean:
	$(RM) *.o
	$(RM) $(OBJS)
