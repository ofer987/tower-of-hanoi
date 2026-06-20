CC = gcc
CFLAGS = -Wall -Wextra -std=c2x -g
TARGET = tower_of_hanoi
SRC = main.c solution.c termbox2.c
OBJ = $(SRC:.c=.o)

.PHONY: all clean format run test lint build

build: $(TARGET)

all: $(TARGET)

$(TARGET): $(OBJ)
	mkdir -p -- dest
	$(CC) $(CFLAGS) -o dest/$@ $(addprefix dest/,$^)

%.o: %.c
	mkdir -p -- dest
	$(CC) $(CFLAGS) -c $< -o dest/$@

clean:
	rm -f $(addprefix dest/,$(OBJ)) dest/$(TARGET)

format:
	clang-format -i $(SRC)

run: $(TARGET)
	./$(TARGET)

lint: $(TARGET)
	@echo "Running linting..."
	./scripts/check-format.sh

test: $(TARGET)
	@echo "Running tests..."
