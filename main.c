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

    env = env_frame(cell_nil, cell_nil, cell_nil);
    //c = parse_string("\"(sequence (define echo (internal echo)) (echo (quote hello) (quote world)))");
    //env_bind(env, cell_new_string("l"), c);
    vm_reg_set(vm_reg_env, env);

    //c = parse_string("echo = internal echo; echo hello world");
    c = parse_string("\"(sequence (define x (lambda x ((internal echo) x))) (x (quote one) (quote two)))");
    fprintf(stderr, "c is %s\n", cell_asprint(c));
    //c = parse_string("eval x = { x }; (internal exit) $${ (internal parse-string) ${ (internal read-file) prelude.es } }");
    //c = parse_string("\"(l (quote %echo) (quote hello) (quote world))");
    //exit(0);

    quux = parse_file("quux.vmc");
    quux = cell_caddr(quux); /* remove unneeded "(define vmc ...)" */
    quux = cell_to_vmc(quux);
    interp = assemble(quux);
    vm_reg_set(vm_reg_exp, c);
    fprintf(stderr, "exp is %s\n", cell_asprint(vm_reg_get(vm_reg_exp)));
    vm_run(interp);

    return 0;
}
