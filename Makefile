BIN1:=navi
OBJ1:=src/navi.o src/list.o src/parser.o src/box.o
BIN2:=alfi
OBJ2:=src/alfi.o src/list.o src/parser.o

all: $(BIN1) $(BIN2)

%.o: %.c
	$(CC) -c -o $@ -Wall -Werror -std=c89 -pedantic $^

$(BIN1): $(OBJ1)
	$(CC) -o $@ -Wall -Werror -std=c89 -pedantic $^ -lm -lGL -lglfw -lnanovg

$(BIN2): $(OBJ2)
	$(CC) -o $@ -Wall -Werror -std=c89 -pedantic $^

clean:
	$(RM) $(BIN1) $(OBJ1) $(BIN2) $(OBJ2)
