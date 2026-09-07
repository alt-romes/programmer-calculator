# https://github.com/danielpinto8zz6/c-cpp-project-generator#readme

SHELL := /bin/bash

CC = gcc
CFLAGS := -std=c11 -Wall -Wextra -g -Werror=missing-declarations -Werror=redundant-decls -D_DEFAULT_SOURCE -pedantic-errors
LFLAGS = -lncurses
# OUTPUT := output
SRC := src
BUILDDIR := build
BINDIR := bin
INCLUDE := include
#	LIB := lib

ifeq ($(OS),Windows_NT)
MAIN := pcalc.exe
SOURCEDIRS := $(SRC)
INCLUDEDIRS := $(INCLUDE)
#	LIBDIRS := $(LIB)
FIXPATH = $(subst /,\,$1)
RM := del /q /f
MD := mkdir
else
MAIN := pcalc
SOURCEDIRS := $(shell find $(SRC) -type d)
INCLUDEDIRS := $(shell find $(INCLUDE) -type d)
#	LIBDIRS := $(shell find $(LIB) -type d)
FIXPATH = $1
RM := rm -rf
MD := mkdir -p
CP := cp -i
endif

INCLUDES := $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))
#	LIBS := $(patsubst %,-L%, $(LIBDIRS:%/=%))
SOURCES := $(wildcard $(patsubst %,%/*.c, $(SOURCEDIRS)))
OBJECTS := $(patsubst $(SOURCEDIRS)/%,$(BUILDDIR)/%,$(SOURCES:.c=.o))

all: projdir $(MAIN)
	@echo Executing "all" complete!

projdir:
	@$(MD) $(BUILDDIR)
	@$(MD) $(BINDIR)

$(MAIN): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BINDIR)/$(MAIN) $(OBJECTS) $(LFLAGS) # $(LIBS)

$(BUILDDIR)/%.o: $(SOURCEDIRS)/%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean
clean:
	$(RM) $(BINDIR)
	$(RM) $(BUILDDIR)
	@echo Cleanup complete!

run: all
	$(BINDIR)/$(MAIN)
	@echo Executing "run: all" complete!

.PHONY: install
# Won't work for Windows Platform
install:
	@echo "Installing!"
	$(MAKE) all
	@$(CP) $(BINDIR)/$(MAIN) /usr/local/bin


.PHONY: uninstall
uninstall:
	@echo "Unistalling :("
	@$(RM) $(shell whereis $(MAIN) | cut -d " " -f 2)

# --- WebAssembly build ---
# ncurses has no official Emscripten port, so we cross-compile a minimal
# static build of it once into WASMDEPS, then link pcalc against it.
#
# A known-good Emscripten SDK is vendored automatically into WASMDEPS/emsdk
# on first use (this is deliberate: the Debian/Ubuntu `emscripten` apt
# package is version 3.1.6 and its old JS-library preprocessor is
# incompatible with xterm-pty's emscripten-pty.js). If you already have a
# newer emsdk activated in your shell, it is ignored in favor of this one,
# so `make web` behaves the same for everyone.
WASMDEPS := $(BUILDDIR)/wasm-deps
NCURSES_VERSION := 6.4
NCURSES_SRC := $(WASMDEPS)/ncurses-$(NCURSES_VERSION)
NCURSES_INSTALL := $(WASMDEPS)/ncurses-install
WEBDIR := web

EMSDK_DIR := $(WASMDEPS)/emsdk
EMSDK_READY := $(EMSDK_DIR)/.ready
EMSDK_ENV := . $(abspath $(EMSDK_DIR))/emsdk_env.sh >/dev/null

.PHONY: web
web: $(EMSDK_READY) $(NCURSES_INSTALL)/lib/libncurses.a $(WEBDIR)/emscripten-pty.js
	$(EMSDK_ENV) && emcc -std=c11 -D_DEFAULT_SOURCE -Werror=missing-declarations -Werror=redundant-decls \
		-I $(INCLUDE) -I $(NCURSES_INSTALL)/include/ncurses -I $(NCURSES_INSTALL)/include \
		$(wildcard $(SRC)/*.c) \
		-L $(NCURSES_INSTALL)/lib -lncurses \
		--js-library=$(WEBDIR)/emscripten-pty.js \
		-s ASYNCIFY -s FORCE_FILESYSTEM \
		--embed-file $(NCURSES_INSTALL)/share/terminfo@/usr/share/terminfo \
		-o $(WEBDIR)/pcalc.mjs
	@echo "Built $(WEBDIR)/pcalc.mjs + $(WEBDIR)/pcalc.wasm -- serve $(WEBDIR)/ over HTTP and open index.html"

$(EMSDK_READY):
	@$(MD) $(WASMDEPS)
	$(RM) $(EMSDK_DIR)
	git clone --depth 1 https://github.com/emscripten-core/emsdk.git $(EMSDK_DIR)
	cd $(EMSDK_DIR) && ./emsdk install latest && ./emsdk activate latest
	@touch $(EMSDK_READY)

$(WEBDIR)/emscripten-pty.js:
	curl -sL https://unpkg.com/xterm-pty/emscripten-pty.js -o $@

$(NCURSES_SRC)/configure:
	@$(MD) $(WASMDEPS)
	curl -sL https://ftp.gnu.org/gnu/ncurses/ncurses-$(NCURSES_VERSION).tar.gz | tar xz -C $(WASMDEPS)

$(NCURSES_INSTALL)/lib/libncurses.a: $(EMSDK_READY) $(NCURSES_SRC)/configure
	@$(MD) $(NCURSES_SRC)/build-wasm
	cd $(NCURSES_SRC)/build-wasm && $(EMSDK_ENV) && emconfigure ../configure \
		--prefix=$(abspath $(NCURSES_INSTALL)) \
		--host=wasm32-unknown-emscripten \
		--without-shared --with-normal --without-debug --without-ada \
		--without-progs --without-manpages --without-tests \
		--disable-stripping --without-pkg-config \
		--with-fallbacks=xterm,xterm-256color,vt100,screen \
		CC=emcc CXX=em++ AR=emar RANLIB=emranlib
	cd $(NCURSES_SRC)/build-wasm && $(EMSDK_ENV) && emmake $(MAKE) -j4 && emmake $(MAKE) install

.PHONY: web-clean
web-clean:
	$(RM) $(WASMDEPS)
	$(RM) $(WEBDIR)/pcalc.mjs $(WEBDIR)/pcalc.wasm $(WEBDIR)/emscripten-pty.js
