/*
 * Copyright (C) 2021 kichikuou <KichikuouChrome@gmail.com>
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
#include "msgqueue.h"

struct msgq_elem {
	void *msg;
	struct msgq_elem *next;
};

struct msgq *msgq_new(void) {
	struct msgq *q = calloc(1, sizeof(struct msgq));
	q->mutex = sdl_create_mutex();
	q->cond_nonempty = sdl_create_condition();
	return q;
}

void msgq_free(struct msgq *q) {
	sdl_destroy_mutex(q->mutex);
	sdl_destroy_condition(q->cond_nonempty);
	free(q);
}

void msgq_enqueue(struct msgq *q, void *msg) {
	struct msgq_elem *e = malloc(sizeof(struct msgq_elem));
	e->msg = msg;
	e->next = NULL;

	sdl_lock_mutex(q->mutex);
	if (!q->head) {
		q->head = q->last = e;
	} else {
		q->last->next = e;
		q->last = e;
	}
	sdl_unlock_mutex(q->mutex);
	sdl_signal_condition(q->cond_nonempty);
}

void *msgq_dequeue(struct msgq *q) {
	sdl_lock_mutex(q->mutex);
	while (!q->head)
		sdl_wait_condition(q->cond_nonempty, q->mutex);

	struct msgq_elem *e = q->head;
	q->head = e->next;
	if (!e->next)
		q->last = NULL;

	sdl_unlock_mutex(q->mutex);

	void *msg = e->msg;
	free(e);
	return msg;
}

void *msgq_dequeue_timeout(struct msgq *q, uint32_t timeout_ms) {
	sdl_lock_mutex(q->mutex);

	while (!q->head &&
	       sdl_wait_condition_timeout(q->cond_nonempty, q->mutex, timeout_ms))
		;

	if (!q->head) {  // timed out
		sdl_unlock_mutex(q->mutex);
		return NULL;
	}

	struct msgq_elem *e = q->head;
	q->head = e->next;
	if (!e->next)
		q->last = NULL;

	sdl_unlock_mutex(q->mutex);

	void *msg = e->msg;
	free(e);
	return msg;
}
