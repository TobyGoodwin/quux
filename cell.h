#ifndef CELL_H
#define CELL_H 1

#include <stddef.h>

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

/* accessors */
extern Cell *cell_nullp(Cell *);
extern Cell *cell_pairp(Cell *);
extern Cell *cell_atomp(Cell *);
extern Cell *cell_cellp(Cell *);
extern int cell_numberp(Cell *);
extern Cell *cell_fxnump(Cell *);
extern int cell_stringp(Cell *);
extern int cell_closurep(Cell *);
extern Cell *cell_car(Cell *);
extern char *cell_car_string(Cell *);
extern long cell_car_fxnum(Cell *);
//extern Boolean cell_car_cellp(Cell *);
extern Cell *cell_car_cell(Cell *);
extern Cell *cell_cleaveM(Cell *);
extern Cell *cell_cdr(Cell *);
extern Cell *cell_cdr_cell(Cell *);

extern Cell *cell_caadr(Cell *);
extern Cell *cell_cadadr(Cell *);
extern Cell *cell_cadar(Cell *);
extern Cell *cell_cadddr(Cell *);
extern Cell *cell_caddr(Cell *);
extern Cell *cell_cadr(Cell *);
extern Cell *cell_cdaadr(Cell *);
extern Cell *cell_cdadr(Cell *);
extern Cell *cell_cddr(Cell *);

/* extra accessors for gc */
extern int _cell_car_t(Cell *);
extern int _cell_cdr_t(Cell *);
extern void *_cell_car_ptr(Cell *);
extern void *_cell_cdr_ptr(Cell *);
extern size_t _cell_size(void);

/* temporaries for changeover */
//extern List *cell_to_list(Cell *);
//extern Cell *cell_from_list(List *);
extern Cell *cell_to_vmc(Cell *);
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

extern Cell *list_reverse(Cell *);

#endif
