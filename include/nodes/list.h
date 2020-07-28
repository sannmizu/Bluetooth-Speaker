/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-04-06 16:54:34
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 22:29:45
 */
#ifndef SANN_LIST_H
#define SANN_LIST_H

#include "list_types.h"

static inline ListCell *
list_head(const List *l)
{
	return l ? l->head : NULL;
}

static inline ListCell *
list_tail(List *l)
{
	return l ? l->tail : NULL;
}

static inline int
list_length(const List *l)
{
	return l ? l->length : 0;
}

#define lnext(lc)				((lc)->next)
#define lfirst(lc)				((lc)->data)
#define linitial(l)				lfirst(list_head(l))
#define list_make1(x1)			lcons(x1, NIL)

extern List *lappend(List *list, void *datum);
extern ListCell *lappend_cell(List *list, ListCell *prev, void *datum);
extern List *lcons(void *datum, List *list);
extern List *list_delete_cell(List *list, ListCell *cell, ListCell *prev);

typedef int (*list_qsort_comparator) (const void *a, const void *b);
extern ListCell *lappend_sort(List *list, void *datum, list_qsort_comparator cmp);

extern void list_free(List *list);
extern void list_free_deep(List *list);
extern List *list_copy(const List *list);
#endif