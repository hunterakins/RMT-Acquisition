CFLAGS  = -g -std=gnu99 -Wall -Werror 
CFLAGS += -I/opt/redpitaya/include
LDFLAGS = -L/opt/redpitaya/lib
LDLIBS = -lm -lrp -lpthread -lconfig

BASENAMES := lin_fit \
	     window \
             write \
	     fft \
	     spectral \

vpath = ./src:./obj
SRCDIR := ./src
OBJDIR := ./obj
BASE := $(addprefix $(OBJDIR)/, $(addsuffix .o, $(BASENAMES)))  

BINDIR := ./bin

rmt : 	rmt.o
	$(CC) $(CFLAGS)  $(BASE) $(OBJDIR)/process.o $(OBJDIR)/rmt.o -o $(BINDIR)/$@ $(LDFLAGS) $(LDLIBS)

rmt.o : process.o cascaded.o 
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c $(SRCDIR)/rmt.c -o $(OBJDIR)/$@

cascaded.o: 
	$(CC) $(CFLAGS) -c $(SRCDIR)/cascaded.c -o $(OBJDIR)/$@

process.o : $(BASE) 
	$(CC) $(CFLAGS) -c $(SRCDIR)/process.c -o $(OBJDIR)/$@


$(OBJDIR)/%.o : $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) -c $(CFLAGS) $< -o $@ 
	

clean: 
	$(RM) *.o
	$(RM) $(wildcard $(addprefix $(OBJDIR)/, *.o))
