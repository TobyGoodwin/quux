/*
 * Copyright 2015 Tobold Jayne Goodwin <toby@paccrat.org>
 *
 * This file is part of quux.
 *
 * Quux is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at
 * your option) any later version.
 *
 * Quux is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with quux.  If not, see <http://www.gnu.org/licenses/>.
 */

/* XXX We need this for open_memstream (but the plan is to get rid of that) */
#define _XOPEN_SOURCE 700

#include <assert.h>
#include <gc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cell.h"
#include "streq.h"

/* A Cell can hold either a pair or an atom. If it holds a pair, there
 * is a type attached to both the car and the cdr. If it holds an atom,
 * the car_type is the type of the atom, and cdr_type is cell_atom.
 * Thus, if car.string == "foo", car_type == cell_string, cdr_type ==
 * cell_atom, the Cell holds the atom foo (the cdr value is immaterial).
 * But if car.string == "bar", car_type == cell_string, cdr.cell ==
 * cell_nil, cdr_type == cell_cell, the Cell holds the singleton list
 * (bar). Note that cdr_type is usually cell_atom or cell_cell,
 * otherwise we have an improper list.
 */

enum cell_type {
	cell_atom, cell_cell,
	cell_fxnum, cell_string,
	cell_invalid
};

struct Cell {
	union {
		char *string; /* first for cell_true */
		Cell *cell;
		long fxnum;
		void *generic;
	} car, cdr;
	enum cell_type car_type;
	enum cell_type cdr_type;
};

static long cell_count = 0;
void cell_report(void) {
    fprintf(stderr, "cell_count is %ld\n", cell_count);
}

/* constructors */
Cell *cell_nil = 0;
static Cell ct = { { "#t" }, 0, cell_string, cell_atom };
Cell *cell_true = &ct;

#if 0
Cell *cell_true(void) {
    static Cell t = { 0 };
    if (!t.car.string) {
	t.car_type = cell_string;
	t.car.string = "#t";
	t.cdr_type = cell_atom;
	t.cdr.generic = (void *)0xdeadbeef;
    }
    return &t;
}
#endif

Cell *cell_new_fxnum(long i) {
    Cell *c = GC_MALLOC(sizeof *c);
    ++cell_count;
    c->car_type = cell_fxnum;
    c->car.fxnum = i;
    c->cdr_type = cell_atom;
    c->cdr.generic = (void *)0xdeadbeef;
    return c;
}

Cell *cell_new_string(char *string) {
	Cell *c = GC_MALLOC(sizeof *c);

	++cell_count;
	c->car_type = cell_string;
	c->car.string = string;
	c->cdr_type = cell_atom;
	c->cdr.generic = (char *)0xdeadbeef; 
	return c;
}

Cell *cell_cons_string(char *string, Cell *cdr) {
	Cell *c = cell_new_string(string);

	c->cdr_type = cell_cell;
	c->cdr.cell = cdr;
	return c;
}

/* If we are consing an atom onto a list, we recycle the Cell. That cons
 * can sometimes avoid an allocation is the payoff for car sometimes
 * needing to make an allocation. However, in this case the semantics of
 * cons are not those in Lisp, since cell_cons() mutates its first
 * argument.  (Don't worry too much about this: it took me a long time
 * to realise the problem, as most uses of cell_cons() don't care.) If
 * the first argument is not an atom, we must allocate a new Cell as
 * usual.
 */
Cell *cell_consM(Cell *a, Cell *d) {
    if (a && a->cdr_type == cell_atom) {
	a->cdr_type = cell_cell;
	a->cdr.cell = d;
	return a;
    }
    return cell_cons(a, d);
}

Cell *cell_cons(Cell *car, Cell *cdr) {
    Cell *c = GC_MALLOC(sizeof *c);

    ++cell_count;
    if (car && car->cdr_type == cell_atom) {
	c->car_type = car->car_type;
	c->car.generic = car->car.generic;
    } else {
	c->car_type = cell_cell;
	c->car.cell = car;
    }
    c->cdr_type = cell_cell;
    c->cdr.cell = cdr;

    return c;
}

/* mutators */
void cell_car_set(Cell *c, Cell *a) {
    c->car_type = cell_cell;
    c->car.cell = a;
}

