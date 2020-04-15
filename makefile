CC = gcc 

INCLUDES = -I/home/jplank/cs360/include 

CFLAGS = -g -lpthread $(INCLUDES)

LIBDIR = /home/jplank/cs360/objs

LIBS = $(LIBDIR)/libfdr.a 

EXECUTABLES = chat_server

all: $(EXECUTABLES)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(CFLAGS) -c $*.c 

chat_server: chat_server.o 
	$(CC) $(CFLAGS) -o chat_server chat_server.o sockettome.o $(LIBS) 

#make clean will rid your directory of the executable,
#object files, and any core dumps you've caused
clean:
	rm $(EXECUTABLES) chat_server.o