bin_PROGRAMS = quux

parse.c: parse.pacc
	pacc -d -ffeed.c -o $@ $<
parse.h: parse.c
feed.c: parse.c

quux_SOURCES = \
    assemble.c \
    cell.c \
    env.c \
    feed.c \
    input.c \
    main.c \
    parse.c \
    syn.c \
    vm.c

check_PROGRAMS = \
    unit_cell

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) tap-driver.sh
TESTS = unit_cell

unit_cell_SOURCES = \
    cell.c \
    unit_cell.c
