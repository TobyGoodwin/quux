#ifndef CELL_H
#define CELL_H 1

#include <stddef.h>
#include <stdio.h>

typedef struct Cell Cell;

/* constructors */

/* the nil Cell */
extern Cell *cell_nil;

/* a true Cell */
extern Cell *cell_true;

/* new atomic Cell containing string */
extern Cell *cell_new_string(char *);
extern Cell *cell_new_fxnum(long);

/* cons a string onto a cell */
extern Cell *cell_cons_string(char *, Cell *);

/* cons and its more efficient (but mutating) friend */
extern Cell *cell_cons(Cell *, Cell *);
extern Cell *cell_consM(Cell *, Cell *);

/* mutators */
extern void cell_car_set(Cell *, Cell *);
extern void cell_cdr_set(Cell *, Cell *);
extern void cell_car_set_fxnum(Cell *, long);
extern void cell_car_set_string(Cell *, char *);

/* predicates */
int cell_nullp(Cell *);
int cell_pairp(Cell *);
int cell_atomp(Cell *);
int cell_cellp(Cell *);
int cell_numberp(Cell *);
int cell_fxnump(Cell *);
int cell_stringp(Cell *);

/* accessors */
extern Cell *cell_car(Cell *);
extern char *cell_car_string(Cell *);
extern long cell_car_fxnum(Cell *);
//extern Boolean cell_car_cellp(Cell *);
extern Cell *cell_car_cell(Cell *);
extern Cell *cell_cleaveM(Cell *);
extern Cell *cell_cdr(Cell *);
extern Cell *cell_cdr_cell(Cell *);

Cell *cell_caar(Cell *c);
Cell *cell_cadr(Cell *c);
Cell *cell_cdar(Cell *c);
Cell *cell_cddr(Cell *c);

Cell *cell_caaar(Cell *c);
Cell *cell_caadr(Cell *c);
Cell *cell_cadar(Cell *c);
Cell *cell_caddr(Cell *c);

Cell *cell_cdaar(Cell *c);
Cell *cell_cdadr(Cell *c);
Cell *cell_cddar(Cell *c);
Cell *cell_cdddr(Cell *c);

Cell *cell_caaaar(Cell *c);
Cell *cell_caaadr(Cell *c);
Cell *cell_caadar(Cell *c);
Cell *cell_caaddr(Cell *c);

Cell *cell_cadaar(Cell *c);
Cell *cell_cadadr(Cell *c);
Cell *cell_caddar(Cell *c);
Cell *cell_cadddr(Cell *c);

Cell *cell_cdaaar(Cell *c);
Cell *cell_cdaadr(Cell *c);
Cell *cell_cdadar(Cell *c);
Cell *cell_cdaddr(Cell *c);

Cell *cell_cddaar(Cell *c);
Cell *cell_cddadr(Cell *c);
Cell *cell_cdddar(Cell *c);
Cell *cell_cddddr(Cell *c);

/* extra accessors for gc */
extern int _cell_car_t(Cell *);
extern int _cell_cdr_t(Cell *);
extern void *_cell_car_ptr(Cell *);
extern void *_cell_cdr_ptr(Cell *);
extern size_t _cell_size(void);

/* temporaries for changeover */
//extern List *cell_to_list(Cell *);
//extern Cell *cell_from_list(List *);
//extern Cell *cell_from_tree(Tree *);

/* Cells as lists */

extern Cell *list_enlist(Cell *);

extern Cell *list_flatten(Cell *);
extern Cell *list_headedP(Cell *, char *);
extern int list_length(Cell *);

/* non-mutating append */
extern Cell *list_append(Cell *, Cell *);

/* mutating append */
extern void list_appendM(Cell *, Cell *);

Cell *cell_assoc(Cell *, Cell *);

extern Cell *list_reverse(Cell *);

int cell_fprint(FILE *, Cell *);
char *cell_asprint(Cell *);
int cell_fprint_flat(FILE *, Cell *, char *);
char *cell_asprint_flat(Cell *, char *);
#endif
