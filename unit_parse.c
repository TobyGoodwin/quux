#include <stdio.h>

#include "cell.h"
#include "check.h"
#include "input.h"
#include "streq.h"

void parse_check(char *q, char *s) {
    Cell *c = parse_string(q);
    char *r = cell_asprint(c);
    check(streq(r, s));
}

void test_raw(void) {
    parse_check("%()", "()");
    parse_check("%foo", "foo");
    parse_check("%(foo)", "(foo)");
    parse_check("%(foo bar)", "(foo bar)");
}

int main(void) {
    printf("1..4\n");
    test_raw();
    return 0;
}
