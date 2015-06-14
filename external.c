#include <gc.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "external.h"
#include "list.h"
#include "tree.h"

Cell *external(Cell *cell) {
    char **argv, *cmd;
    int argc, i;
    pid_t p, w;

    cell = tree_flatten(cell);
    cmd = cell_car_string(cell);
    fprintf(stderr, "external command: %O\n", cell);

    argc = list_length(cell);
    argv = GC_MALLOC(sizeof (char *) * (argc + 1));
    for (i = 0; i < argc; ++i) {
	argv[i] = cell_car_string(cell);
	cell = cell_cdr(cell);
    }
    argv[i] = 0;

    p = fork();
    switch (p) {
    case -1:
	fprintf(stderr, "fork failed!\n");
	break;
    case 0:
	execve(cmd, argv, 0);
	fprintf(stderr, "exec failed!\n");
	_exit(1);
    default:
	do {
	    /* One day we'll have to tackle wait() properly. Eugh. */
	    w = wait(0);
	} while (w != p);
	break;
    }
    /* And, of course, return what wait() said. */
    return cell_nil;
}

