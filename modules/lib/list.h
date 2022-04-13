/*
 * list.h  linked list (for GLib replacement)
 *
 * Copyright (C) 2014-2017 <KichikuouChrome@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __LIST_H_
#define __LIST_H_

typedef struct SList_t {
	void *data;
	struct SList_t *next;
} SList;

typedef void (*ListFunc)(void *data, void *user_data);
typedef int (*CompareFunc)(const void *a, const void *b);

#define slist_next(lst) (lst)->next
void slist_foreach(SList *list, ListFunc func, void *user_data);
void slist_free(SList *list);
SList* slist_append(SList *list, void *data);
SList* slist_remove(SList *list, const void *data);
int slist_index(SList *list, const void *data);
unsigned slist_length(SList *list);
SList* slist_nth(SList *list, unsigned int n);
SList* slist_last(SList *list);
SList* slist_insert_sorted(SList *list, void* data, CompareFunc func);

typedef SList List;

#define list_next slist_next
#define list_append slist_append
#define list_remove slist_remove
#define list_index slist_index
#define list_length slist_length
#define list_nth slist_nth
#define list_last slist_last

#endif // __LIST_H_
