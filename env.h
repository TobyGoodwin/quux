#include "cell.h"

Cell *env_frame(Cell *env, Cell *name, Cell *value);
void env_bind(Cell *env, Cell *name, Cell *value);
Cell *env_lookup(Cell *env, Cell *name);
