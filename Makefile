# The C++ compile we are using
CC = g++
# Our compiler flags
CFLAGS := -Wall -O3 -std=c++11

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
BLIBS := -lboost_filesystem -lboost_system
# Linux libs
LLIBS := -ldl
# Mac libs
MLIBS := -lzmq
# Win libs
WLIBS :=

# Linker flags
# Both
BLFLAGS :=
# Linux
LLFLAGS :=
# Mac
MLFLAGS :=
# Win
# We need to statically link stdlibc++ so stop dll mismatch!
WLFLAGS := -static-libstdc++

# Build env
BUILD_TREE = build

# Any directories containing header files other than /usr/include
#   -I/header/path
INCLUDES = -I./src

# Any library paths in addition to /usr/lib
#   -L/library/path
LIB_PATHS =

# OBJS we need to make
MAIN_OBJS = ${BUILD_TREE}/obj/spine.o ${BUILD_TREE}/obj/main.o

# Modules
MODULE_OBJS = ${BUILD_TREE}/mod/base_module.so

# The platforms available to compile on
PLATS = linux macosx mingw

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
	REBUILD = clean_objs
else
	REBUILD =
endif

# You are expected to never ask for more than one target!!
ifeq (${MAKECMDGOALS},linux)
	BMACROS := ${BMACROS} ${LMACROS}
	BLIBS := ${BLIBS} ${LLIBS}
	BLFLAGS := ${BLFLAGS} ${LLFLAGS}
	ifeq (${modules},yes)
		MODULE_FLAGS = -fPIC -shared
	else
		MODULE_FLAGS =
	endif
endif
ifeq (${MAKECMDGOALS},macosx)
	BMACROS := ${BMACROS} ${MMACROS}
	BLIBS := ${BLIBS} ${MLIBS}
	BLFLAGS := ${BLFLAGS} ${MLFLAGS}
	ifeq (${modules},yes)
		MODULE_FLAGS = -flat_namespace -dynamiclib
	else
		MODULE_FLAGS =
	endif
endif
ifeq (${MAKECMDGOALS},mingw)
	BMACROS := ${BMACROS} ${WMACROS}
	BLIBS := ${BLIBS} ${WLIBS}
	BLFLAGS := ${BLFLAGS} ${WLFLAGS}
	ifeq (${modules},yes)
		MODULE_FLAGS = -shared
	else
		MODULE_FLAGS =
	endif
endif

# Some help text for 'all'
all:
	@echo Please specify your platform: [ ${PLATS} ]
	@echo -- Option Flags \(flag=yes\):
	@echo -- -- rebuild: Set to yes to force a complete recompile
	@echo -- -- modules: Set to yes to build modules
	@echo -- -- debug: Set to yes for a debug build

# Every target has the same logic, just different options!
linux: ${REBUILD} .buildenv ${MAIN_OBJS} ${MODULES}
	${CC} ${CFLAGS} ${BLIBS} ${BLFLAGS} ${BUILD_TREE}/obj/*.o -o ${BUILD_TREE}/release/main
	chmod +x ${BUILD_TREE}/release/main*
macosx: linux
mingw: linux

# Definition for mainline objects, catching headers too
${BUILD_TREE}/obj/%.o: src/main/%.cpp src/main/%.hpp
	${CC} ${CFLAGS} ${DEBUG} ${INCLUDES} ${BMACROS} -c $< -o $@
# Same as above, but for objects without header files
${BUILD_TREE}/obj/%.o: src/main/%.cpp
	${CC} ${CFLAGS} ${DEBUG} ${INCLUDES} ${BMACROS} -c $< -o $@

# This should compile our modules to dynamic libraries
${BUILD_TREE}/mod/%.so: src/modules/%.cpp src/modules/%.hpp
	${CC} ${CFLAGS} ${BLIBS} ${BLFLAGS} ${MODULE_FLAGS} ${INCLUDES} $< -o $@
	cp -f $@ ${BUILD_TREE}/release/modules/

.PHONY: clean_objs clean_all .buildenv
.buildenv:
	mkdir -p ${BUILD_TREE}/{obj,mod}
	mkdir -p ${BUILD_TREE}/release/modules
clean_objs:
	rm -rf ${BUILD_TREE}/{obj,mod}
clean_all:
	rm -rf ${BUILD_TREE}
