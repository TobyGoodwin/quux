2015-06-14
==========

The previous implementation of `list_flatten` worked by bouncing through
the old `List` represenation:

    Cell *list_flatten(Cell *c0) {
             /* this is not an efficient implementation! */
            return cell_from_list(cell_to_list(c0));
    }

Obviously we no longer have `List`, so the inevitable rewrite became
urgent. I'm pleased to report that it is 5 lines of code, which I got
right first time!

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

Right. That's the evaluator working again, up to the point of being able
to run code like:

    %echo hello world; %echo goodbye again

Now, at this point I need to do some hard thinking about various kinds
of calling out that the vm can do. For one thing, there are the "core"
calls that the evalutator code itself makes, such as `(call internal?)`
and `(call lookup)`. I think I know where I am with these.

Next, there are "internal" calls (these were called "primitives" in es).
They are implemented internally, in C, but are not part of the evaluator
itself. The canonical example would be `cd`, also `echo` etc. Parsed
code can unambiguously refer to these as `(internal cd)`.

Finally there are "external" calls, such as `ls`. These, I believe, are
handled in a two stage process. Initially, any unbound name `foo` is
converted to `(search foo path)`. Now `search` is, hopefully, bound to
something that find the first executable `foo` in `path` (note that we
must pass in the current value of `path`, otherwise we'll always be
searching whatever `path` was in effect at the moment that `search` was
defined). So `search` might return something like `(external /bin/ls)`,
where `external` is a special form that actually does a fork and exec.

I'd earlier got worried about what happens if names that are used in
parsed code, such as `lookup`, are rebound. I invented "immutable"
hooks, so that these always refer to their correct special forms. But I
don't think we need this: thanks to lexical scoping it should all work
out.

Finally, the current evaluator does not have macros. I've been toying
with the idea of adding them, and I think I should. In terms of the
evaluator, I think it's easy enough. If the operator evaluates to a
macro, then we simply invoke it with the unevaluated arguments from
`unev`. Errm, what happens then I'm a bit unsure, but we could start
with that and see where it goes.

Hmm. No, on looking at R7RS, I definitely don't understand macros. (I
mean, I think the above is right as far as it goes, but there's a lot
more to it than that.) I'd have to play with them a fair bit before I
thought of implementing them. So let's shelve that idea for a bit.

Rewind a bit, to the immutables. Suppose some code comes in that says

    (eval foo)

The evaluator knows what this means: it moves `foo` to `exp` and jumps to
`eval-dispatch`. This is fundamental syntax: it isn't something that an
internal call can ever do. So if we ever want to be able to rebind `eval`,
there needs to be something that `eval` is bound to in the first place, whether
that's `%eval` or `(core eval)` or something else. And, in fact, there's no
particular reason to bind plain `eval` to that: the parser might as well
generate whatever the eternal name is.

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