void cell_cdr_set(Cell *c, Cell *d) {
    c->cdr_type = cell_cell;
    c->cdr.cell = d;
}

void cell_car_set_fxnum(Cell *c, long i) {
    c->car_type = cell_fxnum;
    c->car.fxnum = i;
}

void cell_car_set_string(Cell *c, char *a) {
	c->car_type = cell_string;
	c->car.string = a;
}

/* accessors */
int cell_nullp(Cell *c) {
    return !c;
}

/* As in Scheme, nil is an atom (it is not a pair). */
int cell_atomp(Cell *c) {
    return !c || c->cdr_type == cell_atom;
}

int cell_pairp(Cell *c) {
    return !cell_atomp(c);
}

int cell_cellp(Cell *c) {
    return c && c->car_type == cell_cell;
}

int cell_fxnump(Cell *c) {
    return c && c->car_type == cell_fxnum;
}

int cell_stringp(Cell *c) {
    return c && c->car_type == cell_string;
}

/* Because types are not completely boxed, car may need to allocate a
 * new Cell and copy its result into it. This is a potentially expensive
 * operation...
 */
Cell *cell_car(Cell *c) {
    Cell *cell;

    if (c->car_type == cell_cell)
        return c->car.cell;
    cell = cell_nil;
    cell = GC_MALLOC(sizeof *cell);
    ++cell_count;
    cell->car_type = c->car_type;
    cell->car.generic = c->car.generic;
    cell->cdr_type = cell_atom;
    cell->cdr.generic = (void *)0xdeadbeef;
    return cell;
}

/* ... and so we provide additional accessors for when the result type
 * is known.
 */

long cell_car_fxnum(Cell *c) {
    assert(cell_fxnump(c));
    return c->car.fxnum;
}

char *cell_car_string(Cell *c) {
    assert(c->car_type == cell_string);
    return c->car.string;
}

Cell *cell_car_cell(Cell *c) {
    assert(c->car_type == cell_cell);
    return c->car.cell;
}

/* ... and the unusual cell_cleave(), which undoes a cons: it mutates
 * the input Cell to its own car, and returns its cdr. Thus it
 * corresponds to (let ((y (cdr x))) (set! x (car x)) y), which you
 * might be unlikely to say in Scheme. It's here because it can be done
 * efficiently, without allocating any new Cells.
 *
 * Unfortunately, it doesn't work, because it can't undo the "if" in
 * cell_cons. Need to think about this some more (or just abandon it).
 */
Cell *cell_cleaveM(Cell *c) {
    Cell *r;

    if (c->car_type == cell_cell) {
	c = cell_car_cell(c);
	r = cell_cdr(c);
    } else {
	r = cell_cdr(c);
	c->cdr_type = cell_atom;
	c->cdr.generic = (void *)0xdeadbeef;
    }
    return r;
}

Cell *cell_cdr(Cell *c) {
    assert(c->cdr_type == cell_cell);
    return c->cdr.cell;
}

Cell *cell_caar(Cell *c) { return cell_car(cell_car(c)); }
Cell *cell_cadr(Cell *c) { return cell_car(cell_cdr(c)); }
Cell *cell_cdar(Cell *c) { return cell_cdr(cell_car(c)); }
Cell *cell_cddr(Cell *c) { return cell_cdr(cell_cdr(c)); }
Cell *cell_cadar(Cell *c) { return cell_car(cell_cdr(cell_car(c))); }
Cell *cell_caddr(Cell *c) { return cell_car(cell_cddr(c)); }
Cell *cell_cdadr(Cell *c) { return cell_cdr(cell_cadr(c)); }
Cell *cell_cadadr(Cell *c) { return cell_car(cell_cdadr(c)); }
Cell *cell_cadddr(Cell *c) { return cell_car(cell_cdr(cell_cddr(c))); }

#if 0
Cell *cell_caadr(Cell *c0) {
    Ref(Cell *, c, cell_cdr(c0));
    c = cell_car(c);
    c = cell_car(c);
    RefReturn(c);
}

Cell *cell_cdaadr(Cell *c) { return cell_cdr(cell_caadr(c)); }

