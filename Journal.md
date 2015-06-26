2015-06-24
==========

Next things to look at: make sure that `if` works, and test that tail
recursion is implemented, including in `if` branches.

Hmm. So this code:

    yes = yes
    no = ()
    if $no { echo yes } { echo no }
    echo in between
    if $yes { echo yes } { echo no }

prints this:

    (no)
    (in between)
    ((-closure (echo (quote yes))-))

So it looks like we're missing a level of eval for the true branch. But
then I'm not sure we're creating the right scheme code in the first
place.

OK. So there was a bug in the VM implementation of `if`. Given the list
`(then else)`, we were using `car` to get `then` (right), but `cdr` to
get `else` (wrong: should be `cadr`).

Now, this code behaves correctly:

%(begin ((internal echo) (if (quote yes) (quote foo) (quote bar))) ((internal echo) (if () (quote baz) (quote quux))))


2015-06-22
==========

Finally got back a state where `prelude.quux` works... well, sort of.
Unfortunately bindings don't work. I *think* what's happening is that
somewhere along the line something is getting mutated when it should be
copied, and we're losing the initial environment frame. But I'm not
sure.

No, it wasn't even as complicated as that. I'd used a `caddr` where I
should have used `cddr`. And handling `(lambda x ...)` case messed up
the no formals case, because `atom?` (and `pair?`) both group `()` with
proper atoms. Obviously the simple fix is to use `pair?` or `null?`.
(The `list?` predicate would do the job properly, although it probably
does more work than is strictly needed here. But in any case I haven't
implemented it yet.)

2015-06-19
==========

Back to trying to get `prelude.quux` working. Back to fighting
`unquote`. It has to go. Restate my assumptions...

It would be possible to implement a shell in Scheme, and if that were
anything at all like quux, the parser might generate code like `(command
'ls '/tmp)`. But that's not what I'm trying to do at all.

Scheme *is* the shell. And that's why I want the parser to build code
like `(ls '/tmp)`, with a special dispensation for the case where `ls`
is not bound. (I was looking for a better name for `path-search`, and it
is `unbound`.) So the parser needs to generate the right code in the
first place, not attempt to patch it up afterwards.

OK. So that's a *much* neater grammar, that correctly handles the cases
of `echo` and `(internal echo)`. Doesn't do `$e` yet... oh, ah, hmmm.
I'm not sure that `echo = internal echo; echo foo` should print
anything. Surely we need to say `$echo foo`? And, if not, what would
`$echo foo` do there? Shouldn't we need something like `echo x = {
(internal read) x}; echo foo` for it to work?

No, that's all right, since `echo` is bound to (`(internal echo)` which
is bound to) a closure. If we said `echo = /bin/echo`, then you'd need
the `$` to turn the string into a closure. Errm, except you wouldn't,
actually, would you? As the normal unbound procedure would make it work
anyway...

So this is something I hadn't anticipated, but I think it works out all
right. In a "normal" shell, the command word is effectively *always*
handed to `path-search`, except there's a hacky extra symbol table (of
functions, aliases, but *not* "variables") that comes before
`path-search`. In those shells, `$echo` adds a lookup in the variable
symbol table before the usual procedure. In quux, we always start with
the (single) symbol table, and only if that yields nothing do we invoke
the `path-search` procedure.

Hmm. Also, should we do something special if a command starts with `/`?
Well, don't 100% need to, as `(external /bin/echo)` is an unambiguous
way to run the command.

2015-06-17
==========

Ugh. The vm is an annoying thing to program. (Having an instruction to
access stack locations other than the top might be useful.) However, I
got there in the end. I can now define `list` in Scheme in the initial
environment, and it works.

Currently `quote`, `sequence` (which should really be called `begin`)
are implemented in the vm, and have those fixed names. But `cons`,
`eval`, and `lookup` are implemented with this `(core foo)` syntax I
invented. (Should `lookup` even exist? How does it differ from `eval`?)

Turns out it's easy enough to build an initial environment which binds
`cons` to `(core cons)`.

Oh, now, the difference between `quote` and `cons` is that `quote` is a
special form, whereas `cons` is just a function. One day, if we have
syntax transformers, that could be tidied up. For now, though, 

2015-06-16
==========

So the current subproject is to make quux understand `(lambda x x)`,
specifically the case where the formal parameter list is a single value. 

Currently we call `(internal frame)` with `exp` holding

  ((names) (vals) (env))

So I think all we need to do is look and see if names is an atom: if it
is, cons it to `cell_nil` and do the same with vals. This could easily
be done in the `frame` wrapper, or even `env_frame()`, but first I'll
try it in the vm itself.

2015-06-15
==========

Thinking about these "core" functions some more: `list` is trivial to
implement in Scheme. Well, provided we support the version of lambda
where formals is a single name, which is bound to the entire argument
list. (Currently we don't.) In fact, list is the identity function:

  (define list (lambda x x ))

However, I don't see any particularly good reason why the parser needs
to generate calls to `list`. I'm reasonably sure that `eval` can be
written in Scheme, although not quite sure how at the moment. At least
`car` and `cdr` and `cons` will need to be core functions. I don't think
having `%car` etc is a *terrible* way to do this, although I'm not sure
it's the best. I do quite like `(core car)`, although it does steal the
name `core`.

Here[1] is a list of "syntactic keywords" that might be useful.

[1] http://groups.csail.mit.edu/mac/ftpdir/scheme-7.4/doc-html/scheme_2.html#SEC27

OK. Let's go with `(core cons)` etc. and see where we get to. Yup, that
seems to work fine. Now done for `cons`, `echo`, `eval` and `lookup`.
Errm, we really don't need `(core echo)` when we have `(internal echo)`.

Now, the reason we need `list` in the parser output is this:

  echo = internal echo

needs to become

    (define echo (list (quote internal) (quote echo)))

And there's no way to avoid the `list`. So... do we want `(core list)`,
or can I actually implement it in Scheme? Well, first of all, what does
a function definition and application look like? If we say

    (define e (lambda (x) (echo x)))

then `e` is bound to a list comprising the constant `closure`, the list
of formals, the procedure body, and the environment. However, I really
can't work out at this time of night how `apply-closure` works. Oh, hang
on. We have a list of formal parameters, and a list of actual
parameters, so we just hook them up into a new environment frame. So to
handle the (lambda x ...) case, we just need to see if formals is an
atom, and if so wrap an extra layer around both formals and actuals.

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
