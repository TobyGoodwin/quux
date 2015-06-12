/* This ain't A-lists. */

#include "es.h"

#include "env.h"

#define EnvTrace if (0)

/* An environment is a list of frames. A frame is a pair: the car of the
 * frame is a list of names; the cdr of the frame is a list of values.
 */

/* modify the environment to make a new binding in the first frame */
void env_bind(Cell *env0, Cell *name0, Cell *val0) {
    Ref(Cell *, env, env0);
    Ref(Cell *, val, val0);
    Ref(Cell *, name, name0);
    EnvTrace fprint(2, "env_bind(): in-env is %O\n", env);
    EnvTrace fprint(2, "env_bind(): in-val is %O\n", val);
    EnvTrace fprint(2, "env_bind(): in-name is %O\n", name);
    if (cell_atomp(name)) {
	name = cell_cons(name, cell_nil);
	EnvTrace fprint(2, "env_bind(): in-name enlisted to %O\n", name);
    }
    val = cell_cons(val, cell_nil);
    Ref(Cell *, frame, cell_car_cell(env));

    EnvTrace fprint(2, "env_bind(): frame is %O\n", frame);
    Ref(Cell *, names, frame ? cell_car_cell(frame) : cell_nil);
    Ref(Cell *, vals, frame ? cell_cdr(frame) : cell_nil);
    EnvTrace fprint(2, "env_bind(): old-names is %O\n", names);
    list_appendM(name, names);
    EnvTrace fprint(2, "env_bind(): out-names is %O\n", name);
    list_appendM(val, vals);
    EnvTrace fprint(2, "env_bind(): out-vals is %O\n", val);
    cell_car_set(frame, name);
    cell_cdr_set(frame, val);
    //EnvTrace fprint(2, "env_bind(): frame is %O\n", frame);
    RefEnd2(vals, names);
    RefEnd3(frame, name, val);
    EnvTrace fprint(2, "env_bind(): out-env is %O\n", env);
    RefEnd(env);
}

/* attach a new frame to an environment */
Cell *env_frame(Cell *env0, Cell *names0, Cell *values0) {
    Ref(Cell *, env, env0);
    Ref(Cell *, values, values0);
    Ref(Cell *, names, names0);
    //EnvTrace fprint(2, "env_frame(): starting with %O\n", env);
    Ref(Cell *, frame, cell_cons(names, values));

    env = cell_cons(frame, env);
    RefEnd3(frame, names, values);
    //EnvTrace fprint(2, "env_frame(): returning %O\n", env);
    RefReturn(env);
}

/* Lookup a name in an environment. The value is returned as a singleton
 * list: this allows the caller to distinguish between an unbound name:
 * lookup returns (); and a name bound to the empty list: lookup returns
 * (()).
 */
Cell *env_lookup(Cell *env, Cell *name) {
    char *tgt = cell_car_string(name);
    Cell *e;

    EnvTrace fprint(2, "env_lookup(): looking for %s\n", tgt);
    for (e = env; e; e = cell_cdr(e)) {
	Cell *frame = cell_car_cell(e);
	Cell *names = cell_car_cell(frame);
	EnvTrace fprint(2, "env_lookup(): names is %O\n", names);
	Cell *vals = cell_cdr(frame);
	EnvTrace fprint(2, "env_lookup(): vals is %O\n", vals);
	Cell *n, *v;
	for (n = names, v = vals; n; n = cell_cdr(n), v = cell_cdr(v))
	    if (streq(cell_car_string(n), tgt))
		return cell_cons(cell_car(v), cell_nil);
    }
    EnvTrace fprint(2, "env_lookup(): no binding found for %s\n", tgt);
    return cell_nil;
}
