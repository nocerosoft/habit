CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lsqlite3

SRC_DIR = src
BIN_DIR = bin

TARGET = $(BIN_DIR)/habit
OBJS = $(SRC_DIR)/habit.o $(SRC_DIR)/db.o

all: $(BIN_DIR) $(TARGET)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(SRC_DIR)/db.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BIN_DIR) $(OBJS)

.PHONY: all clean
