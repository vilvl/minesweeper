#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#

# define the C compiler to use
CC      := clang++

# define any compile-time flags
CFLAGS	:= -std=c++11 -Wall -Werror -pedantic-errors -g3 -O0
#-Wno-unused-parameter -Wno-unused-variable

# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS  := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-network

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
	CLIENT		:= client.exe
	SERVER		:= server.exe
	SOURCEDIRS	:= $(SRC)
	INCLUDEDIRS	:= $(INCLUDE)
	LIBDIRS		:= $(LIB)
	FIXPATH      = $(subst /,\,$1)
	RM			:= del /q /f
	MD	        := mkdir
else
	MAIN	    := main
	CLIENT		:= client
	SERVER 		:= server
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
SOURCES_MAIN	:= 	src/field.cpp \
				src/mine_sweeper_sfml.cpp

SOURCES_SERV	:= src/field.cpp \
				src/shared.cpp \
				src/mine_server.cpp

SOURCES_CLI 	:= src/field.cpp \
				src/shared.cpp \
				src/client.cpp

# HEADERS     := $(wildcard $(patsubst %,%/*.hpp, $(SOURCEDIRS)))

# define the C object files
OBJECTS_MAIN	:= $(SOURCES_MAIN:.cpp=.o)
OBJECTS_SERV	:= $(SOURCES_SERV:.cpp=.o)
OBJECTS_CLI		:= $(SOURCES_CLI:.cpp=.o)

#
# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

OUTPUTMAIN	:= $(call FIXPATH,$(OUTPUT)/$(MAIN))
OUTPUTSERV	:= $(call FIXPATH,$(OUTPUT)/$(SERVER))
OUTPUTCLI	:= $(call FIXPATH,$(OUTPUT)/$(CLIENT))

all: $(OUTPUT) $(MAIN) $(SERVER) # $(CLIENT)
	@echo Executing 'all' complete!

$(OUTPUT):
	$(MD) $(OUTPUT)

$(MAIN): $(OBJECTS_MAIN)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUTPUTMAIN) $(OBJECTS_MAIN) $(LFLAGS) $(LIBS)

$(SERVER): $(OBJECTS_SERV)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUTPUTSERV) $(OBJECTS_SERV) $(LFLAGS) $(LIBS)

$(CLIENT): $(OBJECTS_CLI)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(OUTPUTCLI) $(OBJECTS_CLI) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file)
# (see the gnu make manual section about automatic variables)
.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

.PHONY: clean
clean:
	$(RM) $(OUTPUTMAIN)
	$(RM) $(OUTPUTSERV)
	$(RM) $(OUTPUTCLI)
	$(RM) $(call FIXPATH,$(OBJECTS))
	@echo Cleanup complete!

run: $(MAIN)
	./$(OUTPUTMAIN) 2
	@echo Executing 'run: main' complete!

run_serv: $(SERVER)
	./$(OUTPUTSERV) 5555
	@echo Executing 'run: server' complete!

run_cli: $(CLIENT)
	./$(OUTPUTCLIENT) 2
	@echo Executing 'run: client' complete!

