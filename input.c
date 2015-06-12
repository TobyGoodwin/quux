#include <string.h>

#include "input.h"

Cell *parse(void) {
    Cell *result;
    char *inp = "foo = bar";
    int r;

    struct pacc_parser *pp = pacc_new();
    pacc_input(pp, inp, strlen(inp));
    r = pacc_parse(pp);
    fprintf(stderr, "parse() returns %d\n", r);
    if (r) {
        result = pacc_result(pp);
        fprintf(stderr, "==> %s\n", cell_asprint(result));
        return result;
    }
    return 0;
}
