#include <stdio.h>
#include <stdlib.h>

#include "assemble.h"
#include "cell.h"
#include "env.h"
#include "input.h"
#include "vm.h"

int main(void) {
    Cell *env, *ns, *vs;
    Cell *c, *quux;
    byte *interp;

    ns = cell_cons_string("list", cell_nil);
    ns = parse_string("%(cons eval list)");
    fprintf(stderr, "ns is %s\n", cell_asprint(ns));
    vs = parse_string("%((core cons) (core eval) (closure (x x) ()))"); 
    fprintf(stderr, "vs is %s\n", cell_asprint(vs));
    env = cell_cons(cell_cons(ns, vs), cell_nil);
    fprintf(stderr, "env is %s\n", cell_asprint(env));
    vm_reg_set(vm_reg_env, env);

    //c = parse_string("read-file = internal read-file; read-file prelude.quux");
    c = parse_string("(internal echo) $${ (internal parse-string) ${ (internal read-file) prelude.quux } }");
    //c = parse_string("%(eval (list (list (quote internal) (quote echo)) (list (quote quote) (quote foo))))");
    //c = parse_string("(begin (define echo (list (quote internal) (quote echo))) (echo (quote hello) (quote from) (quote prelude)))");
    //c = parse_string("%((internal echo) (list (quote hello) (quote world)))");
    //c = parse_string("echo = internal echo; echo hello world");
    //c = parse_string("(internal exit) $${ (internal parse-string) ${ (internal read-file) prelude.quux } }");
    fprintf(stderr, "c is %s\n", cell_asprint(c));
    //exit(0);

    quux = parse_file("quux.vmc");
    quux = cell_caddr(quux); /* remove unneeded "(define vmc ...)" */
    quux = cell_to_vmc(quux);
    interp = assemble(quux);
    vm_reg_set(vm_reg_exp, c);
    fprintf(stderr, "exp is %s\n", cell_asprint(vm_reg_get(vm_reg_exp)));
    vm_run(interp);
    fprintf(stderr, "val is %s\n", cell_asprint(vm_reg_get(vm_reg_val)));

    return 0;
}
