#include "cell.h"
#include "internal.h"
#include "streq.h"

Cell *echo(Cell *a) {
    printf("%s\n", cell_asprint(a));
    return cell_nil;
}

Cell *internal(char *f, Cell *args) {
    if (streq(f, "%echo"))
        return echo(args);
    return cell_nil;
}
