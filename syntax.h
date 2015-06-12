#include "cell.h"

Cell *make_command(Cell *, Cell *);
Cell *make_define(Cell *, Cell *);
Cell *make_eval(Cell *);
Cell *make_if(Cell *, Cell *, Cell *);
Cell *make_lambda(Cell *, Cell *);
Cell *make_sequence(Cell *, Cell *);
Cell *prefix(const char *, Cell *);
