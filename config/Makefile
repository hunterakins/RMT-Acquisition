CFLAGS  = -g -std=gnu99 -Wall -Werror 
CFLAGS += -I/opt/redpitaya/include
LDFLAGS = -L/opt/redpitaya/lib
LDLIBS = -lm -lrp -lpthread 

vpath = ./src:./obj
SRCDIR := ./src
OBJDIR := ./obj


conf_test : config.o 
	$(CC) $(CFLAGS) config.o -o conf_test $(LDFLAGS) $(LDLIBS)

config.o : 
	$(CC) $(CFLAGS) config.c -o config.o $(LDFLAGS) $(LDLIBS)


clean: 
	$(RM) *.o
	$(RM) $(wildcard $(addprefix $(OBJDIR)/, *.o))
