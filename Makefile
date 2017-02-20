CC = clang
CFLAGS = -g -Wall -std=c99
LIBS = -lncurses

ifeq ($(debug),1)
	CFLAGS += -DCX_DEBUG_MODE
endif


TARGET = cx

SOURCES = files.c \
	  main.c \
	  path.c \
	  ui.c \
	  util.c

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJECTS)

clobber:
	@rm -f $(TARGET) $(OBJECTS)

.PHONY: all clean clobber