/* extra accessors for gc */
int _cell_car_t(Cell *c) {
	return c->car_type;
}
int _cell_cdr_t(Cell *c) {
	return c->cdr_type;
}
void *_cell_car_ptr(Cell *c) {
	return c->car.generic;
}
void *_cell_cdr_ptr(Cell *c) {
	return c->cdr.generic;
}

size_t _cell_size(void) {
	return sizeof (Cell);
}

/* temporaries to facilitate changeover */
List *cell_to_list(Cell *c) {
	List *l;
	if (!c) return NULL;
	Ref(Cell *, cell, c);
	Ref(Qlist *, q, qlist_new_empty());

	switch (cell->car_type) {
	case cell_fxnum:
	    Ref(char *, v, str("%d", cell->car.fxnum));
	    Ref(Term *, t, mkstr(v));
	    qlist_append_term(q, t);
	    RefEnd2(t, v);
	    break;
	case cell_string:
		Ref(Term *, t, mkstr(cell->car.string));
		qlist_append_term(q, t);
		RefEnd(t);
		break;
	case cell_cell:
		fprint(2, "warning: flattening list\n");
		Ref(List *, l2, cell_to_list(cell->car.cell));
		qlist_append_list(q, l2);
		RefEnd(l2);
		break;
	case cell_closure:
		Ref(Term *, t, mkterm(NULL, cell->car.closure));
		qlist_append_term(q, t);
		RefEnd(t);
		break;
	}
	switch (cell->cdr_type) {
	case cell_atom: /* singleton list */
		break;
	case cell_cell:
		Ref(List *, l2, cell_to_list(cell->cdr.cell));
		qlist_append_list(q, l2);
		RefEnd(l2);
		break;
	default:
		fprint(2, "improper list\n");
		assert(0);
	}
	l = q->list;
	RefEnd2(q, cell);
	return l;
}

Cell *cell_from_list(List *l) {
	if (!l) return cell_nil;
	Ref(Cell *, c, cell_nil);
	Ref(List *, list, l);
	Ref(Term *, term, list->term);
	if (isclosure(term))
		c = cell_new_closure(getclosure(term));
	else
		c = cell_new_string(getstr(term));
	c = cell_consM(c, cell_from_list(list->next));
	RefEnd2(term, list);
	RefReturn(c);
}

#if 0
static Queue *queue_from_tree(Tree *t0) {
    Ref(Queue *, q, queue_nil);
    Ref(Tree *, t, t0);
    q = queue_new_empty();
    if (t)
	switch (t->kind) {
	case nWord: case nQword:
	    Ref(Cell *, c, cell_new_string(t->u[0].s));
	    queue_snoc(q, c);
	    RefEnd(c);
	    break;
	case nList:
	    Ref(Cell *, l, cell_from_tree(t->u[0].p));
	    Ref(Cell *, r, cell_from_tree(t->u[1].p));
fprint(2, "queue_from_tree(): l is >%O<\n", l);
fprint(2, "queue_from_tree(): r is >%O<\n", r);
	    if (!cell_atomp(l)) l = cell_cons(l, cell_nil);
	    queue_append_cell(q, l);
	    queue_append_cell(q, r);
	    RefEnd2(r, l);
	    break;
	}
    RefEnd(t);
    RefReturn(q);
}
#endif

Cell *cell_from_tree_list(Tree *);

/* cell_from_tree() handles complete parse trees: it is used for turning
 * es code into a Cell * structure to be evaluated in the vm. At some
 * point, we will have to fix the parser to return the Cell * structure
 * directly.
 */
