/* This ain't A-lists. */

#include <assert.h>

#include "cell.h"
#include "env.h"
#include "streq.h"

#define EnvTrace if (1)

/* An environment is a list of frames. A frame is a pair: the car of the
 * frame is a list of names; the cdr of the frame is a list of values.
 */

/* modify the environment to make a new binding in the first frame */
void env_bind(Cell *env, Cell *name, Cell *val) {
    Cell *frame, *names, *vals;

    EnvTrace fprintf(stderr, "env_bind(%s, %s, %s)\n",
            cell_asprint(env), cell_asprint(val), cell_asprint(name));
    if (cell_atomp(name)) {
	name = cell_cons(name, cell_nil);
	EnvTrace fprintf(stderr, "env_bind(): name enlisted to %s\n",
                cell_asprint(name));
    }
    val = cell_cons(val, cell_nil);

    frame = cell_car_cell(env);
    EnvTrace fprintf(stderr, "env_bind(): frame is %s\n", cell_asprint(frame));
    names = frame ? cell_car_cell(frame) : cell_nil;
    vals = frame ? cell_cdr(frame) : cell_nil;
    EnvTrace fprintf(stderr, "env_bind(): old-names is %s\n",
            cell_asprint(names));
    list_appendM(name, names);
    EnvTrace fprintf(stderr, "env_bind(): out-names is %s\n",
            cell_asprint(name));
    list_appendM(val, vals);
    EnvTrace fprintf(stderr, "env_bind(): out-vals is %s\n",
            cell_asprint(val));
    cell_car_set(frame, name);
    cell_cdr_set(frame, val);
    //EnvTrace fprint(stderr, "env_bind(): frame is %O\n", frame);
    EnvTrace fprintf(stderr, "env_bind(): out-env is %s\n", cell_asprint(env));
}

/* attach a new frame to an environment */
Cell *env_frame(Cell *env, Cell *names, Cell *values) {
    Cell *frame = cell_cons(names, values);
    assert(cell_nullp(names) || cell_pairp(names));
    assert(cell_nullp(values) || cell_pairp(values));
    env = cell_cons(frame, env);
    return env;
}

/* Lookup a name in an environment. The value is returned as a singleton
 * list: this allows the caller to distinguish between an unbound name:
 * lookup returns (); and a name bound to the empty list: lookup returns
 * (()).
 */
Cell *env_lookup(Cell *env, Cell *name) {
    char *tgt = cell_car_string(name);
    Cell *e;

    EnvTrace fprintf(stderr, "env_lookup(%s)\n", tgt);
    for (e = env; e; e = cell_cdr(e)) {
	Cell *frame = cell_car_cell(e);
	Cell *names = cell_car_cell(frame);
	EnvTrace fprintf(stderr, "env_lookup(): names is %s\n",
                cell_asprint(names));
	Cell *vals = cell_cdr(frame);
	EnvTrace fprintf(stderr, "env_lookup(): vals is %s\n",
                cell_asprint(vals));
	Cell *n, *v;
	for (n = names, v = vals; n; n = cell_cdr(n), v = cell_cdr(v))
	    if (streq(cell_car_string(n), tgt)) {
                EnvTrace fprintf(stderr, "env_lookup(): returning %s\n",
                        cell_asprint(cell_car(v)));
		return cell_cons(cell_car(v), cell_nil);
            }
    }
    EnvTrace fprintf(stderr, "env_lookup(): no binding found for %s\n", tgt);
    return cell_nil;
}
