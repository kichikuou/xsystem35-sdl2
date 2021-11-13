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
	q->mutex = SDL_CreateMutex();
	q->cond_nonempty = SDL_CreateCond();
	return q;
}

void msgq_free(struct msgq *q) {
	SDL_DestroyMutex(q->mutex);
	SDL_DestroyCond(q->cond_nonempty);
	free(q);
}

void msgq_enqueue(struct msgq *q, void *msg) {
	struct msgq_elem *e = malloc(sizeof(struct msgq_elem));
	e->msg = msg;
	e->next = NULL;

	SDL_LockMutex(q->mutex);
	if (!q->head) {
		q->head = q->last = e;
	} else {
		q->last->next = e;
		q->last = e;
	}
	SDL_UnlockMutex(q->mutex);
	SDL_CondSignal(q->cond_nonempty);
}

void *msgq_dequeue(struct msgq *q) {
	SDL_LockMutex(q->mutex);
	while (!q->head)
		SDL_CondWait(q->cond_nonempty, q->mutex);

	struct msgq_elem *e = q->head;
	q->head = e->next;
	if (!e->next)
		q->last = NULL;

	SDL_UnlockMutex(q->mutex);

	void *msg = e->msg;
	free(e);
	return msg;
}

void *msgq_dequeue_timeout(struct msgq *q, uint32_t timeout_ms) {
	SDL_LockMutex(q->mutex);

	while (!q->head && SDL_CondWaitTimeout(q->cond_nonempty, q->mutex, timeout_ms) == 0)
		;

	if (!q->head) {  // timed out
		SDL_UnlockMutex(q->mutex);
		return NULL;
	}

	struct msgq_elem *e = q->head;
	q->head = e->next;
	if (!e->next)
		q->last = NULL;

	SDL_UnlockMutex(q->mutex);

	void *msg = e->msg;
	free(e);
	return msg;
}
