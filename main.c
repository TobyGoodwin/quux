#include <stdio.h>
#include <stdlib.h>

#include "assemble.h"
#include "cell.h"
#include "env.h"
#include "input.h"
#include "vm.h"

int main(void) {
    Cell *c, *env, *quux;
    byte *interp;

    //c = parse_string("echo = (internal echo); echo hello world");
    c = parse_string("eval x = { x }; (internal exit) $${ (internal parse-string) ${ (internal read-file) prelude.es } }");
    //c = parse_string("(internal exit) $ $ { (internal parse-string) $ { (internal read-file) prelude.es  }");
    //c = parse_string("c = (echo hello world)");
    //c = cell_cadr(c);
    fprintf(stderr, "c is %s\n", cell_asprint(c));
    //exit(1);

    quux = parse_file("quux.vmc");
    quux = cell_caddr(quux); /* remove unneeded "(define vmc ...)" */
    quux = cell_to_vmc(quux);
    interp = assemble(quux);
    env = env_frame(cell_nil, cell_nil, cell_nil);
    vm_reg_set(vm_reg_env, env);
    vm_reg_set(vm_reg_exp, c);
    fprintf(stderr, "exp is %s\n", cell_asprint(vm_reg_get(vm_reg_exp)));
    vm_run(interp);

    return 0;
}
