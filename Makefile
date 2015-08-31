CC=g++

SRCDIRS      = src

CSRCS        = src/djs.c

INC_DIRS     = -Isrc
LIB_DIRS     = 
LIBS         = 
CFLAGS       = -g -Wall -Os -fno-rtti -std=c++11

LDFLAGS      = 

COBJS        = $(CSRCS:%.c=%.o)

all: testexe example

.PHONY: doxy

doxyfile.inc: Makefile
	echo INPUT = $(SRCDIRS) > doxyfile.inc
#	echo FILE_PATTERNS = src/*.h $(SRCS) >> doxyfile.inc

doxy: doxyfile.inc $(SRCS)
	doxygen Doxyfile.mk

testexe: $(COBJS) src/test.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(COBJS) src/test.o -o testexe $(LIB_DIRS) $(LIBS)

example: $(COBJS) src/example.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(COBJS) src/example.o -o example $(LIB_DIRS) $(LIBS)

.cpp.o:
	$(CC) -c  $(CFLAGS) $(INC_DIRS) $< -o $@

.c.o:
	$(CC) -c  $(CFLAGS) $(INC_DIRS) $< -o $@

clean:
	rm -f testexe example $(COBJS) src/test.o src/example.o doxyfile.inc 
	rm -rf html

