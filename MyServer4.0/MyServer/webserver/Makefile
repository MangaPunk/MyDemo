
CC = gcc
LD = gcc

INC_DIR = include
SRC_DIR = src
OBJ_DIR = objects

# compile time flag to tell gcc where to find header files
CFLAGS = -I$(INC_DIR)

# get all source files in the source directory
SRCS = $(wildcard $(SRC_DIR)/*.c)
# substitute each source file with its corresponding object file
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
PROG = webserver
RM = /bin/rm

#all: webserver

all: $(PROG)

# greetings: main.o greeter.o
# 	gcc -o greetings main.o greeter.o

$(PROG): $(OBJS)
	$(LD) $^ -o $(PROG)

#main.o: main.c
#	gcc -g -c main.c

#greeter.o: greeter.c
#	gcc -g -c greeter.c

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

#clean:
#	rm main.o greeter.o

clean:
	$(RM) $(PROG) $(OBJS)
