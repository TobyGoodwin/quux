#include "check.h"
#include "env.h"
#include "streq.h"

#if 0
void test0(void) {
    Ref(Cell *, n, cell_new_string("foo"));
    Ref(Cell *, v, cell_new_string("bar"));
    Ref(Cell *, e, env_frame(cell_nil, cell_nil, cell_nil));
    env_bind(e, n, v);
    print("$foo ==> %O\n", env_lookup(e, n));
    check(streq(cell_car_string(env_lookup(e, n)), "bar"));
    Ref(Cell *, q, cell_new_string("qux"));
    print("$qux ==> %O\n", env_lookup(e, q));
    check(!env_lookup(e, q));
    RefEnd(q);
    RefEnd3(e, v, n);
}

void test1(void) {
    char *s;
    Ref(Cell *, n, cell_new_string("foo"));
    Ref(Cell *, v, cell_new_string("bar"));
    Ref(Cell *, e, env_frame(cell_nil, cell_nil, cell_nil));
    env_bind(e, n, v);
    print("$foo ==> %O\n", env_lookup(e, n));
    check(streq(cell_car_string(env_lookup(e, n)), "bar"));
    n = cell_new_string("baz");
    v = cell_new_string("qux");
    env_bind(e, n, v);
    print("$baz ==> %O\n", cell_car(env_lookup(e, n)));
    check(streq(cell_car_string(env_lookup(e, n)), "qux"));
    n = cell_new_string("animals");
    v = cell_cons_string("pangolin", cell_nil);
    v = cell_cons_string("aardvark", v);
    env_bind(e, n, v);
    s = str("%O", cell_car(env_lookup(e, n)));
    print("$animals ==> %s\n", s);
    check(streq(s, "(aardvark pangolin)"));
    n = cell_new_string("baz");
    s = str("%O", cell_car(env_lookup(e, n)));
    print("$baz ==> %s\n", s);
    check(streq(s, "qux"));
    RefEnd3(e, v, n);
}
#endif

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

#if 0
void test3(void) {
    char *s;
    Ref(Cell *, n, cell_cons_string("five", cell_nil));
    n = cell_cons_string("four", n);
    Ref(Cell *, v, cell_cons_string("5", cell_nil));
    v = cell_cons_string("4", v);
    Ref(Cell *, e, env_frame(cell_nil, cell_nil, cell_nil));
    env_bind(e, n, v);

    n = cell_new_string("five");
    s = str("%O", cell_car(env_lookup(e, n)));
    print("$five ==> %s\n", s);
    check(streq(s, "5"));

    n = cell_cons_string("three", cell_nil);
    n = cell_cons_string("two", n);
    v = cell_cons_string("3", cell_nil);
    v = cell_cons_string("2", v);
    e = env_frame(e, n, v);

    n = cell_new_string("five");
    s = str("%O", cell_car(env_lookup(e, n)));
    print("$five ==> %s\n", s);
    check(streq(s, "5"));

    n = cell_new_string("two");
    s = str("%O", cell_car(env_lookup(e, n)));
    print("$two ==> %s\n", s);
    check(streq(s, "2"));

    RefEnd3(e, v, n);
}
#endif

int main(void) {
    //test0();
    //test1();
    test2();
    //test3();
    return 0;
}
