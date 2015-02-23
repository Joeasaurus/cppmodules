# The C++ compile we are using
CC = g++
# Our compiler flags
CFLAGS := -Wall -O0 -std=c++11

# Optionals
debug ?= no
modules ?= no
rebuild ?= no

# Both Macros
BMACROS :=
# Linux Macros
LMACROS :=
# Mac Macros
MMACROS := 
# Win Macros
WMACROS :=

# Any libraries we want to link into executable:
#   -llibname
# Both libs
BLIBS :=
# Linux libs
LLIBS := -ldl
# Mac libs
MLIBS :=
# Win libs
WLIBS :=

# Build env
BUILD_TREE = build

# Any directories containing header files other than /usr/include
#   -I/header/path
INCLUDES = -I./src

# Any library paths in addition to /usr/lib
#   -L/library/path
LIB_PATHS =

# All of our C++ source files
SRCS =

# OBJS we need to make
MAIN_OBJS = ${BUILD_TREE}/obj/main.o

# Modules
MODULE_OBJS = ${BUILD_TREE}/mod/base_module.mod

# The platforms avaliable to compile on
PLATS = linux

ifeq (${debug},yes)
	DEBUG := -g
else
	DEBUG :=
endif

ifeq (${modules},yes)
	MODULES = ${MODULE_OBJS}
else
	MODULES =
endif

ifeq (${rebuild},yes)
	REBUILD = clean
else
	REBUILD =
endif

ifeq (${MAKECMDGOALS},linux)
	BMACROS := ${BMACROS} ${LMACROS}
	BLIBS := ${BLIBS} ${LLIBS}
	ifeq (${modules},yes)
		MODULE_FLAGS = -fPIC -shared
	else
		MODULE_FLAGS =
	endif
endif


all:
	@echo Please specify your platform: [ ${PLATS} ]
	@echo -- Option Flags \(\"flag\"=yes\):
	@echo -- \"rebuild\": Set to yes to force a complete recompile
	@echp -- \"modules\": Build modules
	@echo -- \"debug\": Set to yes for a debug build

linux: ${REBUILD} .buildenv ${MAIN_OBJS} ${MODULES}
	${CC} ${BLIBS} ${BUILD_TREE}/obj/*.o -o ${BUILD_TREE}/release/main
	chmod +x ${BUILD_TREE}/release/main

# Definition for mainline objects, catching headers too
${BUILD_TREE}/obj/%.o: src/main/%.cpp src/main/%.hpp
	${CC} ${CFLAGS} ${DEBUG} ${INCLUDES} ${BMACROS} -c $< -o $@ 
# Same as above, but for objects without header files
${BUILD_TREE}/obj/%.o: src/main/%.cpp
	${CC} ${CFLAGS} ${DEBUG} ${INCLUDES} ${BMACROS} -c $< -o $@

# This should compile our modules in a cross platform manner!
${BUILD_TREE}/mod/%.mod: src/modules/%.cpp src/modules/%.hpp
	${CC} ${CFLAGS} ${MODULE_FLAGS} ${INCLUDES} $< -o $@
	cp -f $@ ${BUILD_TREE}/release/modules/

.PHONY: clean .buildenv
.buildenv:
	mkdir -p ${BUILD_TREE}/{obj,mod}
	mkdir -p ${BUILD_TREE}/release/modules	
clean:
	rm -rf ${BUILD_TREE}
