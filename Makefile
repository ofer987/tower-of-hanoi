CC = gcc
CFLAGS = -Wall -Wextra -std=c2x -g
TARGET = tower_of_hanoi
SRC = main.c solution.c
OBJ = $(SRC:.c=.o)

TEST_TARGET = test_runner
TEST_SRC = tests/test_solution.c solution.c tests/unity/unity.c
TEST_CFLAGS = -Wall -Wextra -std=c2x -g -Itests/unity

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
	rm -f $(addprefix dest/,$(OBJ)) dest/$(TARGET) dest/$(TEST_TARGET)

format:
	clang-format -i $(SRC)

run: $(TARGET)
	./$(TARGET)

lint: $(TARGET)
	@echo "Running linting..."
	./scripts/check-format.sh

test:
	@echo "Running tests..."
	mkdir -p -- dest
	$(CC) $(TEST_CFLAGS) -o dest/$(TEST_TARGET) $(TEST_SRC)
	./dest/$(TEST_TARGET)
