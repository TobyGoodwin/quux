#include "es.h"

#include "assemble.h"
#include "cell.h"
#include "env.h"
#include "vm.h"

#define VmTrace if (1)

static Cell *reg[8];

static Cell *applyp(Cell *c) { return list_headedP(c, "apply"); }
static Cell *definep(Cell *c) { return list_headedP(c, "define"); }
static Cell *closurep(Cell *c) { return list_headedP(c, "closure"); }
static Cell *concatp(Cell *c) { return list_headedP(c, "concat"); }
static Cell *externalp(Cell *c) { return list_headedP(c, "external"); }
static Cell *ifp(Cell *c) { return list_headedP(c, "if"); }
static Cell *internalp(Cell *c) { return list_headedP(c, "internal"); }
static Cell *lambdap(Cell *c) { return list_headedP(c, "lambda"); }
static Cell *lookupp(Cell *c) { return list_headedP(c, "lookup"); }
static Cell *quotep(Cell *c) { return list_headedP(c, "quote"); }
static Cell *sequencep(Cell *c) { return list_headedP(c, "sequence"); }
static Cell *wordp(Cell *c) { return list_headedP(c, "word"); }

static Cell *vm_gq(Cell *c) {
    return cell_new_string(glob_quote(cell_car_string(c)));
}

static Cell *lookup(Cell *c) {
    return env_lookup(reg[vm_reg_env], c);
}

static Cell *bind(Cell *c0) {
    Ref(Cell *, c, c0);
    Ref(Cell *, n, cell_car(c));
    Ref(Cell *, v, cell_cadr(c));
    Ref(Cell *, e, cell_caddr(c));
    VmTrace fprint(2, "bind(): n is %O, v is %O, e is %O\n", n, v, e);
    env_bind(e, n, v);
    RefEnd3(e, v, n);
    RefEnd(c);
    return cell_nil;
}

static Cell *frame(Cell *c0) {
    Ref(Cell *, c, c0);
    Ref(Cell *, n, cell_car(c));
    Ref(Cell *, v, cell_cadr(c));
    Ref(Cell *, e, cell_caddr(c));
    VmTrace fprint(2, "frame(): n is %O, v is %O, e is %O\n", n, v, e);
    c = env_frame(e, n, v);
    RefEnd3(e, v, n);
    RefReturn(c);
}

static Cell *prim_apply(Cell *c) {
    VmTrace fprint(2, "prim_apply(): call %O%O\n", c, reg[vm_reg_argl]);
    return primplus(cell_car_string(c), reg[vm_reg_argl], 0, 0);
}

static Cell *eq_stringP(Cell *c, char *s) {
    if (c && cell_atomp(c) && cell_stringp(c) && streq(cell_car_string(c), s))
	return cell_true;
    return cell_nil;
}

static Cell *eq_pcconsP(Cell *c) { return eq_stringP(c, "%cons"); }
static Cell *eq_pcevalP(Cell *c) { return eq_stringP(c, "%eval"); }
static Cell *eq_listP(Cell *c) { return eq_stringP(c, "list"); }
static Cell *eq_pclookupP(Cell *c) { return eq_stringP(c, "%lookup"); }

/* XXX: only for testing! */
static Cell *eq_pcechoP(Cell *c) { return eq_stringP(c, "%echo"); }

static Cell *immutableP(Cell *c) {
    if (eq_pcconsP(c) || eq_pcevalP(c) || eq_listP(c) ||
	    eq_pclookupP(c)    || eq_pcechoP(c))
	return cell_true;
    return cell_nil;
}

static Cell *eq_pcpath_searchP(Cell *c) {
    return eq_stringP(c, "%path-search");
}

static Cell *make_path_search(Cell *c0) {
    /* date ==> (%path-search (quote date)) */
    //Ref(Cell *, r, cell_nil);
    Ref(Cell *, c, c0);
    c = cell_cons(c, cell_nil);
    c = cell_cons_string("quote", c);
    c = cell_cons(c, cell_nil);
    c = cell_cons_string("%path-search", c);
    //c = cell_cons_string("lookup", c);
    //r = cell_cons(c, r);
    RefReturn(c);
}

