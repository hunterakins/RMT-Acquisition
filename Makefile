CFLAGS  = -g -std=gnu99 -Wall -Werror 
CFLAGS += -I/opt/redpitaya/include
LDFLAGS = -L/opt/redpitaya/lib
LDLIBS = -lm -lrp -lpthread 

SRCDIR := ./src
OBJDIR := ./obj
OBJS := $(addprefix $(OBJDIR)/, lin_fit.o window.o fft.o spectral.o process.o rmt.o)
BINDIR := ./bin

rmt : 	rmt.o
	$(CC) $(CFLAGS)  $(OBJS) -o ./bin/rmt $(LDFLAGS) $(LDLIBS)

rmt.o : $(OBJS) 
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c ./src/rmt.c 

process.o : lin_fit.o window.o fft.o write.o spectral.o 
	$(CC) $(CFLAGS) -c ./src/process.c -o ./obj/process.o

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) -c $(CFLAGS) $< -o $@ 
	


clean: 
	$(RM) $(OBJS)
	$(RM) *.o
