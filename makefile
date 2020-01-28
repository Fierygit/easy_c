
CC = g++
BIN = ecc
OBJ_FILES = main.o scan.o util.o parse.o analyse.o genMid.o asmber.o

$(BIN): $(OBJ_FILES) 
	$(CC) -g -o $(BIN) $(OBJ_FILES)

target: $(BIN)