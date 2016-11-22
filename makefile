##
## makefile for dsort
##
## @author Ulrike Schaefer 1327450
##
## @date 22.11.2016
##
##

CC = gcc
DEFS = -D_XOPEN_SOURCE=500 -D_BSD_SOURCE
CFLAGS = -Wall -g -std=c99 -pedantic $(DEFS)

.PHONY: all clean

all: dsort

dsort: dsort.o

$.o: $.c
	$( CC ) $( CFLAGS ) -c -o $@ $<

clean:
	rm -f dsort dsort.o

