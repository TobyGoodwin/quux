#include <assert.h>
#include <stdlib.h>

#include "assemble.h"
#include "cell.h"
#include "env.h"
#include "external.h"
#include "internal.h"
#include "streq.h"
#include "vm.h"

static int tracing = 0;
#define VmTrace if (1)

static Cell *trace_off(Cell *c) { tracing = 0; return cell_nil; }
static Cell *trace_on(Cell *c) { tracing = 1; return cell_nil; }

static Cell *reg[8];

static Cell *applyp(Cell *c) { return list_headedP(c, "apply"); }
static Cell *beginp(Cell *c) { return list_headedP(c, "begin"); }
static Cell *definep(Cell *c) { return list_headedP(c, "define"); }
static Cell *closurep(Cell *c) { return list_headedP(c, "closure"); }
static Cell *concatp(Cell *c) { return list_headedP(c, "concat"); }
static Cell *corep(Cell *c) { return list_headedP(c, "core"); }
static Cell *externalp(Cell *c) { return list_headedP(c, "external"); }
static Cell *ifp(Cell *c) { return list_headedP(c, "if"); }
static Cell *internalp(Cell *c) { return list_headedP(c, "internal"); }
static Cell *lambdap(Cell *c) { return list_headedP(c, "lambda"); }
static Cell *lookupp(Cell *c) { return list_headedP(c, "lookup"); }
static Cell *quotep(Cell *c) { return list_headedP(c, "quote"); }
static Cell *wordp(Cell *c) { return list_headedP(c, "word"); }

static Cell *vm_gq(Cell *c) {
    //return cell_new_string(glob_quote(cell_car_string(c)));
return 0;
}

static Cell *lookup(Cell *c) {
    return env_lookup(reg[vm_reg_env], c);
}

static Cell *bind(Cell *c) {
    Cell *n = cell_car(c);
    Cell *v = cell_cadr(c);
    Cell *e = cell_caddr(c);
    VmTrace fprintf(stderr, "bind(): n is %s, v is %s, e is %s)\n",
            cell_asprint(n), cell_asprint(v), cell_asprint(e));
    env_bind(e, n, v);
    return cell_nil;
}

static Cell *core_foop(Cell *c, char *x) {
    assert(streq(cell_car_string(c), "core"));
    return streq(cell_car_string(cell_cdr(c)), x) ? cell_true : cell_nil;
}
static Cell *core_consp(Cell *c) { return core_foop(c, "cons"); }
static Cell *core_evalp(Cell *c) { return core_foop(c, "eval"); }
static Cell *core_lookupp(Cell *c) { return core_foop(c, "lookup"); }

static Cell *frame(Cell *c) {
    Cell *n = cell_car(c);
    Cell *v = cell_cadr(c);
    Cell *e = cell_caddr(c);
    VmTrace fprintf(stderr, "frame(): n is %s, v is %s, e is %s\n",
            cell_asprint(n), cell_asprint(v), cell_asprint(e));
    return env_frame(e, n, v);
}

static Cell *internal_call(Cell *c) {
    VmTrace fprintf(stderr, "internal_call(): call %s %s\n",
                cell_asprint(c), cell_asprint(reg[vm_reg_argl]));
    return internal(cell_car_string(c), reg[vm_reg_argl]);
}

static Cell *eq_stringP(Cell *c, char *s) {
    if (c && cell_atomp(c) && cell_stringp(c) && streq(cell_car_string(c), s))
	return cell_true;
    return cell_nil;
}

static Cell *eq_path_searchp(Cell *c) {
    return eq_stringP(c, "path-search");
}

static Cell *make_path_search(Cell *c) {
    /* date ==> (%path-search (quote date)) */
    //Ref(Cell *, r, cell_nil);
    c = cell_cons(c, cell_nil);
    c = cell_cons_string("quote", c);
    c = cell_cons(c, cell_nil);
    c = cell_cons_string("path-search", c);
    //c = cell_cons_string("lookup", c);
    //r = cell_cons(c, r);
    return c;
}