Cell *cell_from_tree(Tree *t0) {
    Cell *result;
    if (!t0) return cell_nil;
    Ref(Tree *, t, t0);
    switch (t->kind) {
    case nAssign:
	Ref(Cell *, l, cell_from_tree(t->u[0].p));
	Ref(Cell *, r, cell_from_tree(t->u[1].p));
	result = cell_cons(r, cell_nil);
	result = cell_cons(l, result);
	result = cell_cons_string("define", result);
	RefEnd2(r, l);
	break;
    case nConcat:
	Ref(Cell *, l, cell_from_tree(t->u[0].p));
	Ref(Cell *, r, cell_from_tree(t->u[1].p));
	result = cell_cons(r, cell_nil);
	result = cell_cons(l, result);
	result = cell_cons_string("concat", result);
	RefEnd2(r, l);
	break;
    case nLambda:
	Ref(Cell *, l, cell_from_tree_list(t->u[0].p));
	Ref(Cell *, r, cell_from_tree(t->u[1].p));
	result = cell_cons_string("apply", r);
	result = cell_cons(result, cell_nil);
fprint(2, "queue_from_tree(): result is >%O<\n", result);
	result = cell_cons(l, result);
	result = cell_cons_string("lambda", result);
	RefEnd2(r, l);
	break;
    case nList:
	Ref(Cell *, l, cell_from_tree(t->u[0].p));
	Ref(Cell *, r, cell_from_tree(t->u[1].p));
	//fprint(2, "queue_from_tree(): l is >%O<\n", l);
	//fprint(2, "queue_from_tree(): r is >%O<\n", r);
	if (cell_nullp(l) || cell_pairp(l)) l = cell_cons(l, cell_nil);
	Ref(Queue *, q, queue_new_empty());
	queue_append_cell(q, l);
	queue_append_cell(q, r);
	result = queue_cell(q);
	RefEnd(q);
	RefEnd2(r, l);
	break;
    case nQword:
	result = cell_cons_string(t->u[0].s, cell_nil);
	result = cell_cons_string("qword", result);
	break;
    case nVar:
	Ref(Cell *, v, cell_from_tree(t->u[0].p));
	v = cell_cons(v, cell_nil);
	result = cell_cons_string("lookup-var", v);
	RefEnd(v);
	break;
    case nWord:
	/* ooh, here's a horrid hack */
	if (streq(t->u[0].s, "apply")) {
	    result = cell_new_string(t->u[0].s);
	} else {
	    result = cell_cons_string(t->u[0].s, cell_nil);
	    result = cell_cons_string("word", result);
	}
	break;
    default:
	result = cell_nil;
    }
    RefEnd(t);
    return result;
}

/* Certain nodes in the tree, corresponding to "params" productions in
 * the grammar, are simply a list of words. For example, the parameter
 * list of a lambda.
 */ 
Cell *cell_from_tree_list(Tree *t0) {
    Cell *result;
    if (!t0) return cell_nil;
    Ref(Tree *, t, t0);
    switch (t->kind) {
    case nList:
	Ref(Cell *, l, cell_from_tree_list(t->u[0].p));
	Ref(Cell *, r, cell_from_tree_list(t->u[1].p));
fprint(2, "cell_from_tree_list(): l is >%O<\n", l);
fprint(2, "cell_from_tree_list(): r is >%O<\n", r);
	if (cell_nullp(l) || cell_pairp(l)) l = cell_cons(l, cell_nil);
	Ref(Queue *, q, queue_new_empty());
	queue_append_cell(q, l);
	queue_append_cell(q, r);
	result = queue_cell(q);
	RefEnd(q);
	RefEnd2(r, l);
	break;
    case nQword:
	result = cell_new_string(t->u[0].s);
	break;
    case nWord:
	result = cell_new_string(t->u[0].s);
	break;
    default:
	assert(0);
    }
    RefEnd(t);
    return result;
}

/* list_enlist(): ensure that a Cell is a list */
Cell *list_enlist(Cell *c0) {
    if (!c0) return c0;
    if (cell_pairp(c0)) return c0;
    Ref(Cell *, c, c0);
    c = cell_cons(c, cell_nil);
    RefReturn(c);
}

/* flatten a list, (foo (bar baz) qux) => (foo bar baz qux) */
Cell *list_flatten(Cell *c0) {
    /* this is not an efficient implementation! */
    return cell_from_list(cell_to_list(c0));
}

/* list_length(): guess what? */
int list_length(Cell *c) {
    int r = 0;
    for ( ; !cell_nullp(c); c = cell_cdr(c))
	++r;
    return r;
}
#endif

/* non-mutating append */
Cell *list_append(Cell *a, Cell *b) {
    Cell *r = cell_nil;
    Cell *c;

    if (cell_nullp(a)) return b;

    /* XXX: these atom tests seem pretty grody */
    c = cell_atomp(a) ? a : cell_car(a);
    r = cell_consM(c, list_append(cell_atomp(a) ? cell_nil : cell_cdr(a), b));
    return r;
}

