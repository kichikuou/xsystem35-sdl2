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
#include "list.h"
#include "unittest.h"
#include <stdio.h>

static void list_func(void *data, void *user_data) {
	ASSERT_EQUAL(*(int*)data, ++*(int*)user_data);
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
		ASSERT_EQUAL(slist_length(l), 0);

		l = slist_append(l, &data1);
		ASSERT_EQUAL(slist_length(l), 1);
		ASSERT_EQUAL_PTR(l->data, &data1);
		ASSERT_NULL(slist_next(l));

		l = slist_append(l, &data2);
		ASSERT_EQUAL(slist_length(l), 2);
		ASSERT_EQUAL_PTR(l->data, &data1);
		ASSERT_EQUAL_PTR(slist_next(l)->data, &data2);
		ASSERT_NULL(slist_next(slist_next(l)));

		slist_free(l);
	}

	// slist_nth, slist_last
	{
		SList* l = NULL;
		ASSERT_NULL(slist_nth(l, 0));
		ASSERT_NULL(slist_nth(l, 1));
		ASSERT_NULL(slist_last(l));

		l = slist_append(l, &data1);
		ASSERT_EQUAL_PTR(slist_nth(l, 0)->data, &data1);
		ASSERT_NULL(slist_nth(l, 1));
		ASSERT_EQUAL_PTR(slist_last(l), slist_nth(l, 0));

		l = slist_append(l, &data2);
		ASSERT_EQUAL_PTR(slist_nth(l, 0)->data, &data1);
		ASSERT_EQUAL_PTR(slist_nth(l, 1)->data, &data2);
		ASSERT_NULL(slist_nth(l, 2));
		ASSERT_EQUAL_PTR(slist_last(l), slist_nth(l, 1));

		slist_free(l);
	}

	// slist_remove
	{
		SList* l = NULL;
		ASSERT_NULL(slist_remove(l, NULL));
		ASSERT_NULL(slist_remove(l, &data1));

		l = slist_append(l, &data1);
		ASSERT_EQUAL_PTR(slist_remove(l, NULL), l);
		l = slist_remove(l, &data1);
		ASSERT_NULL(l);

		l = slist_append(l, &data1);
		l = slist_append(l, &data2);
		l = slist_append(l, &data1);
		l = slist_append(l, &data2);
		ASSERT_EQUAL(slist_length(l), 4);
		l = slist_remove(l, &data2);
		ASSERT_EQUAL(slist_length(l), 3);
		ASSERT_EQUAL_PTR(slist_nth(l, 0)->data, &data1);
		ASSERT_EQUAL_PTR(slist_nth(l, 1)->data, &data1);
		ASSERT_EQUAL_PTR(slist_nth(l, 2)->data, &data2);
		l = slist_remove(l, &data2);
		ASSERT_EQUAL(slist_length(l), 2);
		ASSERT_EQUAL_PTR(slist_nth(l, 0)->data, &data1);
		ASSERT_EQUAL_PTR(slist_nth(l, 1)->data, &data1);

		slist_free(l);
	}

	// slist_index
	{
		SList* l = NULL;
		ASSERT_EQUAL(slist_index(l, NULL), -1);
		ASSERT_EQUAL(slist_index(l, &data1), -1);

		l = slist_append(l, &data1);
		ASSERT_EQUAL(slist_index(l, NULL), -1);
		ASSERT_EQUAL(slist_index(l, &data1), 0);

		l = slist_append(l, &data1);
		l = slist_append(l, &data2);
		ASSERT_EQUAL(slist_index(l, &data1), 0);
		ASSERT_EQUAL(slist_index(l, &data2), 2);

		slist_free(l);
	}

	// slist_foreach
	{
		SList* l = NULL;
		l = slist_append(l, &data1);
		l = slist_append(l, &data2);
		int count = 0;
		slist_foreach(l, list_func, &count);
		ASSERT_EQUAL(count, 2);
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
		ASSERT_EQUAL(slist_length(l), 4);
		ASSERT_EQUAL_PTR(slist_nth(l, 0)->data, &data0);
		ASSERT_EQUAL_PTR(slist_nth(l, 1)->data, &data1);
		ASSERT_EQUAL_PTR(slist_nth(l, 2)->data, &data2);
		ASSERT_EQUAL_PTR(slist_nth(l, 3)->data, &data3);

		slist_free(l);
	}

	return 0;
}
