CFLAGS  = -g -O2 -std=gnu99 -Wall -Werror -Wshadow -Wextra  -Wno-unused-parameter
CFLAGS += -I/opt/redpitaya/include
LDFLAGS = -L/opt/redpitaya/lib
LDFLAGS += -L/usr/local/lib
LDLIBS = -lm -lconfig -lrp -lpthread 

RMT_DEPS := lin_fit \
	   window \
           write \
	   fft \
	   spectral \
	   process \
	   rmt \
	   two_channel_cascaded \
	   cascaded \
	   coherency 

CASCADE_DEPS := write fft window lin_fit

BASENAMES := lin_fit \
	     window \
             write \
	     fft \
	     spectral 

vpath = ./src:./obj
SRCDIR := ./src
OBJDIR := ./obj
BASE := $(addprefix $(OBJDIR)/, $(addsuffix .o, $(BASENAMES)))  
CASCADE_DEPS := $(addprefix $(OBJDIR)/, $(addsuffix .o, $(CASCADE_DEPS)))
RMT_DEPS := $(addprefix $(OBJDIR)/, $(addsuffix .o, $(RMT_DEPS)))

BINDIR := ./bin
TARGET := rmt


$(BINDIR)/$(TARGET) : rmt.o
	$(CC) $(CFLAGS) $(RMT_DEPS) -o $@ $(LDFLAGS) $(LDLIBS)

rmt.o : cascaded.o two_channel_cascaded.o coherency.o 
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c $(SRCDIR)/rmt.c -o $(OBJDIR)/$@ $(LDFLAGS) $(LDLIBS)

coherency.o: process.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/coherency.c -o $(OBJDIR)/$@ $(LDFLAGS) $(LDLIBS)

two_channel_cascaded.o: process.o
	$(CC) $(CFLAGS) -c $(SRCDIR)/two_channel_cascaded.c -o $(OBJDIR)/$@ $(LDFLAGS) $(LDLIBS)

cascaded.o: $(CASCADE_DEPS) 
	$(CC) $(CFLAGS) -c $(SRCDIR)/cascaded.c -o $(OBJDIR)/$@ $(LDFLAGS) $(LDLIBS)

process.o : $(BASE) 
	$(CC) $(CFLAGS) -c $(SRCDIR)/process.c -o $(OBJDIR)/$@


$(OBJDIR)/%.o : $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) -c $(CFLAGS) $< -o $@ 
	

clean: 
	$(RM) *.o
	$(RM) $(wildcard $(addprefix $(OBJDIR)/, *.o))
