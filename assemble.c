#include <assert.h>

#include "assemble.h"
#include "cell.h"
#include "streq.h"
#include "vm.h"

#define AsTrace if (0)

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

byte *assemble(Cell *code) {
    Cell *c = cell_nil;
    Dict *labels = mkdict();
    Cell *labels2 = cell_nil;
    unsigned int ip;

//fprint(2, "assemble(): code is >%O<\n", code);

    /* the first two bytes will hold the address of the "end" label */
    Ref(Cell *, cl, cell_cons(cell_new_fxnum(0), cell_nil));
    cl = cell_cons_string("end", cl);
    labels2 = cell_cons(cl, labels2);
    RefEnd(cl);
    ip = 2;

    for (c = code; !cell_nullp(c); c = cell_cdr(c)) {
	Ref(Cell *, line, cell_car(c));
	AsTrace fprint(2, "assemble(): considering >%O<\n", line);
	if (cell_atomp(line)) {
	    Ref(char *, lab, cell_car_string(line));
	    AsTrace fprint(2, "got label >%s<\n", lab);
	    /* XXX: we need to record the current value of the
	     * instruction pointer, which is of type int. Our
	     * dictionaries can only store values of type void *. For
	     * now, just assume that they are compatible types. In the
	     * future, this needs to be tidied up. */
	    labels = dictput(labels, lab, ip);
	    RefEnd(lab);
	} else {
	    Ref(char *, ins, cell_car_string(line));
	    AsTrace fprint(2, "got instruction >%s<\n", ins);
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
		Ref(char *, l, cell_car_string(cell_cdr(line)));
		void *tgt = dictget(labels, l);
		AsTrace fprint(2, "jump/branch/loadcont to label >%s<\n", l);
		if (streq(ins, "jump"))
		    byte_code[ip++] = vm_jump;
		else if (streq(ins, "branch"))
		    byte_code[ip++] = vm_branch;
		else
		    byte_code[ip++] = vm_loadcont;
		if (tgt) {
		    /* we already know the value of this label, so we
		     * can simply insert it on this pass */
		    int t = tgt; /* XXX: see above */
		    byte_code[ip++] = (t & 0xff00) >> 8;
		    byte_code[ip++] = t & 0xff;
		} else {
		    /* record the fact that we need to fill in the value
		     * of this label on the second pass */
		    Ref(Cell *, cl, cell_cons(cell_new_fxnum(ip), cell_nil));
		    cl = cell_cons_string(l, cl);
		    labels2 = cell_cons(cl, labels2);
		    AsTrace fprint(2, "labels2 is now >%O<\n", labels2);
		    RefEnd(cl);
		    byte_code[ip++] = 0;
		    byte_code[ip++] = 0;
		}
		RefEnd(l);
	    } else if (streq(ins, "continue")) {
		byte_code[ip++] = vm_continue;
	    } else {
		assert(0);
	    }
	    RefEnd(ins);
	}
	RefEnd(line);
    }
    byte_code[ip] = vm_end;

    /* assembler second pass: complete forward jump and branch
     * instructions
     */
    for (c = labels2; !cell_nullp(c); c = cell_cdr(c)) {
	Ref(Cell *, l, cell_car(c));
	Ref(char *, lab, cell_car_string(l));
	int where = cell_car_fxnum(cell_cdr(l));
	void *tgt = dictget(labels, lab);
	AsTrace fprint(2, "fixing jump at %d, target is %s (%d)\n",
		    where, lab, tgt);
	if (tgt) {
	    int t = tgt; /* XXX: see above */
	    byte_code[where] = (t & 0xff00) >> 8;
	    byte_code[where + 1] = t & 0xff;
	} else
	    assert(0);
	RefEnd2(lab, l);
    }
    RefEnd4(labels2, labels, c, code);
    return byte_code;
}
