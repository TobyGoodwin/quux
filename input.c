#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "input.h"

Cell *parse_mem(char *a, size_t s) {
    Cell *result;
    int r;

    struct pacc_parser *pp = pacc_new();
    pacc_input(pp, a, s);
    r = pacc_parse(pp);
    fprintf(stderr, "parse_mem() returns %d\n", r);
    if (r) {
        result = pacc_result(pp);
        fprintf(stderr, "==> %s\n", cell_asprint(result));
        return result;
    }
    return 0;
}

Cell *parse_file(char *fn) {
    Cell *r;
    char *addr;
    int fd;
    struct stat s;

    if ((fd = open(fn, O_RDONLY)) == -1) {
        fprintf(stderr, "cannot open `%s' for input: %s\n", fn,
                strerror(errno));
        return cell_nil;
    }
    if (fstat(fd, &s) != 0) {
        fprintf(stderr, "cannot stat `%s': %s\n", fn, strerror(errno));
        return cell_nil;
    }
    // XXX check for size == 0

    addr = (char *) -1;
    addr = mmap(0, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == (char *)-1) {
        fprintf(stderr, "cannot mmap `%s': %s\n", strerror(errno));
        return cell_nil;
    }
    r = parse_mem(addr, s.st_size);
    munmap(addr, s.st_size);
    close(fd);
    return r;
}

Cell *parse_string(char *s) {
    return parse_mem(s, strlen(s));
}
