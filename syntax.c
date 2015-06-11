#include "syntax.h"

#define SynTrace if (1)
/* make_sequence() yields (sequence t1 t2). If t2 is already a sequence, fold
 * t1 into it. */
extern Cell *make_sequence(Cell *t1, Cell *t2) {
    Cell *r;
    if (!t1) return t2;
    if (!t2) return t1;
    if (list_headedP(t2, "sequence")) {
	t1 = cell_consM(t1, cell_cdr(t2));
	cell_cdr_set(t2, t1);
        SynTrace fprintf(stderr, "make_sequence(): %s\n", cell_asprint(t2));
	return t2;
    }
    r = cell_cons(t2, cell_nil);
    r = cell_cons(t1, r);
    r = cell_cons_string("sequence", r);
    SynTrace fprintf(stderr, "make_sequence(): %s\n", cell_asprint(r));
    return r;
}
