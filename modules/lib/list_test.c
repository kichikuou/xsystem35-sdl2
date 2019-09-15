/*
 * Copyright (C) 2019 <KichikuouChrome@gmail.com>
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
#undef NDEBUG
#include "list.h"
#include <assert.h>
#include <stdio.h>

static void list_func(void *data, void *user_data) {
	assert(*(int*)data == ++*(int*)user_data);
}

static int compare_func(const void *a, const void *b) {
	return *(int*)a - *(int*)b;
}

int main() {
	int data1 = 1;
	int data2 = 2;

	// Basic
	{
		SList* l = NULL;
		assert(slist_length(l) == 0);

		l = slist_append(l, &data1);
		assert(slist_length(l) == 1);
		assert(l->data == &data1);
		assert(slist_next(l) == NULL);

		l = slist_append(l, &data2);
		assert(slist_length(l) == 2);
		assert(l->data == &data1);
		assert(slist_next(l)->data == &data2);
		assert(slist_next(slist_next(l)) == NULL);

		slist_free(l);
	}

	// slist_nth, slist_last
	{
		SList* l = NULL;
		assert(slist_nth(l, 0) == NULL);
		assert(slist_nth(l, 1) == NULL);
		assert(slist_last(l) == NULL);

		l = slist_append(l, &data1);
		assert(slist_nth(l, 0)->data == &data1);
		assert(slist_nth(l, 1) == NULL);
		assert(slist_last(l) == slist_nth(l, 0));

		l = slist_append(l, &data2);
		assert(slist_nth(l, 0)->data == &data1);
		assert(slist_nth(l, 1)->data == &data2);
		assert(slist_nth(l, 2) == NULL);
		assert(slist_last(l) == slist_nth(l, 1));

		slist_free(l);
	}

	// slist_remove
	{
		SList* l = NULL;
		assert(slist_remove(l, NULL) == NULL);
		assert(slist_remove(l, &data1) == NULL);

		l = slist_append(l, &data1);
		assert(slist_remove(l, NULL) == l);
		l = slist_remove(l, &data1);
		assert(l == NULL);

		l = slist_append(l, &data1);
		l = slist_append(l, &data2);
		l = slist_append(l, &data1);
		l = slist_append(l, &data2);
		assert(slist_length(l) == 4);
		l = slist_remove(l, &data2);
		assert(slist_length(l) == 3);
		assert(slist_nth(l, 0)->data == &data1);
		assert(slist_nth(l, 1)->data == &data1);
		assert(slist_nth(l, 2)->data == &data2);
		l = slist_remove(l, &data2);
		assert(slist_length(l) == 2);
		assert(slist_nth(l, 0)->data == &data1);
		assert(slist_nth(l, 1)->data == &data1);

		slist_free(l);
	}

	// slist_index
	{
		SList* l = NULL;
		assert(slist_index(l, NULL) == -1);
		assert(slist_index(l, &data1) == -1);

		l = slist_append(l, &data1);
		assert(slist_index(l, NULL) == -1);
		assert(slist_index(l, &data1) == 0);

		l = slist_append(l, &data1);
		l = slist_append(l, &data2);
		assert(slist_index(l, &data1) == 0);
		assert(slist_index(l, &data2) == 2);

		slist_free(l);
	}

	// slist_foreach
	{
		SList* l = NULL;
		l = slist_append(l, &data1);
		l = slist_append(l, &data2);
		int count = 0;
		slist_foreach(l, list_func, &count);
		assert(count == 2);
		slist_free(l);
	}

	// slist_insert_sorted
	{
		int data0 = 0;
		int data3 = 3;
		SList* l = NULL;
		l = slist_insert_sorted(l, &data1, compare_func);
		l = slist_insert_sorted(l, &data3, compare_func);
		l = slist_insert_sorted(l, &data0, compare_func);
		l = slist_insert_sorted(l, &data2, compare_func);
		assert(slist_length(l) == 4);
		assert(slist_nth(l, 0)->data == &data0);
		assert(slist_nth(l, 1)->data == &data1);
		assert(slist_nth(l, 2)->data == &data2);
		assert(slist_nth(l, 3)->data == &data3);

		slist_free(l);
	}

	return 0;
}
