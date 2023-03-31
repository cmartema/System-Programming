<<<<<<< HEAD
CC = gcc
TARGET = spfind
C_FILES = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(C_FILES))
CFLAGS = -g -Wall -Werror -pedantic-errors
=======
CC      = gcc
TARGET  = spfind
C_FILES = $(wildcard *.c)
OBJS    = $(patsubst %.c,%.o,$(C_FILES))
CFLAGS  = -g -Wall -Werror -pedantic-errors
>>>>>>> d08bb7fc50ac1a6906ffed5220c9d6f22d29e36f
LDFLAGS =

.PHONY: all clean
all: $(TARGET)
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)
%.o: %.c %.h
<<<<<<< HEAD
	$(CC) $(CFLAGS) -c -o $@ $<
=======
>>>>>>> d08bb7fc50ac1a6906ffed5220c9d6f22d29e36f
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
clean:
	rm -f $(OBJS) $(TARGET) $(TARGET).exe
