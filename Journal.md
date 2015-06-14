2015-06-14
==========

The previous implementation of `list_flatten` worked by bouncing through the
old `List` represenation:

    Cell *list_flatten(Cell *c0) {
             /* this is not an efficient implementation! */
            return cell_from_list(cell_to_list(c0));
    }

Obviously we no longer have `List`, so the inevitable rewrite became urgent.
I'm pleased to report that it is 5 lines of code, which I got right first time!

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

2015-06-09
==========

So I've got as far as the very first cell test, checking that the `nil`
cell is `null?`, and already I've hit a snag! Should cell predicates
return a C boolean (i.e. an `int` which is either 0 or 1) or a cell
boolean (a `Cell` which is either nil or `cell_true`)?

The `cell_true` constant contains `#t`, just in case anyone ever prints
it out, but logically the only thing we care about is that it is not
nil. Or should we go more modern, and have a `cell_false` containing
`#f`? But to make that fly we'd need symbols, which we don't (yet?)
have.

It should be `int`. If somebody (vm I'm looking at you) needs cell
booleans, it can trivially make them.
