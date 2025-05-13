

# Compilateur
CC = gcc

# Options de compilation
CFLAGS = -g -Wall -Wvla -std=c99 -pthread -D_XOPEN_SOURCE=700 -Iinclude
LDFLAGS = -pthread

# Répertoires
SRC_DIR = src
INCLUDE_DIR = include
TEST_DIR = tests

# Executables
EXE = srv clt
TEST = test_list test_buff

# Objets pour le serveur
OBJ_SRV = $(SRC_DIR)/serveur.o $(SRC_DIR)/list.o $(SRC_DIR)/user.o

# Objets pour le client
OBJ_CLT = $(SRC_DIR)/client.o $(SRC_DIR)/buffer.o $(SRC_DIR)/utils.o

# Objets pour les tests de liste
OBJ_TEST_LIST = $(TEST_DIR)/test_list.o $(SRC_DIR)/list.o

# Objets pour les tests de buffer
OBJ_TEST_BUFF = $(TEST_DIR)/test_buff.o $(SRC_DIR)/buffer.o

# Cible par défaut
all: $(EXE)

# Règle pour le serveur
srv: $(OBJ_SRV)
	$(CC) $(LDFLAGS) $^ -o $@

# Règle pour le client
clt: $(OBJ_CLT)
	$(CC) $(LDFLAGS) $^ -o $@

# Règles de compilation génériques
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Dépendances spécifiques
$(SRC_DIR)/client.o: $(SRC_DIR)/client.c $(INCLUDE_DIR)/buffer.h $(INCLUDE_DIR)/utils.h
$(SRC_DIR)/serveur.o: $(SRC_DIR)/serveur.c $(INCLUDE_DIR)/list.h $(INCLUDE_DIR)/user.h
$(SRC_DIR)/user.o: $(SRC_DIR)/user.c $(INCLUDE_DIR)/user.h
$(SRC_DIR)/utils.o: $(SRC_DIR)/utils.c $(INCLUDE_DIR)/utils.h
$(SRC_DIR)/list.o: $(SRC_DIR)/list.c $(INCLUDE_DIR)/list.h
$(SRC_DIR)/buffer.o: $(SRC_DIR)/buffer.c $(INCLUDE_DIR)/buffer.h
$(TEST_DIR)/test_list.o: $(TEST_DIR)/test_list.c $(INCLUDE_DIR)/list.h
$(TEST_DIR)/test_buff.o: $(TEST_DIR)/test_buff.c $( INCLUDE_DIR)/buffer.h

# Cibles pour les tests
test_list: $(OBJ_TEST_LIST)
	$(CC) $(LDFLAGS) $^ -o $@

test_buff: $(OBJ_TEST_BUFF)
	$(CC) $(LDFLAGS) $^ -o $@

# Cible générale pour les tests
test: test_list test_buff
	./test_list
	./test_buff

# Test avec Valgrind
valgrind: test_list test_buff
	valgrind --leak-check=full ./test_list
	valgrind --leak-check=full ./test_buff

# Nettoyer les fichiers générés
clean:
	rm -f $(SRC_DIR)/*.o $(TEST_DIR)/*.o $(EXE) $(TEST)

# Indiquer que ces cibles ne sont pas des fichiers
.PHONY: all clean test valgrind test_list test_buff srv clt