static Cell *const_closure(Cell *c) {
    return cell_new_string("closure");
    Ref(Cell *, r, cell_new_string("closure"));
    RefReturn(r);
}

static Cell *make_prim_ext(Cell *c0) {
    Ref(Cell *, c, c0);
    c = cell_cons(c, cell_nil);
    c = cell_cons_string("external", c);
    RefReturn(c);
}

static Cell *lastp(Cell *c) {
    if (c && cell_pairp(c) && !cell_cdr(c))
	return cell_true;
    return cell_nil;
}

/* This was hacked up quickly just to get external commands going at
 * all, and will need some more thought. It doesn't belong here. */
static Cell *external(Cell *cell0) {
    char **argv, *cmd;
    int argc, i;
    pid_t p, w;
    Ref(Cell *, cell, cell0);

    cell = list_flatten(cell);
    cmd = cell_car_string(cell);
    fprint(2, "external command: %O\n", cell);

    argc = list_length(cell);
    argv = malloc(sizeof (char *) * (argc + 1));
    for (i = 0; i < argc; ++i) {
	argv[i] = cell_car_string(cell);
	cell = cell_cdr(cell);
    }
    argv[i] = 0;
    RefEnd(cell);

    p = fork();
    switch (p) {
    case -1:
	fprint(2, "fork failed!\n");
	break;
    case 0:
	execve(cmd, argv, 0);
	fprint(2, "exec failed!\n");
	_exit(1);
    default:
	do {
	    /* One day we'll have to tackle wait() properly. Eugh. */
	    w = wait(0);
	} while (w != p);
	break;
    }
    /* And, of course, return what wait() said. */
    return cell_nil;
}

struct call {
    char *name;
    Cell *(*fn)(Cell *);
};

/* keep these in order for bsearch! */
struct call calls[] = {
    { "application?", &cell_pairp },
    { "apply?", &applyp },
    { "atom?", &cell_atomp },
    { "bind", &bind },
    { "caadr", &cell_caadr },
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
    { "concat", &concat },
    { "concatp", &concatp },
    { "const-closure", &const_closure },
    { "define?", &definep },
    { "eq-%cons?", &eq_pcconsP },
    { "eq-%echo?", &eq_pcechoP },
    { "eq-%eval?", &eq_pcevalP },
    { "eq-%lookup?", &eq_pclookupP },
    { "eq-%path-search?", &eq_pcpath_searchP },
    { "eq-list?", &eq_listP },
    { "external", &external },
    { "external?", &externalp },
    { "frame", &frame },
    { "glob", &glob },
    { "glob_quote", &vm_gq },
    { "if?", &ifp },
    { "immutable?", &immutableP },
    { "internal?", &internalp },
    { "lambdap", &lambdap },
    { "last?", &lastp },
    { "lookup", &lookup },
    { "lookup?", &lookupp },
    { "make-path-search", &make_path_search },
    { "make-prim-ext", &make_prim_ext },
    { "null?", &cell_nullp },
    { "number?", &cell_fxnump },
    { "pairp", &cell_pairp },
    { "prim-apply", &prim_apply },
    { "quote?", &quotep },
    { "reverse", &list_reverse },
    { "sequence?", &sequencep },
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
    assert(c);
    i = c - calls;
    assert(i < 256);
    return i;
}

static char *reg_names[] = {
    "exp", "val", "env", "cont", "argl", "unev", "fun", "exp2"
};

