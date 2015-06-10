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
