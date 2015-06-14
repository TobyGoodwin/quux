#include <assert.h>

#include "assemble.h"
#include "cell.h"
#include "env.h"
#include "streq.h"
#include "vm.h"

#define AsTrace if (1)

/* XXX - sling? */
static byte byte_code[600];

static int reg_code(char *reg) {
    if (streq(reg, "argl")) return vm_reg_argl;
    if (streq(reg, "cont")) return vm_reg_cont;
    if (streq(reg, "env")) return vm_reg_env;
    if (streq(reg, "exp")) return vm_reg_exp;
    if (streq(reg, "exp2")) return vm_reg_exp2;
    if (streq(reg, "fun")) return vm_reg_fun;
    if (streq(reg, "val")) return vm_reg_val;
    if (streq(reg, "unev")) return vm_reg_unev;
    assert(0);
}

Cell *labels;

byte *assemble(Cell *code) {
    Cell *c;
    Cell *cl;
    Cell *slots = cell_nil;
    unsigned int ip;

//fprint(2, "assemble(): code is >%O<\n", code);

    labels = cell_nil;
    /* the first two bytes will hold the address of the "end" label */
    cl = cell_cons(cell_new_fxnum(0), cell_nil);
    cl = cell_cons_string("end", cl);
    slots = cell_cons(cl, slots);
    ip = 2;

    for (c = code; !cell_nullp(c); c = cell_cdr(c)) {
	Cell *line = cell_car(c);
	AsTrace fprintf(stderr, "assemble(): considering >%s<\n",
                cell_asprint(line));
	if (cell_atomp(line)) {
            /* it's a label: record current ip in labels */
            Cell *entry = cell_cons(line, cell_new_fxnum(ip));
            labels = cell_cons(entry, labels);
	} else {
	    char *ins = cell_car_string(line);
	    AsTrace fprintf(stderr, "got instruction >%s<\n", ins);
	    if (streq(ins, "move")) {
		int src = reg_code(cell_car_string(cell_cdr(line)));
		int dst = reg_code(cell_car_string(cell_cdr(cell_cdr(line))));
		byte_code[ip++] = vm_move | (src << 3) | dst;
	    } else if (streq(ins, "nil")) {
		int dst = reg_code(cell_car_string(cell_cdr(line)));
		byte_code[ip++] = vm_nil | dst;
	    } else if (streq(ins, "push")) {
		int dst = reg_code(cell_car_string(cell_cdr(line)));
		byte_code[ip++] = vm_push | dst;
	    } else if (streq(ins, "pop")) {
		int dst = reg_code(cell_car_string(cell_cdr(line)));
		byte_code[ip++] = vm_pop | dst;
	    } else if (streq(ins, "call")) {
		int fun = vm_encode_call(cell_car_string(cell_cdr(line)));
		byte_code[ip++] = vm_call;
		byte_code[ip++] = fun;
	    } else if (streq(ins, "cons")) {
		byte_code[ip++] = vm_cons;
	    } else if (streq(ins, "jump") || streq(ins, "branch") ||
		    streq(ins, "loadcont")) {
		char *l = cell_car_string(cell_cdr(line));
		Cell *tgt = cell_assoc(cell_cdr(line), labels);
		AsTrace fprintf(stderr,
                        "cell_assoc returned >%s<\n", cell_asprint(tgt));
		AsTrace fprintf(stderr,
                        "jump/branch/loadcont to label >%s<\n", l);
		if (streq(ins, "jump"))
		    byte_code[ip++] = vm_jump;
		else if (streq(ins, "branch"))
		    byte_code[ip++] = vm_branch;
		else
		    byte_code[ip++] = vm_loadcont;
		if (cell_fxnump(tgt)) {
		    /* we already know the value of this label, so we
		     * can simply insert it on this pass */
		    long t = cell_car_fxnum(tgt);
		    byte_code[ip++] = (t & 0xff00) >> 8;
		    byte_code[ip++] = t & 0xff;
		} else {
		    /* record the fact that we need to fill in the value
		     * of this label on the second pass */
		    Cell *cl = cell_cons(cell_new_fxnum(ip), cell_nil);
		    cl = cell_cons_string(l, cl);
		    slots = cell_cons(cl, slots);
		    AsTrace fprintf(stderr, "slots is now >%s<\n",
                            cell_asprint(slots));
		    byte_code[ip++] = 0;
		    byte_code[ip++] = 0;
		}
	    } else if (streq(ins, "continue")) {
		byte_code[ip++] = vm_continue;
	    } else {
		assert(0);
	    }
	}
    }
    byte_code[ip] = vm_end;

    /* assembler second pass: complete forward jump and branch
     * instructions
     */
    for (c = slots; !cell_nullp(c); c = cell_cdr(c)) {
	Cell *l = cell_car(c);
	char *lab = cell_car_string(l);
	int where = cell_car_fxnum(cell_cdr(l));
	void *tgt = cell_assoc(cell_car(l), labels);
	AsTrace fprintf(stderr, "fixing jump at %d, target is %s (%s)\n",
		    where, lab, cell_asprint(tgt));
        assert(!cell_nullp(tgt));
        assert(cell_fxnump(tgt));
        int t = cell_car_fxnum(tgt);
        byte_code[where] = (t & 0xff00) >> 8;
        byte_code[where + 1] = t & 0xff;
    }
    return byte_code;
}

/* cell_to_vmc() handles parse trees that are (nested) lists, removing
 * "%list", "%glob", and "%quote" annotations, and building the result we
 * would get if we had the (Schemely) cell_read().
 */
Cell *cell_to_vmc(Cell *c) {
    Cell *result;
    if (cell_nullp(c)) return cell_nil;
    if (list_headedP(c, "list"))
        c = cell_cdr(c);
    if (list_headedP(c, "glob")) {
	result = cell_cadadr(c);
    } else if (list_headedP(c, "quote")) {
	result = cell_cadr(c);
    } else if (cell_pairp(c)) {
	Cell *l = cell_to_vmc(cell_car(c));
	Cell *r = cell_to_vmc(cell_cdr(c));
	result = cell_cons(l, r);
    } else 
	 result = c;
    return result;
}
