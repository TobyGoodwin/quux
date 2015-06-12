enum vm_reg {
    vm_reg_exp, vm_reg_val, vm_reg_env, vm_reg_cont,
    vm_reg_argl, vm_reg_unev, vm_reg_fun, vm_reg_exp2,
    vm_rmask = 0007
};

enum vm_instr2 {
    vm_i5 =     0000,
    vm_move =   0100,
    vm_end =    0200,
    vm_imask2 = 0300
};

enum vm_instr5 {
    vm_i8 =     0000,
    vm_nil =    0010,
    vm_push =   0020,
    vm_pop =    0030,
    vm_imask5 = 0370
};

enum vm_instr8 {
    vm_call =     0001,
    vm_cons =     0002,
    vm_jump =     0003,
    vm_branch =   0004,
    vm_loadcont = 0005,
    vm_continue = 0006
};

enum vm_source {
    vm_src_exp = vm_reg_exp << 3,
    vm_smask = 0070
};

void vm_run(byte *);
void vm_reg_set(int, void *);
Cell *vm_reg_get(int);
byte vm_encode_call(char *);
