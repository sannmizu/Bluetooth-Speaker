vpath %.h ./include
vpath %.h ./include/nodes
vpath %.c ./src
vpath %.c ./src/nodes
CC = gcc
CFLAGS = -I./include -include stdio.h
LIB = -lpthread -lbluetooth -lasound -lnfc
OBJS = main.o bluetooth.o nfc.o player.o pipe.o list.o memory.o music.o slidewnd.o stream.o

all:main clean

main:$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LIB) -o main

$(OBJS):%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	rm -f $(OBJS)

cleanall:
	rm -f $(OBJS) main