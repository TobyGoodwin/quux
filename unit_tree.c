#include <gc.h>
#include <stdio.h>
#include <stdlib.h>

#include "cell.h"
#include "check.h"
#include "streq.h"
#include "tree.h"

void test_flatten(void) {
    Cell *c, *t;
    char *s;
        
    c = cell_nil;
    check(cell_nullp(tree_flatten(c)));

    c = cell_cons_string("one", cell_nil);
    t = tree_flatten(c);
    s = cell_asprint(t);
    fprintf(stderr, "tree_flatten %s ==> %s\n", cell_asprint(c), s);
    check(streq(s, "(one)"));

    c = cell_cons_string("two", c);
    t = tree_flatten(c);
    s = cell_asprint(t);
    fprintf(stderr, "tree_flatten %s ==> %s\n", cell_asprint(c), s);
    check(streq(s, "(two one)"));

    c = cell_cons(c, cell_nil);
    t = tree_flatten(c);
    s = cell_asprint(t);
    fprintf(stderr, "tree_flatten %s ==> %s\n", cell_asprint(c), s);
    check(streq(s, "(two one)"));

    c = cell_cons_string("three", c);
    t = tree_flatten(c);
    s = cell_asprint(t);
    fprintf(stderr, "tree_flatten %s ==> %s\n", cell_asprint(c), s);
    check(streq(s, "(three two one)"));

    c = cell_cons(c, c);
    t = tree_flatten(c);
    s = cell_asprint(t);
    fprintf(stderr, "tree_flatten %s ==> %s\n", cell_asprint(c), s);
    check(streq(s, "(three two one three two one)"));

}

/* construct the flat list (foo bar baz qux) */
Cell *fbbq(void) {
    Cell *c3 = cell_nil;
    Cell *c0 = cell_cons_string("qux", cell_nil);
    Cell *c1 = cell_cons_string("baz", c0);
    Cell *c2 = cell_cons_string("bar", c1);
    c3 = cell_cons_string("foo", c2);
    return c3;
}

int main(void) {
    printf("1..27\n");
    test_flatten();
}
