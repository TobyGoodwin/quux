#include "tree.h"

static Cell *flat(Cell *a, Cell *t) {
    if (!t)
        return a;
    if (cell_atomp(t))
        return cell_cons(t, a);
    a = flat(a, cell_cdr(t));
    return list_append(flat(cell_nil, cell_car(t)), a);
}

Cell *tree_flatten(Cell *t) {
    return flat(cell_nil, t);
}
