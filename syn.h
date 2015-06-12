#include "cell.h"

Cell *syn_command(Cell *, Cell *);
Cell *syn_define(Cell *, Cell *);
Cell *syn_eval(Cell *);
Cell *syn_if(Cell *, Cell *, Cell *);
Cell *syn_lambda(Cell *, Cell *);
Cell *syn_prefix(char *, Cell *);
Cell *syn_sequence(Cell *, Cell *);
