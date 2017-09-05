CFLAGS  = -g -std=gnu99 -Wall -Werror 
CFLAGS += -I/opt/redpitaya/include
LDFLAGS = -L/opt/redpitaya/lib
LDLIBS = -lm -lrp -lpthread 


SRCDIR := ./src
SRCS := $(addprefix $(SRCDIR)/, lin_fit.c lin_fit_test.c)
HDRS := $(addprefix $(SRCDIR)/, lin_fit.h)
OBJDIR := ./obj
OBJS := $(addprefix $(OBJDIR)/, lin_fit.o lin_fit_test.o window.o fft.o)
BINDIR := ./bin

lin_fit_test : lin_fit_test.o
	$(CC) $(CFLAGS)  $(OBJS) -o ./bin/lin_fit_test $(LDFLAGS) $(LDLIBS)

lin_fit_test.o : lin_fit.o window.o fft.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c ./src/lin_fit_test.c ./obj/lin_fit.o  -o ./obj/lin_fit_test.o

lin_fit.o : 
	$(CC) $(CFLAGS) -c ./src/lin_fit.c -o ./obj/lin_fit.o

fft.o:
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c ./src/fft.c -o ./obj/fft.o

window.o :
	$(CC) $(CFLAGS)  -c ./src/window.c -o ./obj/window.o


clean: 
	$(RM) $(OBJS)
	$(RM) *.o
