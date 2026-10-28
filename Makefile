# ==============================================================================
# Makefile for Scientific Calculator (C Engine)
# MISRA C:2012 compliant compilation with zero dynamic memory allocation
# ==============================================================================

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -O2
LDFLAGS = -lm

TARGET = scientific_calculator.exe
SRCS = main.c calculator.c parser.c command.c
OBJS = $(SRCS:.c=.o)
HEADERS = calculator.h parser.h command.h

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

main.o: main.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

calculator.o: calculator.c calculator.h parser.h
	$(CC) $(CFLAGS) -c $< -o $@

parser.o: parser.c parser.h calculator.h command.h
	$(CC) $(CFLAGS) -c $< -o $@

command.o: command.c command.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
ifeq ($(OS),Windows_NT)
	-cmd /c del /f /q $(OBJS) $(TARGET) 2>nul || exit 0
else
	rm -f $(OBJS) $(TARGET)
endif
