#include <stdio.h>
#include <stdlib.h>

#include "cell.h"
#include "streq.h"

int n = 1;

void check(int x) {
    if (!x) printf("not ");
    printf("ok %d\n", n++);
}

/* the nil cell */
void test0(void) {
    Cell *c = cell_nil;
    check(cell_nullp(c));
}

void test1(void) {
    char *a = "Hunoze?";
    Cell * c = cell_new_string(a);
    check(cell_atomp(c));
    check(streq(cell_car_string(c), "Hunoze?"));
}

void test2(void) {
    char *s;
    Cell *c0 = cell_cons_string("bar", cell_nil);
    Cell *c1 = cell_cons_string("foo", c0);
    check(!cell_atomp(c1));
    check(streq(cell_car_string(c1), "foo"));
    check(!cell_atomp(c0));
    check(streq(cell_car_string(c0), "bar"));
    check(cell_cdr(c0) == cell_nil);
    c0 = cell_cleaveM(c1);
    check(cell_pairp(c0));
    check(cell_atomp(c1));

    s = cell_asprint(c1);
    printf("c1 is %s\n", s);
    check(streq(s, "foo"));
    free(s);

    s = cell_asprint(c0);
    printf("c0 is %s\n", s);
    check(streq(s, "(bar)"));
    free(s);
#if 0
    s = str("%O", c1);
    print("%s\n", s);
    check(streq(s, "foo"));

    s = str("%O", c0);
    print("%s\n", s);
    check(streq(s, "(bar)"));
#endif

}

#if 0
void test3(void) {
	char *a2, *a3;
	Cell *c2;
	Ref(char *, a0, "baz");
	Ref(char *, a1, "qux");
	Ref(Cell *, c0, cell_new_atom(a1));
	Ref(Cell *, c1, cell_cons_atom(a0, c0));
	gc(); memdump();

	assert(cell_car_t(c1) == cell_atom);
	a2 = cell_car_atom(c1);
	assert(streq(a2, "baz"));
	assert(cell_cdr_t(c1) == cell_cell);
	c2 = cell_cdr_cell(c1);
	assert(cell_car_t(c2) == cell_atom);
	a3 = cell_car_atom(c2);
	assert(streq(a3, "qux"));
	assert(cell_cdr_t(c2) == cell_nil);
	RefEnd4(c1, c0, a1, a0);
}
#endif

void test3(void) {
    Cell *c0 = cell_cons_string("bar", cell_nil);
    Cell *c1 = cell_cons_string("foo", c0);
    check(cell_pairp(c0));
    check(cell_pairp(c1));
    check(streq(cell_car_string(c1), "foo"));
    check(cell_atomp(cell_car(c1)));
    check(streq(cell_car_string(cell_car(c1)), "foo"));
}

#if 0
/* construct the flat list (foo bar baz qux) */
Cell *fbbq(void) {
	Ref(Cell *, c3, cell_nil);
	Ref(Cell *, c0, cell_cons_string("qux", cell_nil));
	Ref(Cell *, c1, cell_cons_string("baz", c0));
	Ref(Cell *, c2, cell_cons_string("bar", c1));
	c3 = cell_cons_string("foo", c2);
	RefEnd3(c2, c1, c0);
	RefReturn(c3);
}

void test4(void) {
	Ref(Cell *, c, fbbq());

	gc();

	Ref(char *, s, str("%M", c, " "));
	assert(streq(s, "foo bar baz qux"));
	RefEnd2(s, c);

}

void test5(void) {
	char *s;
	Ref(Cell *, c0, cell_cons_string("qux", cell_nil));
	Ref(Cell *, c1, cell_cons_string("baz", c0));
	Ref(Cell *, c2, cell_cons_string("bar", cell_nil));
	Ref(Cell *, c3, cell_cons_string("foo", c2));
	Ref(Cell *, c4, cell_consM(c3, c1));

	gc();

	s = str("%O", c1, " ");
	print("%s\n", s);
	assert(streq(s, "(baz qux)"));

	s = str("%O", c4, " ");
	print("%s\n", s);
	assert(streq(s, "((foo bar) baz qux)"));

	RefEnd(c4);
	RefEnd4(c3, c2, c1, c0);
}

void test5a(void) {
	char *s;
	Ref(Cell *, c0, cell_cons_string("qux", cell_nil));
	Ref(Cell *, c1, cell_cons_string("baz", c0));
	Ref(Cell *, c2, cell_cons_string("bar", cell_nil));
	Ref(Cell *, c3, cell_cons_string("foo", c2));
	Ref(Cell *, c4, cell_cons(c1, cell_nil));
	Ref(Cell *, c5, cell_cons(c3, c4));

	s = str("%O", c1, " ");
	print("%s\n", s);
	assert(streq(s, "(baz qux)"));

	s = str("%O", c5, " ");
	print("%s\n", s);
	assert(streq(s, "((foo bar) (baz qux))"));

	RefEnd2(c5, c4);
	RefEnd4(c3, c2, c1, c0);
}

void test6(void) {
	Ref(Cell *, c, fbbq());
	Ref(List *, l, cell_to_list(c));
	print("%L\n", l, " ");

	Ref(char *, s, str("%L", l, " "));
	assert(streq(s, "foo bar baz qux"));
	RefEnd(s);

	gc(); memdump();

	Ref(Cell *, c1, cell_from_list(l));
	Ref(char *, s, str("%M", c1, " "));
	assert(streq(s, "foo bar baz qux"));
	RefEnd2(s, c1);

	RefEnd2(l, c);
}

void test7(void) {
    char *s;
    Ref(Cell *, c0, cell_cons_string("qux", cell_nil));
    Ref(Cell *, c1, cell_cons_string("baz", cell_nil));
    Ref(Cell *, c2, cell_cons_string("bar", c1));
    Ref(Cell *, c3, cell_cons(c2, c0));
    Ref(Cell *, c4, cell_cons_string("foo", c3));
    s = str("%O", c4);
    print("%s\n", s);
    assert(streq(s, "(foo (bar baz) qux)"));
    RefEnd(c4);
    RefEnd4(c3, c2, c1, c0);
}

void test8(void) {
    char *s;
    Ref(Cell *, c0, fbbq());
    Ref(Cell *, c1, cell_cons_string("vark", cell_nil));
    Ref(Cell *, c2, cell_cons_string("aard", c1));
    Ref(Cell *, c3, list_append(c0, c2));

    s = str("%O", c3, " ");
    print("%s\n", s);
    assert(streq(s, "(foo bar baz qux aard vark)"));
    RefEnd4(c3, c2, c1, c0);
}

void test9(void) {
    char *s;
    Ref(Cell *, c, cell_new_fxnum(42));;
    assert(cell_fxnump(c));;
    assert(cell_car_fxnum(c) == 42);
    Ref(Cell *, l, cell_cons(c, cell_nil));
    s = str("%O", l);
    print("%s\n", s);
    assert(streq(s, "(42)"));
    RefEnd2(l, c);
}

#endif

void unit_cell(void) {
    printf("1..10\n");
    test0();
    test1();
    test2();
}
