bin_PROGRAMS = quux

parse.c: parse.pacc
	pacc -d -ffeed.c -o $@ $<
parse.h: parse.c
feed.c: parse.c

quux_SOURCES = \
    cell.c \
    feed.c \
    input.c \
    main.c \
    parse.c \
    syntax.c

check_PROGRAMS = \
    unit_cell

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) tap-driver.sh
TESTS = unit_cell

unit_cell_SOURCES = \
    cell.c \
    unit_cell.c
