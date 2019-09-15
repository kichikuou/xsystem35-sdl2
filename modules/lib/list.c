/*
 * list.c  linked list (for GLib replacement)
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

#include <stdlib.h>

#include "list.h"

void slist_foreach(SList *list, ListFunc func, void *user_data) {
	for (; list; list = list->next)
		func(list->data, user_data);
}

void slist_free(SList *list) {
	while (list) {
		SList* next = list->next;
		free(list);
		list = next;
	}
}

SList* slist_append(SList *list, void *data) {
	SList* node = malloc(sizeof(SList));
	node->data = data;
	node->next = NULL;

	if (!list)
		return node;

	SList* last = list;
	while (last->next)
		last = last->next;
	last->next = node;
	return list;
}

SList* slist_remove(SList *list, const void *data) {
	if (!list)
		return NULL;

	SList* node = list;
	if (list->data == data) {
		list = list->next;
		free(node);
		return list;
	}

	for (; node->next; node = node->next) {
		if (node->next->data == data) {
			SList* nodeToRemove = node->next;
			node->next = nodeToRemove->next;
			free(nodeToRemove);
			break;
		}
	}
	return list;
}

int slist_index(SList *list, const void *data) {
	int i;
	for (i = 0; list; list = list->next, i++) {
		if (list->data == data)
			return i;
	}
	return -1;
}

unsigned slist_length(SList *list) {
	unsigned len;
	for (len = 0; list; list = list->next)
		len++;
	return len;
}

SList* slist_nth(SList* list, unsigned int n) {
	while (list && n--)
		list = list->next;
	return list;
}

SList* slist_last(SList *list) {
	if (list) {
		while (list->next)
			list = list->next;
	}
	return list;
}

SList* slist_insert_sorted(SList *list, void* data, CompareFunc func) {
	SList* node = malloc(sizeof(SList));
	node->data = data;

	SList* prev = NULL;
	SList* current = list;
	while (current && func(current->data, data) <= 0) {
		prev = current;
		current = current->next;
	}
	if (!prev) {
		node->next = list;
		return node;
	}
	prev->next = node;
	node->next = current;
	return list;
}
