CC = gcc

# Try to find SDL3 installation
SDL3_PREFIX := $(shell find /opt/homebrew/Cellar/sdl3 -maxdepth 1 -type d | sort -V | tail -1)
ifeq ($(SDL3_PREFIX),)
    SDL3_PREFIX := /usr/local
endif

CFLAGS = -Wall -Wextra -std=gnu99 -I$(SDL3_PREFIX)/include
LDFLAGS = -L$(SDL3_PREFIX)/lib -lSDL3

TARGET = main
OBJECTS = main.o platform_sdl.o chip8.o

all: $(TARGET)

verbose: CFLAGS += -DLOG_LEVEL=4 -g
verbose: main


$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
