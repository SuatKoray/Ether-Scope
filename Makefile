CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -std=c11

SRC_DIR = src
OBJ_DIR = build
BIN_DIR = bin

TARGET = $(BIN_DIR)/ether_scope

# src klasöründeki tüm .c dosyalarını bulur
SRCS = $(wildcard $(SRC_DIR)/*.c)

# .c dosyalarını build klasöründe .o dosyalarına çevirir
OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: $(TARGET)

# Nihai birleştirme adımı
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

# Her bir .c dosyasını ayrı ayrı derleme adımı
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)/* $(BIN_DIR)/*