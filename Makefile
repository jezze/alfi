BIN_ALFI:=alfi
OBJ_ALFI:=src/alfi.o src/list.o src/parser.o src/call.o src/pool.o
BIN_NAVI:=navi
OBJ_NAVI:=src/navi.o src/list.o src/parser.o src/call.o src/pool.o src/box.o src/nvg.o src/nvg_gl.o
BIN_NAVI_RESOLVE:=navi-resolve

all: $(BIN_ALFI) $(BIN_NAVI) $(BIN_NAVI_RESOLVE)

%.o: %.c
	@echo CC $@
	@$(CC) -c -o $@ -Wall -Werror -Wno-misleading-indentation -pedantic $^

$(BIN_ALFI): $(OBJ_ALFI)
	@echo LD $@
	@$(CC) -o $@ -Wall -Werror -pedantic $^

$(BIN_NAVI): $(OBJ_NAVI)
	@echo LD $@
	@$(CC) -o $@ -Wall -Werror -pedantic $^ -lm -lGL -lGLEW -lglfw

$(BIN_NAVI_RESOLVE): src/$(BIN_NAVI_RESOLVE)
	@echo CP $@
	@cp $^ $@

clean:
	@$(RM) $(BIN_ALFI) $(OBJ_ALFI) $(BIN_NAVI) $(OBJ_NAVI) $(BIN_NAVI_RESOLVE)

install:
	mkdir -p /usr/bin
	cp $(BIN_NAVI) /usr/bin/$(BIN_NAVI)
	cp $(BIN_ALFI) /usr/bin/$(BIN_ALFI)
	cp $(BIN_NAVI_RESOLVE) /usr/bin/$(BIN_NAVI_RESOLVE)
	mkdir -p /usr/share/navi
	cp data/icofont.ttf /usr/share/navi/icofont.ttf
	cp data/roboto-bold.ttf /usr/share/navi/roboto-bold.ttf
	cp data/roboto-light.ttf /usr/share/navi/roboto-light.ttf
	cp data/roboto-regular.ttf /usr/share/navi/roboto-regular.ttf
	cp data/image6.jpg /usr/share/navi/image6.jpg
	cp data/example.alfi /usr/share/navi/example.alfi

