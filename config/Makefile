CFLAGS  = -g -O2 -std=gnu99 -Wall -Werror -Wshadow -Wextra -Wdeclaration-after-statement -Wno-unused-parameter
CFLAGS += -I/opt/redpitaya/include
LDFLAGS = -L/opt/redpitaya/lib
LDFLAGS += -L/usr/local/lib
LDLIBS = -lm -lrp -lpthread -lconfig

example : example1.c
	$(CC) $(CFLAGS) example1.c  $(LDFLAGS) $(LDLIBS)

example1.c :


clean: 
	$(RM) *.o
	$(RM) $(wildcard $(addprefix $(OBJDIR)/, *.o))
