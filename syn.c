#include "syn.h"

#include "streq.h"

#define SynTrace if (1)

/* syn_sequence() yields (begin t1 t2). If t2 is already a sequence, fold t1
 * into it. */
extern Cell *syn_sequence(Cell *t1, Cell *t2) {
    Cell *r;
    if (!t1) return t2;
    if (!t2) return t1;
    if (list_headedP(t2, "begin")) {
	t1 = cell_consM(t1, cell_cdr(t2));
	cell_cdr_set(t2, t1);
        SynTrace fprintf(stderr, "syn_sequence(): %s\n", cell_asprint(t2));
	return t2;
    }
    r = cell_cons(t2, cell_nil);
    r = cell_cons(t1, r);
    r = cell_cons_string("begin", r);
    SynTrace fprintf(stderr, "syn_sequence(): %s\n", cell_asprint(r));
    return r;
}

Cell *syn_define(Cell *left, Cell *right) {
    Cell *r, *formals;

    SynTrace fprintf(stderr, "syn_define(): left is %s, right is %s\n",
            cell_asprint(left), cell_asprint(right));
    //if (!formalize(left)) return cell_nil; /* XXX! */
    // XXX not working: formals = cell_cleaveM(left);
    formals = cell_cleaveM(left);
    SynTrace fprintf(stderr, "syn_define(): name is %s, formals is %s\n",
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
        SynTrace fprintf(stderr, "syn_define(): c is %s\n", cell_asprint(c));
        if (streq(cell_car_string(c), "lambda")) {
            c = cell_cdr(c);
            if (!cell_car_cell(c)) {
                cell_car_set(c, formals); /* splice in the formals list */
                SynTrace fprintf(stderr, "syn_define(): right is %s\n",
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
    SynTrace fprintf(stderr, "syn_define(): result is %s\n", cell_asprint(r));
    return r;
}
/* The unquote() function handles the argument to the $ operator (which
 * is implicit for the first item in a command). XXX: there is a smell
 * of arbitrariness about some of this; it needs to be reconsidered
 * carefully. (In mitigation, unit_parse.c checks that the obvious cases
 * work OK.) */
Cell *unquote(Cell *c) {
    Cell *p;

    SynTrace fprintf(stderr, "unquote(%s)\n", cell_asprint(c));
    /* The obvious case: (quote x) ==> x */
    if (list_headedP(c, "quote"))
	return cell_cadr(c);

    if (cell_atomp(c))
        return syn_prefix("eval", c);

    /* Unquote a lambda expression by applying it. */
    if (list_headedP(c, "lambda"))
	if (!cell_cadr(c)) /* null args => closure */
	    return cell_cons(c, cell_nil);

    c = cell_cons(unquote(cell_car(c)), unquote(cell_cdr(c)));

    return c;
}

Cell *syn_command(Cell *c, Cell *l) {
    if (cell_nullp(c))
        return cell_nil;
    c = unquote(c);
    c = cell_cons(c, l);
    return c;
}

Cell *syn_prefix(char *p, Cell *c) {
    return cell_cons_string(p, cell_cons(c, cell_nil));
}

/* XXX this seems too "clever" */
Cell *syn_eval(Cell *c) {
    fprintf(stderr, "make_eval(): evaluand was %s\n", cell_asprint(c));
    if (list_headedP(c, "quote"))
        c = cell_cadr(c);
    else if (list_headedP(c, "lambda"))
        c = cell_cons(c, cell_nil);
    /* Parentheses after a $ imply grouping, not listage. For example,
     * "foobar=qux; echo $(foo^bar)" ==> qux */
    else if (list_headedP(c, "list"))
        cell_car_set_string(c, "eval");
    else
        c = syn_prefix("eval", c);
    fprintf(stderr, "make_eval(): evaluand is now %s\n", cell_asprint(c));
    return c;
}

Cell *syn_lambda(Cell *args, Cell *body) {
    Cell *r = cell_nil;

    fprintf(stderr, "make_lambda(): body is %s\n", cell_asprint(body));
    /* If the body is a thunk, we can recycle it. XXX: this *looks* like
     * a mere optimization, but in fact is definitely very important,
     * otherwise we end up with unevaluated (unapplied) thunks. Need to
     * check this out some more. Test case: "f = lambda (x) { echo $x };
     * f qux" */
    if (cell_stringp(body) && streq(cell_car_string(body), "lambda")) {
        Cell *c;
        c = cell_cdr(body);
        if (!cell_car_cell(c)) {
            cell_car_set(c, args); /* splice in the formals list */
    fprintf(stderr, "make_lambda(): body is %s\n", cell_asprint(body));
        }
        return body;
    }
    r = cell_cons(body, r);
    r = cell_cons(args, r);
    r = cell_cons_string("lambda", r);
    fprintf(stderr, "make_lambda(%s, %s) ==> %s\n",
            cell_asprint(args), cell_asprint(body), cell_asprint(r));
    return r;
}

Cell *syn_if(Cell *i, Cell *j, Cell *k) {
    return 0;
}
