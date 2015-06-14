/* XXX should split this stuff out better */
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <gc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cell.h"
#include "input.h"
#include "internal.h"
#include "streq.h"

static Cell *echo(Cell *a) {
    printf("%s\n", cell_asprint_flat(a, " "));
    return cell_cons_string("echo_done", cell_nil);
}

static Cell *intl_parse_string(Cell *a) {
    if (cell_nullp(a)) return cell_nil;

    return parse_string(cell_car_string(a));
}

static Cell *intl_exit(Cell *a) {
    printf("exit %s\n", cell_asprint(a));
    exit(0);
}

/* XXX assumes read will slurp entire file
 * XXX why not mmap?
 */
static Cell *read_file(Cell *a) {
    Cell *r;
    char *f = cell_car_string(a);
    char *s;
    int fd;
    struct stat statbuf;

    fprintf(stderr, "read_file %s\n", cell_asprint(a));
    fd = open(f, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "cannot open `%s': %s\n", f, strerror(errno));
        exit(1);
    }
    if (fstat(fd, &statbuf) < 0) {
        fprintf(stderr, "cannot stat `%s': %s\n", f, strerror(errno));
        exit(1);
    }
    /* XXX this shouldn't be a 0-terminated string */
    s = GC_MALLOC(statbuf.st_size + 1);
    if (read(fd, s, statbuf.st_size) != statbuf.st_size) {
        fprintf(stderr, "cannot stat `%s': %s\n", f, strerror(errno));
        exit(1);
    }
    close(fd);
    s[statbuf.st_size] = '\0';
    r = cell_new_string(s);

    return r;
}

/* XXX obviously, needs proper dispatch */
Cell *internal(char *f, Cell *args) {
    if (streq(f, "%echo"))
        return echo(args);
    if (streq(f, "echo"))
        return echo(args);
    if (streq(f, "exit"))
        return intl_exit(args);
    if (streq(f, "parse-string"))
        return intl_parse_string(args);
    if (streq(f, "read-file"))
        return read_file(args);
    return cell_nil;
}
