#include <stdio.h>

#include "assemble.h"
#include "env.h"
#include "input.h"
#include "vm.h"

int main(void) {
    Cell *c, *env, *quux;
    byte *interp;

    c = parse_string("%echo hello world");
    quux = parse_file("quux.vmc");
    quux = cell_caddr(quux); /* remove unneeded "(define vmc ...)" */
    interp = assemble(quux);
    env = env_frame(cell_nil, cell_nil, cell_nil);
    vm_reg_set(vm_reg_env, e);
    vm_reg_set(vm_reg_exp, c);
    vm_run(interpreter);

    return 0;
}