void vm_run(byte *code) {
    int ip = 0;
    int operand, source;

    Ref(Cell *, stack, cell_nil);
    /* The exp and env registers have been set externally. The other
     * registers we need to zero (as they may contain dangling pointers)
     */
    reg[vm_reg_val] = reg[vm_reg_exp2] = reg[vm_reg_cont] = cell_nil;

    RefAdd(reg[vm_reg_exp]); RefAdd(reg[vm_reg_val]);
    RefAdd(reg[vm_reg_env]); RefAdd(reg[vm_reg_cont]);
    RefAdd(reg[vm_reg_argl]); RefAdd(reg[vm_reg_unev]);
    RefAdd(reg[vm_reg_fun]); RefAdd(reg[vm_reg_exp2]);

    /* the first two bytes are the address of label "end": load them into the
     * cont register */
    operand = code[ip++] << 8;
    operand += code[ip++];
    reg[vm_reg_cont] = cell_new_fxnum(operand);

    while (code[ip] != vm_end) {
	VmTrace if (ip == 2) {
	    fprint(2, "vm_run(): EVAL %O\n", reg[vm_reg_exp]);
	    fprint(2, "vm_run(): EVAL stack: %O\n", stack);
        }
	operand = code[ip] & vm_rmask;
	switch (code[ip] & vm_imask2) {
	default: assert(0);
	case vm_move:
	    source = (code[ip] & vm_smask) >> 3;
	    VmTrace fprint(2, "vm_run(): move %O from %s to %s\n",
			reg[source], reg_names[source], reg_names[operand]);
	    reg[operand] = reg[source];
	    break;
	case vm_i5:
	    switch (code[ip] & vm_imask5) {
	    default: assert(0);
	    case vm_nil:
		VmTrace fprint(2, "vm_run(): nil %s\n", reg_names[operand]);
		reg[operand] = cell_nil;
		break;
	    case vm_push:
		VmTrace fprint(2, "vm_run(): %d: push %O from %s\n",
			    ip, reg[operand], reg_names[operand]);
		stack = cell_cons(reg[operand], stack);
		break;
	    case vm_pop:
		reg[operand] = cell_car(stack);
		VmTrace fprint(2, "vm_run(): %d: pop %O to %s\n",
			    ip, reg[operand], reg_names[operand]);
		stack = cell_cdr(stack);
		break;
	    case vm_i8:
		switch (code[ip]) {
		default: assert(0);
		case vm_call:
		    VmTrace fprint(2, "vm_run(): call (%s %O) ==> ",
				calls[code[ip + 1]].name, reg[vm_reg_exp]); 
		    reg[vm_reg_val] = calls[code[++ip]].fn(reg[vm_reg_exp]);
		    VmTrace fprint(2, "%O\n", reg[vm_reg_val]);
		    break;
		case vm_cons:
		    VmTrace fprint(2, "vm_run(): (cons %O %O) ==> ",
				reg[vm_reg_exp], reg[vm_reg_exp2]);
		    reg[vm_reg_val] = cell_cons(reg[vm_reg_exp],
						    reg[vm_reg_exp2]);
		    VmTrace fprint(2, "%O\n", reg[vm_reg_val]);
		    break;
		case vm_branch:
		    if (!reg[vm_reg_val]) {
			VmTrace fprint(2, "vm_run(): branch not taken\n");
			ip += 2;
			break;
		    }
		    VmTrace fprint(2, "vm_run(): branch taken\n");
		    /* fall through */
		case vm_jump:
		    operand = code[++ip] << 8;
		    operand += code[++ip];
		    VmTrace fprint(2, "vm_run(): jump to %d\n", operand);
		    /* easier to subtract 1 than to skip the "++ip" below */
		    ip = operand - 1;
		    break;
		case vm_loadcont:
		    operand = code[++ip] << 8;
		    operand += code[++ip];
		    VmTrace fprint(2, "vm_run(): load cont with %d\n", operand);
		    reg[vm_reg_cont] = cell_new_fxnum(operand);
		    break;
		case vm_continue:
		    ip = cell_car_fxnum(reg[vm_reg_cont]) - 1;
		    VmTrace fprint(2, "vm_run(): continue at %d\n", ip + 1);
		    break;
		}
	    }
	}
	++ip;
    }
    RefRemove(reg[vm_reg_exp2]); RefRemove(reg[vm_reg_fun]);
    RefRemove(reg[vm_reg_unev]); RefRemove(reg[vm_reg_argl]);
    RefRemove(reg[vm_reg_cont]); RefRemove(reg[vm_reg_env]);
    RefRemove(reg[vm_reg_val]); RefRemove(reg[vm_reg_exp]);
    RefEnd(stack);
}

void vm_reg_set(int r, void *val) {
    reg[r] = val;
}

Cell *vm_reg_get(int r) {
    return reg[r];
}
