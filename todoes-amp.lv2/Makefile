#!/usr/bin/make -f

NAME = todoes-amp
TEST_PATH = ./test/plugin/

# ----------------------- #

include Makefile.mk

# --------------------------------------------------------------

PREFIX  ?= /usr/local
DESTDIR ?=
DEBUG   ?= false

# --------------------------------------------------------------
# Default target is to build all plugins

all: build
build: ${NAME}

# --------------------------------------------------------------
# build rules

${NAME}: ${NAME}.lv2/amp$(LIB_EXT) ${NAME}.lv2/manifest.ttl

${NAME}.lv2/amp$(LIB_EXT): amp.c
	$(CC) $^ $(BUILD_C_FLAGS) $(LINK_FLAGS) -lm $(SHARED) -o $@

${NAME}.lv2/manifest.ttl: ${NAME}.lv2/manifest.ttl.in
	sed -e "s|@LIB_EXT@|$(LIB_EXT)|" $< > $@

# --------------------------------------------------------------

clean: test_clean
	rm -f ${NAME}.lv2/amp$(LIB_EXT) ${NAME}.lv2/manifest.ttl

# --------------------------------------------------------------

test: all
	install -d ${TEST_PATH}/${NAME}.lv2

	install -m 644 ${NAME}.lv2/*.so  ${TEST_PATH}/${NAME}.lv2/
	install -m 644 ${NAME}.lv2/*.ttl ${TEST_PATH}/${NAME}.lv2/
	cd test && bash test.sh
# --------------------------------------------------------------
 
test_clean: 
	rm -fr ${TEST_PATH}

# --------------------------------------------------------------

install: build
	install -d $(DESTDIR)$(PREFIX)/lib/lv2/${NAME}.lv2

	install -m 644 ${NAME}.lv2/*.so  $(DESTDIR)$(PREFIX)/lib/lv2/${NAME}.lv2/
	install -m 644 ${NAME}.lv2/*.ttl $(DESTDIR)$(PREFIX)/lib/lv2/${NAME}.lv2/

# --------------------------------------------------------------
.PHONY: test test_clean clean install

