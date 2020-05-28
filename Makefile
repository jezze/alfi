GL:=GLES3
BIN_ALFI:=alfi
OBJ_ALFI:=src/alfi.o src/list.o src/parser.o src/url.o src/resource.o src/pool.o src/attribute.o src/widget.o src/history.o
BIN_NAVI:=navi
OBJ_NAVI:=src/navi.o src/list.o src/parser.o src/url.o src/resource.o src/pool.o src/attribute.o src/widget.o src/history.o src/view.o src/gridfmt.o src/style.o src/nvg.o src/nvg_gl.o src/fons.o src/render.o src/animation.o
BIN_NAVI_RESOLVE:=navi-resolve
CFLAGS_GL2:=-DNVG_GL_VERSION_GL2 -DNVG_GL_GLEW
CFLAGS_GL3:=-DNVG_GL_VERSION_GL3 -DNVG_GL_GLEW
CFLAGS_GLES2:=-DNVG_GL_VERSION_GLES2
CFLAGS_GLES3:=-DNVG_GL_VERSION_GLES3
CFLAGS:=$(CFLAGS_$(GL))
LDFLAGS_GL2:=-lGL -lGLEW -lglfw
LDFLAGS_GL3:=-lGL -lGLEW -lglfw
LDFLAGS_GLES2:=-lGL -lglfw
LDFLAGS_GLES3:=-lGL -lglfw
LDFLAGS:=$(LDFLAGS_$(GL))

all: $(BIN_ALFI) $(BIN_NAVI) $(BIN_NAVI_RESOLVE)

%.o: %.c
	@echo CC $@
	@$(CC) -c -o $@ -Wall -Werror -Wno-misleading-indentation -pedantic $(CFLAGS) $^

$(BIN_ALFI): $(OBJ_ALFI)
	@echo LD $@
	@$(CC) -o $@ -Wall -Werror -pedantic $^ -lm

$(BIN_NAVI): $(OBJ_NAVI)
	@echo LD $@
	@$(CC) -o $@ -Wall -Werror -pedantic $^ -lm $(LDFLAGS)

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
	cp data/roboto-regular.ttf /usr/share/navi/roboto-regular.ttf
	cp data/robotomono-regular.ttf /usr/share/navi/robotomono-regular.ttf

