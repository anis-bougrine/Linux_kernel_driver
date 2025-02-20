CC = gcc

CFLAGS = -s -Wall -Wextra -std=c11

SRCS = user_space_sample_code.c

TARGET = user_space_sample_code.elf

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
