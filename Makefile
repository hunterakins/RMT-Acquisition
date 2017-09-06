CFLAGS  = -g -std=gnu99 -Wall -Werror 
CFLAGS += -I/opt/redpitaya/include
LDFLAGS = -L/opt/redpitaya/lib
LDLIBS = -lm -lrp -lpthread 


OBJDIR := ./obj
OBJS := $(addprefix $(OBJDIR)/, lin_fit.o window.o fft.o spectral.o process.o rmt.o)
BINDIR := ./bin

rmt : rmt.o
	$(CC) $(CFLAGS)  $(OBJS) -o ./bin/rmt $(LDFLAGS) $(LDLIBS)

rmt.o : lin_fit.o window.o fft.o write.o process.o spectral.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c ./src/rmt.c -o ./obj/rmt.o

process.o : lin_fit.o window.o fft.o write.o spectral.o 
	$(CC) $(CFLAGS) -c ./src/process.c -o ./obj/process.o

spectral.o :
	$(CC) $(CFLAGS) -c ./src/spectral.c -o ./obj/spectral.o

lin_fit.o : 
	$(CC) $(CFLAGS) -c ./src/lin_fit.c -o ./obj/lin_fit.o

fft.o:
	$(CC) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c ./src/fft.c -o ./obj/fft.o

window.o :
	$(CC) $(CFLAGS)  -c ./src/window.c -o ./obj/window.o

write.o :
	$(CC) $(CFLAGS) -c ./src/write.c -o ./obj/write.o

clean: 
	$(RM) $(OBJS)
	$(RM) *.o
