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

#ifndef __MSGQUEUE_H__
#define __MSGQUEUE_H__

#include <stdbool.h>
#include <SDL_mutex.h>

struct msgq_elem;

struct msgq {
	SDL_mutex *mutex;
	SDL_cond *cond_nonempty;
	struct msgq_elem *head;
	struct msgq_elem *last;
};

static inline bool msgq_isempty(struct msgq *q) {
	return !q->head;
}

struct msgq *msgq_new(void);
void msgq_free(struct msgq *q);
void msgq_enqueue(struct msgq *q, void *msg);
void *msgq_dequeue(struct msgq *q);
void *msgq_dequeue_timeout(struct msgq *q, uint32_t timeout_ms);

#endif // __MSGQUEUE_H__
