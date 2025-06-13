CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -Iinclude
SRC_DIR = src
BIN_DIR = bin

# Liste automatique de tous les fichiers .c dans src/
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BIN_DIR)/%.o,$(SRCS))

EXEC = $(BIN_DIR)/simulateur_reseau

all: $(EXEC)

$(BIN_DIR):
	@mkdir -p $(BIN_DIR)

$(EXEC): $(OBJS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN_DIR)/%.o: $(SRC_DIR)/%.c | $(BIN_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Nettoyage des fichiers compilés..."
	@rm -f $(BIN_DIR)/*.o $(EXEC)
	@rmdir $(BIN_DIR) 2>/dev/null || true

run: all
	@echo "Lancement du simulateur réseau..."
	./$(EXEC) reseau_config.txt

.PHONY: all clean run
