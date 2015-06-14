#include <stdio.h>

#include "cell.h"
#include "check.h"
#include "list.h"
#include "streq.h"

#if 0
void test_append(void) {
    char *s;
    Ref(Cell *, c0, fbbq());
    Ref(Cell *, c1, fbbq());

    list_appendM(c0, c1);
    s = str("%O", c0);
    print("%s\n", s);
    assert(streq(s, "(foo bar baz qux foo bar baz qux)"));
    RefEnd2(c1, c0);
}
#endif

void test_length(void) {
    Cell *c;

    c = cell_nil;
    check(list_length(c) == 0);

    c = cell_cons_string("one", c);
    check(list_length(c) == 1);

    c = cell_cons(c, cell_nil);
    check(list_length(c) == 1);

    c = cell_cons_string("two", c);
    check(list_length(c) == 2);
}

void test_reverse(void) {
    Cell *c, *r;
    char *s;

    c = cell_nil;
    check(cell_nullp(list_reverse(c)));

    c = cell_cons_string("one", c);
    r = list_reverse(c);
    s = cell_asprint(r);
    fprintf(stderr, "list_reverse %s ==> %s\n", cell_asprint(c), s);
    check(streq(s, "(one)"));

    c = cell_cons_string("two", c);
    r = list_reverse(c);
    s = cell_asprint(r);
    fprintf(stderr, "list_reverse %s ==> %s\n", cell_asprint(c), s);
    check(streq(s, "(one two)"));

    c = cell_cons(c, cell_nil);
    c = cell_cons_string("three", c);
    r = list_reverse(c);
    s = cell_asprint(r);
    fprintf(stderr, "list_reverse %s ==> %s\n", cell_asprint(c), s);
    check(streq(s, "((two one) three)"));
}


int main(void) {
    printf("1..4\n");
    test_length();
    test_reverse();
    //test_append();
    return 0;
}
