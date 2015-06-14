bin_PROGRAMS = quux

parse.c: parse.pacc
	pacc -d -ffeed.c -o $@ $<
parse.h: parse.c
feed.c: parse.c

quux_SOURCES = \
    assemble.c \
    cell.c \
    env.c \
    external.c \
    feed.c \
    input.c \
    internal.c \
    main.c \
    parse.c \
    syn.c \
    tree.c \
    vm.c

check_PROGRAMS = \
    unit_cell \
    unit_env \
    unit_tree

TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) tap-driver.sh
TESTS = unit_cell unit_env unit_tree

unit_cell_SOURCES = cell.c unit_cell.c
unit_env_SOURCES = cell.c check.c env.c unit_env.c
unit_tree_SOURCES = cell.c check.c tree.c unit_tree.c
