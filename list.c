#include "list.h"

int list_length(Cell *c) {
    int r = 0;
    for ( ; c; c = cell_cdr(c))
	++r;
    return r;
}

/* list_reverse() builds a new list, and - while it is O(n) in time - is
 * therefore somewhat inefficient. We could make a much quicker
 * destructive reverse using cell_cleave()...
 */
Cell *list_reverse(Cell *c) {
    Cell *r = cell_nil;

    for ( ; c; c = cell_cdr(c))
	r = cell_cons(cell_car(c), r);
    
    return r;
}
