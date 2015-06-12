#include "stralloc.h"

struct hash_table {
	struct hash_item **hashes;
	unsigned int n;
};

int hash_init(struct hash_table *, unsigned int);
int hash_insert_once(struct hash_table *, stralloc *, stralloc *, stralloc *);
