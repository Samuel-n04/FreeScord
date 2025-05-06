CC = gcc
CFLAGS = -g -Wall -Wvla -std=c99 -pthread -D_XOPEN_SOURCE=700 -Iinclude
LDFLAGS = -pthread -Wall
EXE = srv clt
TEST = test_list

SRC_DIR = src
INCLUDE_DIR = include
OBJ = $(SRC_DIR)/serveur.o $(SRC_DIR)/list.o $(SRC_DIR)/user.o
OBJ_CLT = $(SRC_DIR)/client.o $(SRC_DIR)/buffer.o $(SRC_DIR)/utils.o
OBJ_TEST = $(SRC_DIR)/test_list.o $(SRC_DIR)/list.o

all: $(EXE)

srv: $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

clt: $(OBJ_CLT)
	$(CC) $(LDFLAGS) $^ -o $@

$(SRC_DIR)/client.o: $(SRC_DIR)/client.c $(INCLUDE_DIR)/buffer.h $(INCLUDE_DIR)/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/serveur.o: $(SRC_DIR)/serveur.c $(INCLUDE_DIR)/list.h $(INCLUDE_DIR)/user.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/user.o: $(SRC_DIR)/user.c $(INCLUDE_DIR)/user.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/utils.o: $(SRC_DIR)/utils.c $(INCLUDE_DIR)/utils.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/list.o: $(SRC_DIR)/list.c $(INCLUDE_DIR)/list.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/buffer.o: $(SRC_DIR)/buffer.c $(INCLUDE_DIR)/buffer.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/test_list.o: $(SRC_DIR)/test_list.c $(INCLUDE_DIR)/list.h
	$(CC) $(CFLAGS) -c $< -o $@

test_list: $(OBJ_TEST)
	$(CC) $(LDFLAGS) $^ -o $@

test: test_list
	valgrind --leak-check=full ./test_list

clean:
	rm -f $(SRC_DIR)/*.o $(EXE) $(TEST)
