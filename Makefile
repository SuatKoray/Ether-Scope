CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -std=c11
TARGET = bin/ether_scope
SRC = src/main.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -rf bin/* build/*