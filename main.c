#include <stdio.h>

#include "assemble.h"
#include "cell.h"
#include "env.h"
#include "input.h"
#include "vm.h"

int main(void) {
    Cell *c, *env, *quux;
    byte *interp;

    c = parse_string("%echo hello world");
    quux = parse_file("quux.vmc");
    quux = cell_caddr(quux); /* remove unneeded "(define vmc ...)" */
    quux = cell_to_vmc(quux);
    interp = assemble(quux);
#if 0
    env = env_frame(cell_nil, cell_nil, cell_nil);
    vm_reg_set(vm_reg_env, env);
    vm_reg_set(vm_reg_exp, c);
    vm_run(interp);
#endif

    return 0;
}
