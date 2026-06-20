CC = gcc
CFLAGS = -Wall -Wextra -std=c23 -O2
TARGET = tower_of_hanoi
SRC = main.c solution.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean format run test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

format:
	clang-format -i $(SRC)

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	@echo "Running tests..."
	./$(TARGET)
