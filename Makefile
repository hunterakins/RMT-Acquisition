CFLAGS  = -g -O2 -std=gnu99 -Wall -Werror -Wshadow -Wextra  -Wno-unused-parameter
CFLAGS += -I/opt/redpitaya/include
LDFLAGS = -L/opt/redpitaya/lib
LDFLAGS += -L/usr/local/lib
LDLIBS = -lm -lconfig -lrp -lpthread 

BASENAMES := lin_fit \
	     window \
             write \
	     fft \
	     spectral \


vpath = ./src:./obj
SRCDIR := ./src
OBJDIR := ./obj
# programs needed for process.o
BASE := $(addprefix $(OBJDIR)/, $(addsuffix .o, $(BASENAMES)))  
BINDIR := ./bin

rmt : 	rmt.o
	$(CC) $(CFLAGS) $(BASE) $(OBJDIR)/process.o $(OBJDIR)/gnuplot_i.o $(OBJDIR)/cascaded.o $(OBJDIR)/two_channel_cascaded.o $(OBJDIR)/rmt.o -o $(BINDIR)/$@ $(LDFLAGS) $(LDLIBS)

rmt.o : process.o cascaded.o two_channel_cascaded.o  
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c $(SRCDIR)/rmt.c -o $(OBJDIR)/$@ $(LDFLAGS) $(LDLIBS)

two_channel_cascaded.o: cascaded.o
	$(CC) $(CFLAGS) $(OBJDIR)/cascaded.o -c $(SRCDIR)/two_channel_cascaded.c -o $(OBJDIR)/$@ $(LDFLAGS) $(LDLIBS)

cascaded.o: 
	$(CC) $(CFLAGS) -c $(SRCDIR)/cascaded.c -o $(OBJDIR)/$@ $(LDFLAGS) $(LDLIBS)

process.o : $(BASE) 
	$(CC) $(CFLAGS) -c $(SRCDIR)/process.c -o $(OBJDIR)/$@


$(OBJDIR)/%.o : $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) -c $(CFLAGS) $< -o $@ 
	

clean: 
	$(RM) *.o
	$(RM) $(wildcard $(addprefix $(OBJDIR)/, *.o))
