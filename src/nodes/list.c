/*
 * @Descripttion : 
 * @version      : 
 * @Author       : sannmizu
 * @Date         : 2020-04-06 16:50:18
 * @LastEditors  : sannmizu
 * @LastEditTime : 2020-05-24 23:54:47
 */
#include <stdlib.h>
#include "nodes/list.h"

static List *
new_list()
{
	List	   *new_list;
	ListCell   *new_head;

	new_head = (ListCell *) malloc(sizeof(*new_head));
	new_head->next = NULL;
	/* new_head->data is left undefined! */

	new_list = (List *) malloc(sizeof(*new_list));
	new_list->length = 1;
	new_list->head = new_head;
	new_list->tail = new_head;

	return new_list;
}

static void
new_head_cell(List *list)
{
	ListCell   *new_head;

	new_head = (ListCell *) malloc(sizeof(*new_head));
	new_head->next = list->head;

	list->head = new_head;
	list->length++;
}

static void
new_tail_cell(List *list)
{
	ListCell   *new_tail;

	new_tail = (ListCell *) malloc(sizeof(*new_tail));
	new_tail->next = NULL;

	list->tail->next = new_tail;
	list->tail = new_tail;
	list->length++;
}

List *
lappend(List *list, void *datum)
{
	if (list == NIL)
		list = new_list();
	else
		new_tail_cell(list);

	lfirst(list->tail) = datum;
	return list;
}

static ListCell *
add_new_cell(List *list, ListCell *prev_cell)
{
	ListCell   *new_cell;

	new_cell = (ListCell *) malloc(sizeof(*new_cell));
	/* new_cell->data is left undefined! */
	if (prev_cell != NULL) {
		new_cell->next = prev_cell->next;
		prev_cell->next = new_cell;
	} else {
		new_cell->next = list_head(list);
		list->head = new_cell;
	}

	if (list->tail == prev_cell)
		list->tail = new_cell;

	list->length++;

	return new_cell;
}

ListCell *
lappend_cell(List *list, ListCell *prev, void *datum)
{
	ListCell   *new_cell;

	new_cell = add_new_cell(list, prev);
	lfirst(new_cell) = datum;
	return new_cell;
}

List *
lcons(void *datum, List *list)
{
	if (list == NIL)
		list = new_list();
	else
		new_head_cell(list);

	lfirst(list->head) = datum;
	return list;
}

List *
list_delete_cell(List *list, ListCell *cell, ListCell *prev)
{
	if (list->length == 1)
	{
		list_free(list);
		return NIL;
	}
	
	list->length--;

	if (prev)
		prev->next = cell->next;
	else
		list->head = cell->next;

	if (list->tail == cell)
		list->tail = prev;

	free(cell);
	return list;
}

static void
list_free_private(List *list, int deep)
{
	ListCell   *cell;

	cell = list_head(list);
	while (cell != NULL)
	{
		ListCell   *tmp = cell;

		cell = lnext(cell);
		if (deep)
			free(lfirst(tmp));
		free(tmp);
	}

	if (list)
		free(list);
}

void
list_free(List *list)
{
	list_free_private(list, 0);
}

void
list_free_deep(List *list)
{
	/*
	 * A "deep" free operation only makes sense on a list of pointers.
	 */
	list_free_private(list, 1);
}

List *
list_copy(const List *oldlist)
{
	List	   *newlist;
	ListCell   *newlist_prev;
	ListCell   *oldlist_cur;

	if (oldlist == NIL)
		return NIL;

	newlist = new_list();
	newlist->length = oldlist->length;

	/*
	 * Copy over the data in the first cell; new_list() has already allocated
	 * the head cell itself
	 */
	newlist->head->data = oldlist->head->data;

	newlist_prev = newlist->head;
	oldlist_cur = oldlist->head->next;
	while (oldlist_cur)
	{
		ListCell   *newlist_cur;

		newlist_cur = (ListCell *) malloc(sizeof(*newlist_cur));
		newlist_cur->data = oldlist_cur->data;
		newlist_prev->next = newlist_cur;

		newlist_prev = newlist_cur;
		oldlist_cur = oldlist_cur->next;
	}

	newlist_prev->next = NULL;
	newlist->tail = newlist_prev;

	return newlist;
}