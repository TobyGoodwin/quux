#include "check.h"
#include "env.h"
#include "streq.h"

void test0(void) {
    Cell *n = cell_new_string("foo");
    Cell *v = cell_new_string("bar");
    Cell *e = env_frame(cell_nil, cell_nil, cell_nil);
    env_bind(e, n, v);
    printf("$foo ==> %s\n", cell_asprint(env_lookup(e, n)));
    check(streq(cell_car_string(env_lookup(e, n)), "bar"));
    Cell *q = cell_new_string("qux");
    printf("$qux ==> %s\n", cell_asprint(env_lookup(e, q)));
    check(!env_lookup(e, q));
}

void test1(void) {
    Cell *n = cell_new_string("foo");
    Cell *v = cell_new_string("bar");
    Cell *e = env_frame(cell_nil, cell_nil, cell_nil);
    char *s;

    env_bind(e, n, v);
    printf("$foo ==> %s\n", cell_asprint(env_lookup(e, n)));
    check(streq(cell_car_string(env_lookup(e, n)), "bar"));
    n = cell_new_string("baz");
    v = cell_new_string("qux");
    env_bind(e, n, v);
    printf("$baz ==> %O\n", cell_car(env_lookup(e, n)));
    check(streq(cell_car_string(env_lookup(e, n)), "qux"));
    n = cell_new_string("animals");
    v = cell_cons_string("pangolin", cell_nil);
    v = cell_cons_string("aardvark", v);
    env_bind(e, n, v);
    s = cell_asprint(cell_car(env_lookup(e, n)));
    printf("$animals ==> %s\n", s);
    check(streq(s, "(aardvark pangolin)"));
    n = cell_new_string("baz");
    s = cell_asprint(cell_car(env_lookup(e, n)));
    printf("$baz ==> %s\n", s);
    check(streq(s, "qux"));
}

void test2(void) {
    char *s;
    Cell *n = cell_nil;
    Cell *v = cell_nil;
    Cell *e = env_frame(cell_nil, cell_nil, cell_nil);
    n = cell_new_string("animals");
    v = cell_cons_string("pangolin", cell_nil);
    v = cell_cons_string("aardvark", v);
    env_bind(e, n, v);
    s = cell_asprint(cell_car(env_lookup(e, n)));
    printf("$animals ==> %s\n", s);
    check(streq(s, "(aardvark pangolin)"));
    e = env_frame(e, cell_nil, cell_nil);
    cell_car_set_string(n, "outer");
    env_bind(e, n, v);
    cell_car_set_string(n, "foo");
    v = cell_new_string("bar");
    env_bind(e, n, v);

    cell_car_set_string(n, "outer");
    s = cell_asprint(cell_car(env_lookup(e, n)));
    printf("$outer ==> %s\n", s);
    check(streq(s, "(aardvark pangolin)"));

    cell_car_set_string(n, "foo");
    s = cell_asprint(cell_car(env_lookup(e, n)));
    printf("$foo ==> %s\n", s);
    check(streq(s, "bar"));

    cell_car_set_string(n, "animals");
    s = cell_asprint(cell_car(env_lookup(e, n)));
    printf("$animals ==> %s\n", s);
    check(streq(s, "(aardvark pangolin)"));
}

void test3(void) {
    Cell *e, *n, *v;
    char *s;

    n = cell_cons_string("five", cell_nil);
    v = cell_cons_string("5", cell_nil);

    n = cell_cons_string("four", n);
    v = cell_cons_string("4", v);
    e = env_frame(cell_nil, n, v);

    n = cell_new_string("five");
    s = cell_asprint(env_lookup(e, n));
    printf("$five ==> %s\n", s);
    check(streq(s, "(5)"));

    n = cell_cons_string("three", cell_nil);
    n = cell_cons_string("two", n);
    v = cell_cons_string("3", cell_nil);
    v = cell_cons_string("2", v);
    e = env_frame(e, n, v);

    n = cell_new_string("five");
    s = cell_asprint(env_lookup(e, n));
    printf("$five ==> %s\n", s);
    check(streq(s, "(5)"));

    n = cell_new_string("two");
    s = cell_asprint(env_lookup(e, n));
    printf("$two ==> %s\n", s);
    check(streq(s, "(2)"));
}

int main(void) {
    printf("1..13\n");
    test0();
    test1();
    test2();
    test3();
    return 0;
}
