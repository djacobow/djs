CC=g++

SRCDIRS      = src

CSRCS        = src/djs.c
CPPSRCS      = src/test.cpp

EXECUTABLE   = testexe 
INC_DIRS     = -Isrc
LIB_DIRS     = 
LIBS         = -lpthread -lPocoNet -lPocoFoundation
CFLAGS       = -g -Wall -O0 -fno-rtti -std=c++11

LDFLAGS      = 

COBJS        = $(CSRCS:%.c=%.o)
CPPOBJS      = $(CPPSRCS:%.cpp=%.o)
OBJS         = $(COBJS) $(CPPOBJS)

all: $(EXECUTABLE) 

.PHONY: doxy

doxyfile.inc: Makefile
	echo INPUT = $(SRCDIRS) > doxyfile.inc
#	echo FILE_PATTERNS = src/*.h $(SRCS) >> doxyfile.inc

doxy: doxyfile.inc $(SRCS)
	doxygen Doxyfile.mk

$(EXECUTABLE): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o$(EXECUTABLE) $(LIB_DIRS) $(LIBS)

.cpp.o:
	$(CC) -c  $(CFLAGS) $(INC_DIRS) $< -o $@

.c.o:
	$(CC) -c  $(CFLAGS) $(INC_DIRS) $< -o $@

clean:
	rm -f $(EXECUTABLE) $(OBJS) doxyfile.inc 
	rm -rf html