/* mutating append */
void list_appendM(Cell *a, Cell *b) {
    assert(a); /* can't mutate ()! */
    while (cell_cdr(a))
	a = cell_cdr(a);
    cell_cdr_set(a, b);
}

Cell *cell_assoc(Cell *k, Cell *l) {
    if (cell_nullp(l))
        return cell_nil;
    if (streq(cell_car_string(k), cell_car_string(cell_car(l))))
        return cell_cdar(l);
    return cell_assoc(k, cell_cdr(l));
}

Cell *list_headedP(Cell *c, char *h) {
    if (!cell_nullp(c) && cell_pairp(c) && cell_stringp(c) &&
	    streq(cell_car_string(c), h))
	return cell_true;
    return cell_nil;
}

#if 0
/* list_reverse() builds a new list, and - while it is O(n) in time - is
 * therefore somewhat inefficient. We could make a much quicker
 * destructive reverse using cell_cleave()...
 */
Cell *list_reverse(Cell *c0) {
    Ref(Cell *, r, cell_nil);
    Ref(Cell *, c, c0);
//fprint(2, "reverse(): input is %O\n", c);
    for ( ; c; c = cell_cdr(c)) {
	Ref(Cell *, a, cell_car(c));
	r = cell_cons(a, r);
	RefEnd(a);
    }
    RefEnd(c);
//fprint(2, "reverse(): result is %O\n", r);
    RefReturn(r);
}

Cell *list_singletonP(Cell *c) {
    if (cell_pairp(c) && !cell_cdr(c))
	return cell_true;
    return cell_nil;
}
#endif

int cell_fprint(FILE *f, Cell *c) {
    int atom = cell_atomp(c);

    Cell *next = cell_nil;
    if (!atom) fprintf(f, "(");
    for (; !cell_nullp(c); c = next) {
	Cell *car;
	/* Nasty hack. This function cannot print circular lists. In
	 * general, these occur only within closures, so punt on
	 * printing them. XXX make it print all but the environment
	 * part. */
	if (list_headedP(c, "closure")) {
	    //fmtprint(f, "-closure %O-", cell_cadadr(c));
	    fprintf(f, "-closure-");
	    break;
	}
	car = cell_car(c);
	if (cell_atomp(car)) {
	    if (cell_nullp(car))
		fprintf(f, "()");
	    else if (cell_fxnump(c))
		fprintf(f, "%d", cell_car_fxnum(c));
	    else if (cell_stringp(c))
		fprintf(f, "%s", cell_car_string(c));
	    else
		fprintf(f, "[unknown]");
	} else {
	    cell_fprint(f, car);
	}
	if (cell_atomp(c))
	    next = cell_nil;
	else
	    next = cell_cdr(c);
	if (!cell_nullp(next)) {
	    if (cell_pairp(next))
		fprintf(f, " ");
	    else
		fprintf(f, " . ");
	}
    }
    if (!atom) fprintf(f, ")");

    return 0;
}

/* XXX this isn't a completely crazy way to do it, but it has the snag that we
 * end up with a string allocated by malloc(). We could, and perhaps should,
 * copy it into a gc-allocated string. But in the long run it should be
 * reimplemented without the stream abstraction. */
char *cell_asprint(Cell *c) {
    char *buf, *gcbuf;
    size_t size;
    FILE *s = open_memstream(&buf, &size);
    cell_fprint(s, c);
    fclose(s);
    gcbuf = GC_MALLOC(size + 1);
    memcpy(gcbuf, buf, size + 1);
    free(buf);
    return gcbuf;
}

int cell_fprint_flat(FILE *f, Cell *c, char *sep) {
    Cell *next;

    for (; c; c = next) {
        next = cell_cdr(c);
        fprintf(f, "%s%s", cell_car_string(c), next ? sep : "");
    }
}

char *cell_asprint_flat(Cell *c, char *sep) {
    char *buf;
    size_t size;
    FILE *s = open_memstream(&buf, &size);
    cell_fprint_flat(s, c, sep);
    fclose(s);

    return buf;
}
