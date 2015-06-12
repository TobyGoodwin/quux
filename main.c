#include <stdio.h>

#include "input.h"

int main(void) {
    parse_string("foo = bar; qux = baz");
    parse_file("quux.vmc");
    return 0;
}
