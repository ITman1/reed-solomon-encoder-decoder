# Course:   Bezdrátové a mobilní sítě
# Project:  Projekt č.1: Kodér a dekodér pro chybové zabezpečení dat
# Author:   Radim Loskot, xlosko01@stud.fit.vutbr.cz
# Date:     Říjen 2013
# 
# Usage:
#	- make               compile project - release version
#	- make pack          packs all required files to compile this project    
#	- make clean         clean temp compilers files    
#	- make debug         builds in debug mode    
#	- make release       builds in release mode 
#

# Makefile for Cross Interleaved Reed Solomon encoder/decoder
#
# (c) Henry Minsky, Universal Access 1991-1996
#

PACKAGE_NAME=xlosko01
PACKAGE_FILES=Makefile bms1A.cpp bms1B.cpp berlekamp.c berlekamp.c ecc.h ecc.h rs.c galois.c crcgen.c

RANLIB = ranlib
AR = ar


VERSION = 1.0
DIRNAME= rscode-$(VERSION)

CXX = g++
CC = gcc
# OPTIMIZE_FLAGS = -O69
DEBUG_FLAGS = -g
CFLAGS = -Wall -Wstrict-prototypes  $(OPTIMIZE_FLAGS) $(DEBUG_FLAGS) -I..
CXXFLAGS = $(CXXOPT) -std=c++11 -Wall -pedantic -W
LDFLAGS = $(OPTIMIZE_FLAGS) $(DEBUG_FLAGS)

LIB_CSRC = rs.c galois.c berlekamp.c crcgen.c 
LIB_HSRC = ecc.h
LIB_OBJS = rs.o galois.o berlekamp.o crcgen.o 

TARGET_LIB = libecc.a
TEST_PROGS = bms1A bms1B

TARGETS = $(TARGET_LIB) $(TEST_PROGS)

all: $(TARGETS)

$(TARGET_LIB): $(LIB_OBJS)
	$(RM) $@exa
	$(AR) cq $@ $(LIB_OBJS)
	if [ "$(RANLIB)" ]; then $(RANLIB) $@; fi

bms1A: bms1A.o galois.o berlekamp.o crcgen.o rs.o
	g++ -o bms1A bms1A.o -L. -lecc

bms1B: bms1B.o galois.o berlekamp.o crcgen.o rs.o
	g++ -o bms1B bms1B.o -L. -lecc
	
clean:
	rm -f *.o bms1A bms1B libecc.a
	rm -f *~

dist:
	(cd ..; tar -cvf rscode-$(VERSION).tar $(DIRNAME))

depend:
	makedepend $(SRCS)

debug:
	make -B all CXXOPT="-g3 -O0"

release:
	make -B all CXXOPT=-O3

pack:
	zip $(PACKAGE_NAME).zip $(PACKAGE_FILES)

# DO NOT DELETE THIS LINE -- make depend depends on it.

