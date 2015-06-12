#include "syntax.h"

#include "streq.h"

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

Cell *make_define(Cell *left, Cell *right) {
    Cell *r, *formals;

    SynTrace fprintf(stderr, "make_define(): left is %s, right is %s\n",
            cell_asprint(left), cell_asprint(right));
    //if (!formalize(left)) return cell_nil; /* XXX! */
    // XXX not working: formals = cell_cleaveM(left);
    formals = cell_cleaveM(left);
    SynTrace fprintf(stderr, "make_define(): name is %s, formals is %s\n",
            cell_asprint(left), cell_asprint(formals));
    //name = unquote(name);
    r = right;
    /* If there is more than one item on the RHS, we need to wrap a %list
     * around it. */
    if (cell_cdr(r))
        r = cell_cons(cell_cons_string("list", r), cell_nil);
    if (formals) {
        /* f x = { echo $x }
           If the RHS is a single {body} construct, we can recycle its
           lambda. */
        Cell *c;
        c = cell_car(r);
        SynTrace fprintf(stderr, "make_define(): c is %s\n", cell_asprint(c));
        if (streq(cell_car_string(c), "lambda")) {
            c = cell_cdr(c);
            if (!cell_car_cell(c)) {
                cell_car_set(c, formals); /* splice in the formals list */
                SynTrace fprintf(stderr, "make_define(): right is %s\n",
                        cell_asprint(right));
            }
        } else {
            r = cell_cons(formals, r);
            r = cell_cons_string("lambda", r);
            r = cell_cons(r, cell_nil);
        }
    }
    r = cell_cons(left, r);
    r = cell_cons_string("define", r);
    SynTrace fprintf(stderr, "make_define(): result is %s\n", cell_asprint(r));
    return r;
}

Cell *make_command(Cell *c, Cell *l) {
    return 0;
}

Cell *prefix(const char *p, Cell *c) {
    return cell_cons_string(p, cell_cons(c, cell_nil));
}

Cell *make_eval(Cell *e) {
    return 0;
}
Cell *make_lambda(Cell *l, Cell *c) {
    return 0;
}
Cell *make_if(Cell *i, Cell *j, Cell *k) {
    return 0;
}
