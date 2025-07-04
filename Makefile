# https://github.com/danielpinto8zz6/c-cpp-project-generator#readme

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
