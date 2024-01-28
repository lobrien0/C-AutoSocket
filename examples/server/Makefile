# Compiler
CC := gcc

# Compiler flags
CFLAGS := -Wall -Wextra -Werror

# Source files
SRCS := $(wildcard *.c)

# Object files
OBJS := $(SRCS:.c=.o)

# Header files
HEADERS := $(wildcard *.h)

# Target executable
TARGET := serverAS

# Default target
all: $(TARGET)

# Rule to compile object files
%.o: %.c $(HEADERS)
	cp ../../AutoSocket.h ./
	cp ../../AutoSocket.c ./
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to link object files and create the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

# Clean rule
clean:
	rm -f $(OBJS) $(TARGET) AutoSocket*
