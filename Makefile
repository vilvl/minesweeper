#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC      := clang++

# define any compile-time flags
CFLAGS	:= -std=c++11 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-variable -g -O0

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS  := -lsfml-graphics -lsfml-window -lsfml-system

# define output directory
OUTPUT	:= build

# define source directory
SRC		:= src

# define include directory
INCLUDE	:= src/include

# define lib directory
LIB		:= lib

ifeq ($(OS),Windows_NT)
	MAIN	    := main.exe
	SOURCEDIRS	:= $(SRC)
	INCLUDEDIRS	:= $(INCLUDE)
	LIBDIRS		:= $(LIB)
	FIXPATH      = $(subst /,\,$1)
	RM			:= del /q /f
	MD	        := mkdir
else
	MAIN	    := main
	SOURCEDIRS	:= $(shell find $(SRC) -type d)
	INCLUDEDIRS	:= $(shell find $(INCLUDE) -type d)
	LIBDIRS		:= $(shell find $(LIB) -type d)
	FIXPATH      = $1
	RM           = rm -f
	MD	        := mkdir -p
endif

# define any directories containing header files other than /usr/include
INCLUDES	:= $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))

# define the C libs
LIBS		:= $(patsubst %,-L%, $(LIBDIRS:%/=%))

# define the C source files
# SOURCES		:= $(wildcard $(patsubst %,%/*.cpp, $(SOURCEDIRS)))
SOURCES		:= src/field.cpp src/mine_sweeper_sfml.cpp src/shared.cpp

# HEADERS     := $(wildcard $(patsubst %,%/*.hpp, $(SOURCEDIRS)))

# define the C object files
OBJECTS		:= $(SOURCES:.cpp=.o)

#
# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT)/$(MAIN))

all: $(OUTPUT) $(MAIN)
	@echo Executing 'all' complete!

$(OUTPUT):
	$(MD) $(OUTPUT)

$(MAIN): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

.PHONY: clean
clean:
	$(RM) $(OUTPUTMAIN)
	$(RM) $(call FIXPATH,$(OBJECTS))
	@echo Cleanup complete!

run: all
	./$(OUTPUTMAIN) 3
	@echo Executing 'run: all' complete!