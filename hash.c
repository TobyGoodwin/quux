/* Public domain. */

#include "alloc.h"
#include "assert.h"
#include "bun_hash.h"
#include "byte.h"
#include "stralloc.h"

struct hash_item {
	unsigned int h;
	char *key;
	unsigned int klen;
	char *data;
	unsigned int dlen;
	struct hash_item *next;
};

#define HASHSTART 5381

static unsigned int hashadd(unsigned int h, unsigned char c) {
	h += h << 5;
	h ^= c;
	return h;
}

static int isprime(unsigned int n) {
	unsigned int i;
	for (i = 2; i <= n / 2; ++i)
		if (n % i == 0) return 0;
	return 1;
}

int hash_init(struct hash_table *ht, unsigned int n) {
	assert(isprime(n)); /* sufficient, but by no means necessary: most composites are OK */
	ht->n = n;
	ht->hashes = (struct hash_item **)alloc(n * sizeof(struct hash_item *));
	return (ht->hashes != 0);
}

int hash_insert_once(struct hash_table *ht, stralloc *key, stralloc *data, stralloc *current) {
	struct hash_item **hashes, *here, *new, *p;
	unsigned long h, i;

	hashes = ht->hashes;
	h = HASHSTART;
	for (i = 0; i < key->len; ++i)
		h = hashadd(h, key->s[i]);
	p = hashes[h % ht->n];
	here = 0;
	while (p) {
		if (p->h > h) break;
		if (p->h == h && p->klen == key->len && byte_equal(p->key, key->len, key->s)) {
			if (!stralloc_copyb(current, p->data, p->dlen)) return 0;
			return 1;
		}
		here = p;
		p = p-> next;
	}

	if (!data) return 1;

	new = (struct hash_item *)alloc(sizeof(struct hash_item));
	if (!new) return 0;
	new->h = h;
	new->klen = key->len;
	new->dlen = data->len;
	new->key = alloc(key->len);
	if (!new->key) return 0;
	byte_copy(new->key, key->len, key->s);
	new->data = alloc(data->len);
	if (!new->data) return 0;
	byte_copy(new->data, data->len, data->s);

	if (here) {
		new->next = here->next;
		here->next = new;
	} else {
		new->next = p;
		hashes[h % ht->n] = new;
	}

	return 1;
}