static Cell *const_closure(Cell *c) {
    return cell_new_string("closure");
}

static Cell *make_prim_ext(Cell *c) {
    c = cell_cons(c, cell_nil);
    c = cell_cons_string("external", c);
    return c;
}

static Cell *lastp(Cell *c) {
    if (c && cell_pairp(c) && !cell_cdr(c))
	return cell_true;
    return cell_nil;
}

static Cell *atomp(Cell *x) {
    return cell_atomp(x) ? cell_true : cell_nil;
}

static Cell *fxnump(Cell *x) {
    return cell_fxnump(x) ? cell_true : cell_nil;
}

static Cell *nullp(Cell *x) {
    return cell_nullp(x) ? cell_true : cell_nil;
}

static Cell *pairp(Cell *x) {
    return cell_pairp(x) ? cell_true : cell_nil;
}

struct call {
    char *name;
    Cell *(*fn)(Cell *);
};

/* keep these in order for bsearch! */
struct call calls[] = {
    { "application?", &pairp },
    { "apply?", &applyp },
    { "atom?", &atomp },
    { "begin?", &beginp },
    { "bind", &bind },
    { "caadr", &cell_caadr },
    { "caar", &cell_caar },
    { "cadadr", &cell_cadadr },
    { "cadddr", &cell_cadddr },
    { "caddr", &cell_caddr },
    { "cadr", &cell_cadr },
    { "car", &cell_car },
    { "cdaadr", &cell_cdaadr },
    { "cdadr", &cell_cdadr },
    { "cddr", &cell_cddr },
    { "cdr", &cell_cdr },
    { "closure?", &closurep },
    { "const-closure", &const_closure },
    { "core-cons?", &core_consp },
    { "core-eval?", &core_evalp },
    { "core-lookup?", &core_lookupp },
    { "core?", &corep },
    { "define?", &definep },
    { "eq-path-search?", &eq_path_searchp },
    { "external", &external },
    { "external?", &externalp },
    { "frame", &frame },
    { "if?", &ifp },
    { "internal", &internal_call },
    { "internal?", &internalp },
    { "lambdap", &lambdap },
    { "last?", &lastp },
    { "lookup", &lookup },
    { "lookup?", &lookupp },
    { "make-path-search", &make_path_search },
    { "make-prim-ext", &make_prim_ext },
    { "null?", &nullp },
    { "number?", &fxnump },
    { "pair?", &pairp },
    { "quote?", &quotep },
    { "reverse", &list_reverse },
    { "trace-off", &trace_off },
    { "trace-on", &trace_on },
    { "wordp", &wordp }
};

static int call_compare(const void *a0, const void *b0) {
    const struct call *a = a0, *b = b0;
    return strcmp(a->name, b->name);
}

byte vm_encode_call(char *n) {
    int i;
    struct call *c, k;

    k.name = n;
    c = bsearch(&k, calls, sizeof calls / sizeof(struct call),
	    sizeof(struct call), call_compare);
    if (!c) {
        fprintf(stderr, "quux: fatal: cannot find call `%s'\n", n);
        exit(1);
    }
    i = c - calls;
    assert(i < 256);
    return i;
}

static char *reg_names[] = {
    "exp", "val", "env", "cont", "argl", "unev", "fun", "exp2"
};

