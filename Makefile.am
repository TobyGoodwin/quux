bin_PROGRAMS = quux

parse.c: parse.pacc
	pacc -d -ffeed.c -o $@ $<
parse.h: parse.c
feed.c: parse.c

quux_SOURCES = \
    cell.c \
    feed.c \
    main.c \
    syntax.c \
    unit_cell.c