void vm_run(byte *code) {
    Cell *stack = cell_nil;
    int ip = 0;
    int operand, source;

    /* The exp and env registers have been set externally. The other
     * registers we need to zero (as they may contain dangling pointers)
     */
    reg[vm_reg_val] = reg[vm_reg_exp2] = reg[vm_reg_cont] = cell_nil;

    /* the first two bytes are the address of label "end": load them into the
     * cont register */
    operand = code[ip++] << 8;
    operand += code[ip++];
    reg[vm_reg_cont] = cell_new_fxnum(operand);

    while (code[ip] != vm_end) {
	VmTrace if (ip == 2) {
	    fprintf(stderr, "vm_run(): EVAL %s\n",
                    cell_asprint(reg[vm_reg_exp]));
	    fprintf(stderr, "vm_run(): EVAL stack: %s\n", cell_asprint(stack));
        }
	operand = code[ip] & vm_rmask;
	switch (code[ip] & vm_imask2) {
	default: assert(0);
	case vm_move:
	    source = (code[ip] & vm_smask) >> 3;
	    VmTrace fprintf(stderr, "vm_run(): move %s from %s to %s\n",
			cell_asprint(reg[source]),
                        reg_names[source], reg_names[operand]);
	    reg[operand] = reg[source];
	    break;
	case vm_i5:
	    switch (code[ip] & vm_imask5) {
	    default: assert(0);
	    case vm_nil:
		VmTrace fprintf(stderr, "vm_run(): nil %s\n", reg_names[operand]);
		reg[operand] = cell_nil;
		break;
	    case vm_push:
		VmTrace fprintf(stderr, "vm_run(): %d: push %s from %s\n",
                        ip, cell_asprint(reg[operand]), reg_names[operand]);
		stack = cell_cons(reg[operand], stack);
		break;
	    case vm_pop:
		reg[operand] = cell_car(stack);
		VmTrace fprintf(stderr, "vm_run(): %d: pop %s to %s\n",
                            ip, cell_asprint(reg[operand]), reg_names[operand]);
		stack = cell_cdr(stack);
		break;
	    case vm_i8:
		switch (code[ip]) {
		default: assert(0);
		case vm_call:
		    VmTrace fprintf(stderr, "vm_run(): call (%s %s) ==> ",
				calls[code[ip + 1]].name,
                                cell_asprint(reg[vm_reg_exp]));
		    reg[vm_reg_val] = calls[code[++ip]].fn(reg[vm_reg_exp]);
		    VmTrace fprintf(stderr, "%s\n",
                            cell_asprint(reg[vm_reg_val]));
		    break;
		case vm_cons:
		    VmTrace fprintf(stderr, "vm_run(): (cons %s %s) ==> ",
                            cell_asprint(reg[vm_reg_exp]),
                            cell_asprint(reg[vm_reg_exp2]));
		    reg[vm_reg_val] = cell_cons(reg[vm_reg_exp],
						    reg[vm_reg_exp2]);
		    VmTrace fprintf(stderr, "%s\n",
                            cell_asprint(reg[vm_reg_val]));
		    break;
		case vm_branch:
		    if (!reg[vm_reg_val]) {
			VmTrace fprintf(stderr, "vm_run(): branch not taken\n");
			ip += 2;
			break;
		    }
		    VmTrace fprintf(stderr, "vm_run(): branch taken\n");
		    /* fall through */
		case vm_jump:
		    operand = code[++ip] << 8;
		    operand += code[++ip];
		    VmTrace fprintf(stderr, "vm_run(): jump to %d\n", operand);
		    /* easier to subtract 1 than to skip the "++ip" below */
		    ip = operand - 1;
		    break;
		case vm_loadcont:
		    operand = code[++ip] << 8;
		    operand += code[++ip];
		    VmTrace fprintf(stderr, "vm_run(): load cont with %d\n", operand);
		    reg[vm_reg_cont] = cell_new_fxnum(operand);
		    break;
		case vm_continue:
		    ip = cell_car_fxnum(reg[vm_reg_cont]) - 1;
		    VmTrace fprintf(stderr, "vm_run(): continue at %d\n", ip + 1);
		    break;
		}
	    }
	}
	++ip;
    }
}

void vm_reg_set(int r, void *val) {
    reg[r] = val;
}

Cell *vm_reg_get(int r) {
    return reg[r];
}
