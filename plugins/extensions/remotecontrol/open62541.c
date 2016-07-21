/* THIS IS A SINGLE-FILE DISTRIBUTION CONCATENATED FROM THE OPEN62541 SOURCES 
 * visit http://open62541.org/ for information about this software
 * Git-Revision: v0.2.0-RC1
 */
 
 /*
 * Copyright (C) 2015 the contributors as stated in the AUTHORS file
 *
 * This file is part of open62541. open62541 is free software: you can
 * redistribute it and/or modify it under the terms of the GNU Lesser General
 * Public License, version 3 (as published by the Free Software Foundation) with
 * a static linking exception as stated in the LICENSE file provided with
 * open62541.
 *
 * open62541 is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */


 #ifndef UA_DYNAMIC_LINKING
# define UA_DYNAMIC_LINKING
#endif

#ifndef UA_INTERNAL
#define UA_INTERNAL
#endif

#include "open62541.h"

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/deps/queue.h" ***********************************/

/*	$OpenBSD: queue.h,v 1.38 2013/07/03 15:05:21 fgsch Exp $	*/
/*	$NetBSD: queue.h,v 1.11 1996/05/16 05:17:14 mycroft Exp $	*/

/*
 * Copyright (c) 1991, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)queue.h	8.5 (Berkeley) 8/20/94
 */

#ifndef	_SYS_QUEUE_H_
#define	_SYS_QUEUE_H_

/*
 * This file defines five types of data structures: singly-linked lists, 
 * lists, simple queues, tail queues, and circular queues.
 *
 *
 * A singly-linked list is headed by a single forward pointer. The elements
 * are singly linked for minimum space and pointer manipulation overhead at
 * the expense of O(n) removal for arbitrary elements. New elements can be
 * added to the list after an existing element or at the head of the list.
 * Elements being removed from the head of the list should use the explicit
 * macro for this purpose for optimum efficiency. A singly-linked list may
 * only be traversed in the forward direction.  Singly-linked lists are ideal
 * for applications with large datasets and few or no removals or for
 * implementing a LIFO queue.
 *
 * A list is headed by a single forward pointer (or an array of forward
 * pointers for a hash table header). The elements are doubly linked
 * so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before
 * or after an existing element or at the head of the list. A list
 * may only be traversed in the forward direction.
 *
 * A simple queue is headed by a pair of pointers, one the head of the
 * list and the other to the tail of the list. The elements are singly
 * linked to save space, so elements can only be removed from the
 * head of the list. New elements can be added to the list before or after
 * an existing element, at the head of the list, or at the end of the
 * list. A simple queue may only be traversed in the forward direction.
 *
 * A tail queue is headed by a pair of pointers, one to the head of the
 * list and the other to the tail of the list. The elements are doubly
 * linked so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before or
 * after an existing element, at the head of the list, or at the end of
 * the list. A tail queue may be traversed in either direction.
 *
 * A circle queue is headed by a pair of pointers, one to the head of the
 * list and the other to the tail of the list. The elements are doubly
 * linked so that an arbitrary element can be removed without a need to
 * traverse the list. New elements can be added to the list before or after
 * an existing element, at the head of the list, or at the end of the list.
 * A circle queue may be traversed in either direction, but has a more
 * complex end of list detection.
 *
 * For details on the use of these macros, see the queue(3) manual page.
 */

#if defined(QUEUE_MACRO_DEBUG) || (defined(_KERNEL) && defined(DIAGNOSTIC))
#define _Q_INVALIDATE(a) (a) = ((void *)-1)
#else
#define _Q_INVALIDATE(a)
#endif

/*
 * Singly-linked List definitions.
 */
#define SLIST_HEAD(name, type)						\
struct name {								\
	struct type *slh_first;	/* first element */			\
}
 
#define	SLIST_HEAD_INITIALIZER(head)					\
	{ NULL }
 
#define SLIST_ENTRY(type)						\
struct {								\
	struct type *sle_next;	/* next element */			\
}
 
/*
 * Singly-linked List access methods.
 */
#define	SLIST_FIRST(head)	((head)->slh_first)
#define	SLIST_END(head)		NULL
#define	SLIST_EMPTY(head)	(SLIST_FIRST(head) == SLIST_END(head))
#define	SLIST_NEXT(elm, field)	((elm)->field.sle_next)

#define	SLIST_FOREACH(var, head, field)					\
	for((var) = SLIST_FIRST(head);					\
	    (var) != SLIST_END(head);					\
	    (var) = SLIST_NEXT(var, field))

#define	SLIST_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = SLIST_FIRST(head);				\
	    (var) && ((tvar) = SLIST_NEXT(var, field), 1);		\
	    (var) = (tvar))

/*
 * Singly-linked List functions.
 */
#define	SLIST_INIT(head) {						\
	SLIST_FIRST(head) = SLIST_END(head);				\
}

#define	SLIST_INSERT_AFTER(slistelm, elm, field) do {			\
	(elm)->field.sle_next = (slistelm)->field.sle_next;		\
	(slistelm)->field.sle_next = (elm);				\
} while (0)

#define	SLIST_INSERT_HEAD(head, elm, field) do {			\
	(elm)->field.sle_next = (head)->slh_first;			\
	(head)->slh_first = (elm);					\
} while (0)

#define	SLIST_REMOVE_AFTER(elm, field) do {				\
	(elm)->field.sle_next = (elm)->field.sle_next->field.sle_next;	\
} while (0)

#define	SLIST_REMOVE_HEAD(head, field) do {				\
	(head)->slh_first = (head)->slh_first->field.sle_next;		\
} while (0)

#define SLIST_REMOVE(head, elm, type, field) do {			\
	if ((head)->slh_first == (elm)) {				\
		SLIST_REMOVE_HEAD((head), field);			\
	} else {							\
		struct type *curelm = (head)->slh_first;		\
									\
		while (curelm->field.sle_next != (elm))			\
			curelm = curelm->field.sle_next;		\
		curelm->field.sle_next =				\
		    curelm->field.sle_next->field.sle_next;		\
		_Q_INVALIDATE((elm)->field.sle_next);			\
	}								\
} while (0)

/*
 * List definitions.
 */
#define LIST_HEAD(name, type)						\
struct name {								\
	struct type *lh_first;	/* first element */			\
}

#define LIST_HEAD_INITIALIZER(head)					\
	{ NULL }

#define LIST_ENTRY(type)						\
struct {								\
	struct type *le_next;	/* next element */			\
	struct type **le_prev;	/* address of previous next element */	\
}

/*
 * List access methods
 */
#define	LIST_FIRST(head)		((head)->lh_first)
#define	LIST_END(head)			NULL
#define	LIST_EMPTY(head)		(LIST_FIRST(head) == LIST_END(head))
#define	LIST_NEXT(elm, field)		((elm)->field.le_next)

#define LIST_FOREACH(var, head, field)					\
	for((var) = LIST_FIRST(head);					\
	    (var)!= LIST_END(head);					\
	    (var) = LIST_NEXT(var, field))

#define	LIST_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = LIST_FIRST(head);				\
	    (var) && ((tvar) = LIST_NEXT(var, field), 1);		\
	    (var) = (tvar))

/*
 * List functions.
 */
#define	LIST_INIT(head) do {						\
	LIST_FIRST(head) = LIST_END(head);				\
} while (0)

#define LIST_INSERT_AFTER(listelm, elm, field) do {			\
	if (((elm)->field.le_next = (listelm)->field.le_next) != NULL)	\
		(listelm)->field.le_next->field.le_prev =		\
		    &(elm)->field.le_next;				\
	(listelm)->field.le_next = (elm);				\
	(elm)->field.le_prev = &(listelm)->field.le_next;		\
} while (0)

#define	LIST_INSERT_BEFORE(listelm, elm, field) do {			\
	(elm)->field.le_prev = (listelm)->field.le_prev;		\
	(elm)->field.le_next = (listelm);				\
	*(listelm)->field.le_prev = (elm);				\
	(listelm)->field.le_prev = &(elm)->field.le_next;		\
} while (0)

#define LIST_INSERT_HEAD(head, elm, field) do {				\
	if (((elm)->field.le_next = (head)->lh_first) != NULL)		\
		(head)->lh_first->field.le_prev = &(elm)->field.le_next;\
	(head)->lh_first = (elm);					\
	(elm)->field.le_prev = &(head)->lh_first;			\
} while (0)

#define LIST_REMOVE(elm, field) do {					\
	if ((elm)->field.le_next != NULL)				\
		(elm)->field.le_next->field.le_prev =			\
		    (elm)->field.le_prev;				\
	*(elm)->field.le_prev = (elm)->field.le_next;			\
	_Q_INVALIDATE((elm)->field.le_prev);				\
	_Q_INVALIDATE((elm)->field.le_next);				\
} while (0)

#define LIST_REPLACE(elm, elm2, field) do {				\
	if (((elm2)->field.le_next = (elm)->field.le_next) != NULL)	\
		(elm2)->field.le_next->field.le_prev =			\
		    &(elm2)->field.le_next;				\
	(elm2)->field.le_prev = (elm)->field.le_prev;			\
	*(elm2)->field.le_prev = (elm2);				\
	_Q_INVALIDATE((elm)->field.le_prev);				\
	_Q_INVALIDATE((elm)->field.le_next);				\
} while (0)

/*
 * Simple queue definitions.
 */
#define SIMPLEQ_HEAD(name, type)					\
struct name {								\
	struct type *sqh_first;	/* first element */			\
	struct type **sqh_last;	/* addr of last next element */		\
}

#define SIMPLEQ_HEAD_INITIALIZER(head)					\
	{ NULL, &(head).sqh_first }

#define SIMPLEQ_ENTRY(type)						\
struct {								\
	struct type *sqe_next;	/* next element */			\
}

/*
 * Simple queue access methods.
 */
#define	SIMPLEQ_FIRST(head)	    ((head)->sqh_first)
#define	SIMPLEQ_END(head)	    NULL
#define	SIMPLEQ_EMPTY(head)	    (SIMPLEQ_FIRST(head) == SIMPLEQ_END(head))
#define	SIMPLEQ_NEXT(elm, field)    ((elm)->field.sqe_next)

#define SIMPLEQ_FOREACH(var, head, field)				\
	for((var) = SIMPLEQ_FIRST(head);				\
	    (var) != SIMPLEQ_END(head);					\
	    (var) = SIMPLEQ_NEXT(var, field))

#define	SIMPLEQ_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = SIMPLEQ_FIRST(head);				\
	    (var) && ((tvar) = SIMPLEQ_NEXT(var, field), 1);		\
	    (var) = (tvar))

/*
 * Simple queue functions.
 */
#define	SIMPLEQ_INIT(head) do {						\
	(head)->sqh_first = NULL;					\
	(head)->sqh_last = &(head)->sqh_first;				\
} while (0)

#define SIMPLEQ_INSERT_HEAD(head, elm, field) do {			\
	if (((elm)->field.sqe_next = (head)->sqh_first) == NULL)	\
		(head)->sqh_last = &(elm)->field.sqe_next;		\
	(head)->sqh_first = (elm);					\
} while (0)

#define SIMPLEQ_INSERT_TAIL(head, elm, field) do {			\
	(elm)->field.sqe_next = NULL;					\
	*(head)->sqh_last = (elm);					\
	(head)->sqh_last = &(elm)->field.sqe_next;			\
} while (0)

#define SIMPLEQ_INSERT_AFTER(head, listelm, elm, field) do {		\
	if (((elm)->field.sqe_next = (listelm)->field.sqe_next) == NULL)\
		(head)->sqh_last = &(elm)->field.sqe_next;		\
	(listelm)->field.sqe_next = (elm);				\
} while (0)

#define SIMPLEQ_REMOVE_HEAD(head, field) do {			\
	if (((head)->sqh_first = (head)->sqh_first->field.sqe_next) == NULL) \
		(head)->sqh_last = &(head)->sqh_first;			\
} while (0)

#define SIMPLEQ_REMOVE_AFTER(head, elm, field) do {			\
	if (((elm)->field.sqe_next = (elm)->field.sqe_next->field.sqe_next) \
	    == NULL)							\
		(head)->sqh_last = &(elm)->field.sqe_next;		\
} while (0)

/*
 * XOR Simple queue definitions.
 */
#define XSIMPLEQ_HEAD(name, type)					\
struct name {								\
	struct type *sqx_first;	/* first element */			\
	struct type **sqx_last;	/* addr of last next element */		\
	unsigned long sqx_cookie;					\
}

#define XSIMPLEQ_ENTRY(type)						\
struct {								\
	struct type *sqx_next;	/* next element */			\
}

/*
 * XOR Simple queue access methods.
 */
#define XSIMPLEQ_XOR(head, ptr)	    ((__typeof(ptr))((head)->sqx_cookie ^ \
					(unsigned long)(ptr)))
#define	XSIMPLEQ_FIRST(head)	    XSIMPLEQ_XOR(head, ((head)->sqx_first))
#define	XSIMPLEQ_END(head)	    NULL
#define	XSIMPLEQ_EMPTY(head)	    (XSIMPLEQ_FIRST(head) == XSIMPLEQ_END(head))
#define	XSIMPLEQ_NEXT(head, elm, field)    XSIMPLEQ_XOR(head, ((elm)->field.sqx_next))


#define XSIMPLEQ_FOREACH(var, head, field)				\
	for ((var) = XSIMPLEQ_FIRST(head);				\
	    (var) != XSIMPLEQ_END(head);				\
	    (var) = XSIMPLEQ_NEXT(head, var, field))

#define	XSIMPLEQ_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = XSIMPLEQ_FIRST(head);				\
	    (var) && ((tvar) = XSIMPLEQ_NEXT(head, var, field), 1);	\
	    (var) = (tvar))

/*
 * XOR Simple queue functions.
 */
#define	XSIMPLEQ_INIT(head) do {					\
	arc4random_buf(&(head)->sqx_cookie, sizeof((head)->sqx_cookie)); \
	(head)->sqx_first = XSIMPLEQ_XOR(head, NULL);			\
	(head)->sqx_last = XSIMPLEQ_XOR(head, &(head)->sqx_first);	\
} while (0)

#define XSIMPLEQ_INSERT_HEAD(head, elm, field) do {			\
	if (((elm)->field.sqx_next = (head)->sqx_first) ==		\
	    XSIMPLEQ_XOR(head, NULL))					\
		(head)->sqx_last = XSIMPLEQ_XOR(head, &(elm)->field.sqx_next); \
	(head)->sqx_first = XSIMPLEQ_XOR(head, (elm));			\
} while (0)

#define XSIMPLEQ_INSERT_TAIL(head, elm, field) do {			\
	(elm)->field.sqx_next = XSIMPLEQ_XOR(head, NULL);		\
	*(XSIMPLEQ_XOR(head, (head)->sqx_last)) = XSIMPLEQ_XOR(head, (elm)); \
	(head)->sqx_last = XSIMPLEQ_XOR(head, &(elm)->field.sqx_next);	\
} while (0)

#define XSIMPLEQ_INSERT_AFTER(head, listelm, elm, field) do {		\
	if (((elm)->field.sqx_next = (listelm)->field.sqx_next) ==	\
	    XSIMPLEQ_XOR(head, NULL))					\
		(head)->sqx_last = XSIMPLEQ_XOR(head, &(elm)->field.sqx_next); \
	(listelm)->field.sqx_next = XSIMPLEQ_XOR(head, (elm));		\
} while (0)

#define XSIMPLEQ_REMOVE_HEAD(head, field) do {				\
	if (((head)->sqx_first = XSIMPLEQ_XOR(head,			\
	    (head)->sqx_first)->field.sqx_next) == XSIMPLEQ_XOR(head, NULL)) \
		(head)->sqx_last = XSIMPLEQ_XOR(head, &(head)->sqx_first); \
} while (0)

#define XSIMPLEQ_REMOVE_AFTER(head, elm, field) do {			\
	if (((elm)->field.sqx_next = XSIMPLEQ_XOR(head,			\
	    (elm)->field.sqx_next)->field.sqx_next)			\
	    == XSIMPLEQ_XOR(head, NULL))				\
		(head)->sqx_last = 					\
		    XSIMPLEQ_XOR(head, &(elm)->field.sqx_next);		\
} while (0)

		    
/*
 * Tail queue definitions.
 */
#define TAILQ_HEAD(name, type)						\
struct name {								\
	struct type *tqh_first;	/* first element */			\
	struct type **tqh_last;	/* addr of last next element */		\
}

#define TAILQ_HEAD_INITIALIZER(head)					\
	{ NULL, &(head).tqh_first }

#define TAILQ_ENTRY(type)						\
struct {								\
	struct type *tqe_next;	/* next element */			\
	struct type **tqe_prev;	/* address of previous next element */	\
}

/* 
 * tail queue access methods 
 */
#define	TAILQ_FIRST(head)		((head)->tqh_first)
#define	TAILQ_END(head)			NULL
#define	TAILQ_NEXT(elm, field)		((elm)->field.tqe_next)
#define TAILQ_LAST(head, headname)					\
	(*(((struct headname *)((head)->tqh_last))->tqh_last))
/* XXX */
#define TAILQ_PREV(elm, headname, field)				\
	(*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))
#define	TAILQ_EMPTY(head)						\
	(TAILQ_FIRST(head) == TAILQ_END(head))

#define TAILQ_FOREACH(var, head, field)					\
	for((var) = TAILQ_FIRST(head);					\
	    (var) != TAILQ_END(head);					\
	    (var) = TAILQ_NEXT(var, field))

#define	TAILQ_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = TAILQ_FIRST(head);					\
	    (var) != TAILQ_END(head) &&					\
	    ((tvar) = TAILQ_NEXT(var, field), 1);			\
	    (var) = (tvar))


#define TAILQ_FOREACH_REVERSE(var, head, headname, field)		\
	for((var) = TAILQ_LAST(head, headname);				\
	    (var) != TAILQ_END(head);					\
	    (var) = TAILQ_PREV(var, headname, field))

#define	TAILQ_FOREACH_REVERSE_SAFE(var, head, headname, field, tvar)	\
	for ((var) = TAILQ_LAST(head, headname);			\
	    (var) != TAILQ_END(head) &&					\
	    ((tvar) = TAILQ_PREV(var, headname, field), 1);		\
	    (var) = (tvar))

/*
 * Tail queue functions.
 */
#define	TAILQ_INIT(head) do {						\
	(head)->tqh_first = NULL;					\
	(head)->tqh_last = &(head)->tqh_first;				\
} while (0)

#define TAILQ_INSERT_HEAD(head, elm, field) do {			\
	if (((elm)->field.tqe_next = (head)->tqh_first) != NULL)	\
		(head)->tqh_first->field.tqe_prev =			\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(head)->tqh_first = (elm);					\
	(elm)->field.tqe_prev = &(head)->tqh_first;			\
} while (0)

#define TAILQ_INSERT_TAIL(head, elm, field) do {			\
	(elm)->field.tqe_next = NULL;					\
	(elm)->field.tqe_prev = (head)->tqh_last;			\
	*(head)->tqh_last = (elm);					\
	(head)->tqh_last = &(elm)->field.tqe_next;			\
} while (0)

#define TAILQ_INSERT_AFTER(head, listelm, elm, field) do {		\
	if (((elm)->field.tqe_next = (listelm)->field.tqe_next) != NULL)\
		(elm)->field.tqe_next->field.tqe_prev =			\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(listelm)->field.tqe_next = (elm);				\
	(elm)->field.tqe_prev = &(listelm)->field.tqe_next;		\
} while (0)

#define	TAILQ_INSERT_BEFORE(listelm, elm, field) do {			\
	(elm)->field.tqe_prev = (listelm)->field.tqe_prev;		\
	(elm)->field.tqe_next = (listelm);				\
	*(listelm)->field.tqe_prev = (elm);				\
	(listelm)->field.tqe_prev = &(elm)->field.tqe_next;		\
} while (0)

#define TAILQ_REMOVE(head, elm, field) do {				\
	if (((elm)->field.tqe_next) != NULL)				\
		(elm)->field.tqe_next->field.tqe_prev =			\
		    (elm)->field.tqe_prev;				\
	else								\
		(head)->tqh_last = (elm)->field.tqe_prev;		\
	*(elm)->field.tqe_prev = (elm)->field.tqe_next;			\
	_Q_INVALIDATE((elm)->field.tqe_prev);				\
	_Q_INVALIDATE((elm)->field.tqe_next);				\
} while (0)

#define TAILQ_REPLACE(head, elm, elm2, field) do {			\
	if (((elm2)->field.tqe_next = (elm)->field.tqe_next) != NULL)	\
		(elm2)->field.tqe_next->field.tqe_prev =		\
		    &(elm2)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm2)->field.tqe_next;		\
	(elm2)->field.tqe_prev = (elm)->field.tqe_prev;			\
	*(elm2)->field.tqe_prev = (elm2);				\
	_Q_INVALIDATE((elm)->field.tqe_prev);				\
	_Q_INVALIDATE((elm)->field.tqe_next);				\
} while (0)

/*
 * Circular queue definitions.
 */
#define CIRCLEQ_HEAD(name, type)					\
struct name {								\
	struct type *cqh_first;		/* first element */		\
	struct type *cqh_last;		/* last element */		\
}

#define CIRCLEQ_HEAD_INITIALIZER(head)					\
	{ CIRCLEQ_END(&head), CIRCLEQ_END(&head) }

#define CIRCLEQ_ENTRY(type)						\
struct {								\
	struct type *cqe_next;		/* next element */		\
	struct type *cqe_prev;		/* previous element */		\
}

/*
 * Circular queue access methods 
 */
#define	CIRCLEQ_FIRST(head)		((head)->cqh_first)
#define	CIRCLEQ_LAST(head)		((head)->cqh_last)
#define	CIRCLEQ_END(head)		((void *)(head))
#define	CIRCLEQ_NEXT(elm, field)	((elm)->field.cqe_next)
#define	CIRCLEQ_PREV(elm, field)	((elm)->field.cqe_prev)
#define	CIRCLEQ_EMPTY(head)						\
	(CIRCLEQ_FIRST(head) == CIRCLEQ_END(head))

#define CIRCLEQ_FOREACH(var, head, field)				\
	for((var) = CIRCLEQ_FIRST(head);				\
	    (var) != CIRCLEQ_END(head);					\
	    (var) = CIRCLEQ_NEXT(var, field))

#define	CIRCLEQ_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = CIRCLEQ_FIRST(head);				\
	    (var) != CIRCLEQ_END(head) &&				\
	    ((tvar) = CIRCLEQ_NEXT(var, field), 1);			\
	    (var) = (tvar))

#define CIRCLEQ_FOREACH_REVERSE(var, head, field)			\
	for((var) = CIRCLEQ_LAST(head);					\
	    (var) != CIRCLEQ_END(head);					\
	    (var) = CIRCLEQ_PREV(var, field))

#define	CIRCLEQ_FOREACH_REVERSE_SAFE(var, head, headname, field, tvar)	\
	for ((var) = CIRCLEQ_LAST(head, headname);			\
	    (var) != CIRCLEQ_END(head) && 				\
	    ((tvar) = CIRCLEQ_PREV(var, headname, field), 1);		\
	    (var) = (tvar))

/*
 * Circular queue functions.
 */
#define	CIRCLEQ_INIT(head) do {						\
	(head)->cqh_first = CIRCLEQ_END(head);				\
	(head)->cqh_last = CIRCLEQ_END(head);				\
} while (0)

#define CIRCLEQ_INSERT_AFTER(head, listelm, elm, field) do {		\
	(elm)->field.cqe_next = (listelm)->field.cqe_next;		\
	(elm)->field.cqe_prev = (listelm);				\
	if ((listelm)->field.cqe_next == CIRCLEQ_END(head))		\
		(head)->cqh_last = (elm);				\
	else								\
		(listelm)->field.cqe_next->field.cqe_prev = (elm);	\
	(listelm)->field.cqe_next = (elm);				\
} while (0)

#define CIRCLEQ_INSERT_BEFORE(head, listelm, elm, field) do {		\
	(elm)->field.cqe_next = (listelm);				\
	(elm)->field.cqe_prev = (listelm)->field.cqe_prev;		\
	if ((listelm)->field.cqe_prev == CIRCLEQ_END(head))		\
		(head)->cqh_first = (elm);				\
	else								\
		(listelm)->field.cqe_prev->field.cqe_next = (elm);	\
	(listelm)->field.cqe_prev = (elm);				\
} while (0)

#define CIRCLEQ_INSERT_HEAD(head, elm, field) do {			\
	(elm)->field.cqe_next = (head)->cqh_first;			\
	(elm)->field.cqe_prev = CIRCLEQ_END(head);			\
	if ((head)->cqh_last == CIRCLEQ_END(head))			\
		(head)->cqh_last = (elm);				\
	else								\
		(head)->cqh_first->field.cqe_prev = (elm);		\
	(head)->cqh_first = (elm);					\
} while (0)

#define CIRCLEQ_INSERT_TAIL(head, elm, field) do {			\
	(elm)->field.cqe_next = CIRCLEQ_END(head);			\
	(elm)->field.cqe_prev = (head)->cqh_last;			\
	if ((head)->cqh_first == CIRCLEQ_END(head))			\
		(head)->cqh_first = (elm);				\
	else								\
		(head)->cqh_last->field.cqe_next = (elm);		\
	(head)->cqh_last = (elm);					\
} while (0)

#define	CIRCLEQ_REMOVE(head, elm, field) do {				\
	if ((elm)->field.cqe_next == CIRCLEQ_END(head))			\
		(head)->cqh_last = (elm)->field.cqe_prev;		\
	else								\
		(elm)->field.cqe_next->field.cqe_prev =			\
		    (elm)->field.cqe_prev;				\
	if ((elm)->field.cqe_prev == CIRCLEQ_END(head))			\
		(head)->cqh_first = (elm)->field.cqe_next;		\
	else								\
		(elm)->field.cqe_prev->field.cqe_next =			\
		    (elm)->field.cqe_next;				\
	_Q_INVALIDATE((elm)->field.cqe_prev);				\
	_Q_INVALIDATE((elm)->field.cqe_next);				\
} while (0)

#define CIRCLEQ_REPLACE(head, elm, elm2, field) do {			\
	if (((elm2)->field.cqe_next = (elm)->field.cqe_next) ==		\
	    CIRCLEQ_END(head))						\
		(head)->cqh_last = (elm2);				\
	else								\
		(elm2)->field.cqe_next->field.cqe_prev = (elm2);	\
	if (((elm2)->field.cqe_prev = (elm)->field.cqe_prev) ==		\
	    CIRCLEQ_END(head))						\
		(head)->cqh_first = (elm2);				\
	else								\
		(elm2)->field.cqe_prev->field.cqe_next = (elm2);	\
	_Q_INVALIDATE((elm)->field.cqe_prev);				\
	_Q_INVALIDATE((elm)->field.cqe_next);				\
} while (0)

#endif	/* !_SYS_QUEUE_H_ */

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/deps/pcg_basic.h" ***********************************/

/*
 * PCG Random Number Generation for C.
 *
 * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *     http://www.pcg-random.org
 */

/*
 * This code is derived from the full C implementation, which is in turn
 * derived from the canonical C++ PCG implementation. The C++ version
 * has many additional features and is preferable if you can use C++ in
 * your project.
 */


#include <inttypes.h>

#if __cplusplus
extern "C" {
#endif

struct pcg_state_setseq_64 {    // Internals are *Private*.
    uint64_t state;             // RNG state.  All values are possible.
    uint64_t inc;               // Controls which RNG sequence (stream) is
                                // selected. Must *always* be odd.
};
typedef struct pcg_state_setseq_64 pcg32_random_t;

// If you *must* statically initialize it, here's one.

#define PCG32_INITIALIZER   { 0x853c49e6748fea9bULL, 0xda3e39cb94b95bdbULL }

// pcg32_srandom(initial_state, initseq)
// pcg32_srandom_r(rng, initial_state, initseq):
//     Seed the rng.  Specified in two parts, state initializer and a
//     sequence selection constant (a.k.a. stream id)

void pcg32_srandom(uint64_t initial_state, uint64_t initseq);
void pcg32_srandom_r(pcg32_random_t* rng, uint64_t initial_state,
                     uint64_t initseq);

// pcg32_random()
// pcg32_random_r(rng)
//     Generate a uniformly distributed 32-bit random number

uint32_t pcg32_random(void);
uint32_t pcg32_random_r(pcg32_random_t* rng);

// pcg32_boundedrand(bound):
// pcg32_boundedrand_r(rng, bound):
//     Generate a uniformly distributed number, r, where 0 <= r < bound

uint32_t pcg32_boundedrand(uint32_t bound);
uint32_t pcg32_boundedrand_r(pcg32_random_t* rng, uint32_t bound);

#if __cplusplus
}
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/deps/libc_time.h" ***********************************/


#include <limits.h>
#include <time.h>
int __secs_to_tm(long long t, struct tm *tm);


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/ua_util.h" ***********************************/



#include <assert.h>
#define UA_assert(ignore) assert(ignore)

/*********************/
/* Memory Management */
/*********************/

/* Replace the macros with functions for custom allocators if necessary */
#include <stdlib.h> // malloc, free
#ifdef _WIN32
# include <malloc.h>
#endif

#ifndef UA_free
# define UA_free(ptr) free(ptr)
#endif
#ifndef UA_malloc
# define UA_malloc(size) malloc(size)
#endif
#ifndef UA_calloc
# define UA_calloc(num, size) calloc(num, size)
#endif
#ifndef UA_realloc
# define UA_realloc(ptr, size) realloc(ptr, size)
#endif

#ifndef NO_ALLOCA
# ifdef __GNUC__
#  define UA_alloca(size) __builtin_alloca (size)
# elif defined(_WIN32)
#  define UA_alloca(SIZE) _alloca(SIZE)
# else
#  include <alloca.h>
#  define UA_alloca(SIZE) alloca(SIZE)
# endif
#endif

#define container_of(ptr, type, member) \
    (type *)((uintptr_t)ptr - offsetof(type,member))

/************************/
/* Thread Local Storage */
/************************/

#ifdef UA_ENABLE_MULTITHREADING
# ifdef __GNUC__
#  define UA_THREAD_LOCAL __thread
# elif defined(_MSC_VER)
#  define UA_THREAD_LOCAL __declspec(thread)
# else
#  error No thread local storage keyword defined for this compiler
# endif
#else
# define UA_THREAD_LOCAL
#endif

/********************/
/* System Libraries */
/********************/

#ifdef _WIN32
# include <winsock2.h> //needed for amalgamation
# include <windows.h>
# undef SLIST_ENTRY
#endif

#include <time.h>
#if defined(_WIN32) && !defined(__MINGW32__)
int gettimeofday(struct timeval *tp, struct timezone *tzp);
#else
# include <sys/time.h>
#endif

#if defined(__APPLE__) || defined(__MACH__)
#include <mach/clock.h>
#include <mach/mach.h>
#endif

/*************************/
/* External Dependencies */
/*************************/


#ifdef UA_ENABLE_MULTITHREADING
# define _LGPL_SOURCE
# include <urcu.h>
# include <urcu/wfcqueue.h>
# include <urcu/uatomic.h>
# include <urcu/rculfhash.h>
# include <urcu/lfstack.h>
# ifdef NDEBUG
#  define UA_RCU_LOCK() rcu_read_lock()
#  define UA_RCU_UNLOCK() rcu_read_unlock()
#  define UA_ASSERT_RCU_LOCKED()
#  define UA_ASSERT_RCU_UNLOCKED()
# else
   extern UA_THREAD_LOCAL bool rcu_locked;
#   define UA_ASSERT_RCU_LOCKED() assert(rcu_locked)
#   define UA_ASSERT_RCU_UNLOCKED() assert(!rcu_locked)
#   define UA_RCU_LOCK() do {                     \
        UA_ASSERT_RCU_UNLOCKED();                 \
        rcu_locked = true;                        \
        rcu_read_lock(); } while(0)
#   define UA_RCU_UNLOCK() do {                   \
        UA_ASSERT_RCU_LOCKED();                   \
        rcu_locked = false;                       \
        rcu_read_lock(); } while(0)
# endif
#else
# define UA_RCU_LOCK()
# define UA_RCU_UNLOCK()
# define UA_ASSERT_RCU_LOCKED()
# define UA_ASSERT_RCU_UNLOCKED()
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/ua_types_encoding_binary.h" ***********************************/



typedef UA_StatusCode (*UA_exchangeEncodeBuffer)(void *handle, UA_ByteString *buf, size_t offset);

UA_StatusCode
UA_encodeBinary(const void *src, const UA_DataType *type,
                UA_exchangeEncodeBuffer exchangeBufferCallback, void *exchangeBufferCallbackHandle,
                UA_ByteString *dst, size_t *offset) UA_FUNC_ATTR_WARN_UNUSED_RESULT;

UA_StatusCode
UA_decodeBinary(const UA_ByteString *src, size_t *offset, void *dst,
                const UA_DataType *type) UA_FUNC_ATTR_WARN_UNUSED_RESULT;

size_t UA_calcSizeBinary(void *p, const UA_DataType *type);


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/build/src_generated/ua_types_generated_encoding_binary.h" ***********************************/

/* Generated from Opc.Ua.Types.bsd with script /home/travis/build/open62541/open62541/tools/generate_datatypes.py
 * on host testing-worker-linux-docker-8551a97d-3382-linux-4 by user travis at 2016-05-18 08:48:03 */
 

/* Boolean */
static UA_INLINE UA_StatusCode UA_Boolean_encodeBinary(const UA_Boolean *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BOOLEAN], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_Boolean_decodeBinary(const UA_ByteString *src, size_t *offset, UA_Boolean *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BOOLEAN]); }

/* SByte */
static UA_INLINE UA_StatusCode UA_SByte_encodeBinary(const UA_SByte *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_SBYTE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_SByte_decodeBinary(const UA_ByteString *src, size_t *offset, UA_SByte *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_SBYTE]); }

/* Byte */
static UA_INLINE UA_StatusCode UA_Byte_encodeBinary(const UA_Byte *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BYTE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_Byte_decodeBinary(const UA_ByteString *src, size_t *offset, UA_Byte *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BYTE]); }

/* Int16 */
static UA_INLINE UA_StatusCode UA_Int16_encodeBinary(const UA_Int16 *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_INT16], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_Int16_decodeBinary(const UA_ByteString *src, size_t *offset, UA_Int16 *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_INT16]); }

/* UInt16 */
static UA_INLINE UA_StatusCode UA_UInt16_encodeBinary(const UA_UInt16 *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_UINT16], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_UInt16_decodeBinary(const UA_ByteString *src, size_t *offset, UA_UInt16 *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_UINT16]); }

/* Int32 */
static UA_INLINE UA_StatusCode UA_Int32_encodeBinary(const UA_Int32 *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_INT32], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_Int32_decodeBinary(const UA_ByteString *src, size_t *offset, UA_Int32 *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_INT32]); }

/* UInt32 */
static UA_INLINE UA_StatusCode UA_UInt32_encodeBinary(const UA_UInt32 *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_UINT32], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_UInt32_decodeBinary(const UA_ByteString *src, size_t *offset, UA_UInt32 *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_UINT32]); }

/* Int64 */
static UA_INLINE UA_StatusCode UA_Int64_encodeBinary(const UA_Int64 *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_INT64], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_Int64_decodeBinary(const UA_ByteString *src, size_t *offset, UA_Int64 *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_INT64]); }

/* UInt64 */
static UA_INLINE UA_StatusCode UA_UInt64_encodeBinary(const UA_UInt64 *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_UINT64], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_UInt64_decodeBinary(const UA_ByteString *src, size_t *offset, UA_UInt64 *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_UINT64]); }

/* Float */
static UA_INLINE UA_StatusCode UA_Float_encodeBinary(const UA_Float *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_FLOAT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_Float_decodeBinary(const UA_ByteString *src, size_t *offset, UA_Float *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_FLOAT]); }

/* Double */
static UA_INLINE UA_StatusCode UA_Double_encodeBinary(const UA_Double *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DOUBLE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_Double_decodeBinary(const UA_ByteString *src, size_t *offset, UA_Double *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DOUBLE]); }

/* String */
static UA_INLINE UA_StatusCode UA_String_encodeBinary(const UA_String *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_STRING], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_String_decodeBinary(const UA_ByteString *src, size_t *offset, UA_String *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_STRING]); }

/* DateTime */
static UA_INLINE UA_StatusCode UA_DateTime_encodeBinary(const UA_DateTime *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DATETIME], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DateTime_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DateTime *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DATETIME]); }

/* Guid */
static UA_INLINE UA_StatusCode UA_Guid_encodeBinary(const UA_Guid *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_GUID], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_Guid_decodeBinary(const UA_ByteString *src, size_t *offset, UA_Guid *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_GUID]); }

/* ByteString */
static UA_INLINE UA_StatusCode UA_ByteString_encodeBinary(const UA_ByteString *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BYTESTRING], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ByteString_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ByteString *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BYTESTRING]); }

/* XmlElement */
static UA_INLINE UA_StatusCode UA_XmlElement_encodeBinary(const UA_XmlElement *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_XMLELEMENT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_XmlElement_decodeBinary(const UA_ByteString *src, size_t *offset, UA_XmlElement *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_XMLELEMENT]); }

/* NodeId */
static UA_INLINE UA_StatusCode UA_NodeId_encodeBinary(const UA_NodeId *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_NODEID], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_NodeId_decodeBinary(const UA_ByteString *src, size_t *offset, UA_NodeId *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_NODEID]); }

/* ExpandedNodeId */
static UA_INLINE UA_StatusCode UA_ExpandedNodeId_encodeBinary(const UA_ExpandedNodeId *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_EXPANDEDNODEID], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ExpandedNodeId_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ExpandedNodeId *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_EXPANDEDNODEID]); }

/* StatusCode */
static UA_INLINE UA_StatusCode UA_StatusCode_encodeBinary(const UA_StatusCode *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_STATUSCODE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_StatusCode_decodeBinary(const UA_ByteString *src, size_t *offset, UA_StatusCode *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_STATUSCODE]); }

/* QualifiedName */
static UA_INLINE UA_StatusCode UA_QualifiedName_encodeBinary(const UA_QualifiedName *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_QUALIFIEDNAME], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_QualifiedName_decodeBinary(const UA_ByteString *src, size_t *offset, UA_QualifiedName *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]); }

/* LocalizedText */
static UA_INLINE UA_StatusCode UA_LocalizedText_encodeBinary(const UA_LocalizedText *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_LocalizedText_decodeBinary(const UA_ByteString *src, size_t *offset, UA_LocalizedText *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]); }

/* ExtensionObject */
static UA_INLINE UA_StatusCode UA_ExtensionObject_encodeBinary(const UA_ExtensionObject *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ExtensionObject_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ExtensionObject *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_EXTENSIONOBJECT]); }

/* DataValue */
static UA_INLINE UA_StatusCode UA_DataValue_encodeBinary(const UA_DataValue *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DATAVALUE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DataValue_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DataValue *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DATAVALUE]); }

/* Variant */
static UA_INLINE UA_StatusCode UA_Variant_encodeBinary(const UA_Variant *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_VARIANT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_Variant_decodeBinary(const UA_ByteString *src, size_t *offset, UA_Variant *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_VARIANT]); }

/* DiagnosticInfo */
static UA_INLINE UA_StatusCode UA_DiagnosticInfo_encodeBinary(const UA_DiagnosticInfo *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DIAGNOSTICINFO], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DiagnosticInfo_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DiagnosticInfo *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DIAGNOSTICINFO]); }

/* SignedSoftwareCertificate */
static UA_INLINE UA_StatusCode UA_SignedSoftwareCertificate_encodeBinary(const UA_SignedSoftwareCertificate *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_SIGNEDSOFTWARECERTIFICATE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_SignedSoftwareCertificate_decodeBinary(const UA_ByteString *src, size_t *offset, UA_SignedSoftwareCertificate *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_SIGNEDSOFTWARECERTIFICATE]); }

/* BrowsePathTarget */
static UA_INLINE UA_StatusCode UA_BrowsePathTarget_encodeBinary(const UA_BrowsePathTarget *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BROWSEPATHTARGET], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_BrowsePathTarget_decodeBinary(const UA_ByteString *src, size_t *offset, UA_BrowsePathTarget *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BROWSEPATHTARGET]); }

/* ViewAttributes */
static UA_INLINE UA_StatusCode UA_ViewAttributes_encodeBinary(const UA_ViewAttributes *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_VIEWATTRIBUTES], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ViewAttributes_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ViewAttributes *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_VIEWATTRIBUTES]); }

/* BrowseResultMask */
static UA_INLINE UA_StatusCode UA_BrowseResultMask_encodeBinary(const UA_BrowseResultMask *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BROWSERESULTMASK], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_BrowseResultMask_decodeBinary(const UA_ByteString *src, size_t *offset, UA_BrowseResultMask *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BROWSERESULTMASK]); }

/* RequestHeader */
static UA_INLINE UA_StatusCode UA_RequestHeader_encodeBinary(const UA_RequestHeader *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_REQUESTHEADER], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_RequestHeader_decodeBinary(const UA_ByteString *src, size_t *offset, UA_RequestHeader *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_REQUESTHEADER]); }

/* MonitoredItemModifyResult */
static UA_INLINE UA_StatusCode UA_MonitoredItemModifyResult_encodeBinary(const UA_MonitoredItemModifyResult *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYRESULT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_MonitoredItemModifyResult_decodeBinary(const UA_ByteString *src, size_t *offset, UA_MonitoredItemModifyResult *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYRESULT]); }

/* ViewDescription */
static UA_INLINE UA_StatusCode UA_ViewDescription_encodeBinary(const UA_ViewDescription *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_VIEWDESCRIPTION], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ViewDescription_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ViewDescription *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_VIEWDESCRIPTION]); }

/* CloseSecureChannelRequest */
static UA_INLINE UA_StatusCode UA_CloseSecureChannelRequest_encodeBinary(const UA_CloseSecureChannelRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_CloseSecureChannelRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_CloseSecureChannelRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELREQUEST]); }

/* AddNodesResult */
static UA_INLINE UA_StatusCode UA_AddNodesResult_encodeBinary(const UA_AddNodesResult *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_ADDNODESRESULT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_AddNodesResult_decodeBinary(const UA_ByteString *src, size_t *offset, UA_AddNodesResult *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_ADDNODESRESULT]); }

/* VariableAttributes */
static UA_INLINE UA_StatusCode UA_VariableAttributes_encodeBinary(const UA_VariableAttributes *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_VariableAttributes_decodeBinary(const UA_ByteString *src, size_t *offset, UA_VariableAttributes *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES]); }

/* NotificationMessage */
static UA_INLINE UA_StatusCode UA_NotificationMessage_encodeBinary(const UA_NotificationMessage *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_NOTIFICATIONMESSAGE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_NotificationMessage_decodeBinary(const UA_ByteString *src, size_t *offset, UA_NotificationMessage *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_NOTIFICATIONMESSAGE]); }

/* NodeAttributesMask */
static UA_INLINE UA_StatusCode UA_NodeAttributesMask_encodeBinary(const UA_NodeAttributesMask *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_NODEATTRIBUTESMASK], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_NodeAttributesMask_decodeBinary(const UA_ByteString *src, size_t *offset, UA_NodeAttributesMask *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_NODEATTRIBUTESMASK]); }

/* MonitoringMode */
static UA_INLINE UA_StatusCode UA_MonitoringMode_encodeBinary(const UA_MonitoringMode *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_MONITORINGMODE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_MonitoringMode_decodeBinary(const UA_ByteString *src, size_t *offset, UA_MonitoringMode *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_MONITORINGMODE]); }

/* CallMethodResult */
static UA_INLINE UA_StatusCode UA_CallMethodResult_encodeBinary(const UA_CallMethodResult *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CALLMETHODRESULT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_CallMethodResult_decodeBinary(const UA_ByteString *src, size_t *offset, UA_CallMethodResult *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CALLMETHODRESULT]); }

/* ParsingResult */
static UA_INLINE UA_StatusCode UA_ParsingResult_encodeBinary(const UA_ParsingResult *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_PARSINGRESULT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ParsingResult_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ParsingResult *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_PARSINGRESULT]); }

/* RelativePathElement */
static UA_INLINE UA_StatusCode UA_RelativePathElement_encodeBinary(const UA_RelativePathElement *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_RELATIVEPATHELEMENT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_RelativePathElement_decodeBinary(const UA_ByteString *src, size_t *offset, UA_RelativePathElement *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_RELATIVEPATHELEMENT]); }

/* BrowseDirection */
static UA_INLINE UA_StatusCode UA_BrowseDirection_encodeBinary(const UA_BrowseDirection *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BROWSEDIRECTION], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_BrowseDirection_decodeBinary(const UA_ByteString *src, size_t *offset, UA_BrowseDirection *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BROWSEDIRECTION]); }

/* CallMethodRequest */
static UA_INLINE UA_StatusCode UA_CallMethodRequest_encodeBinary(const UA_CallMethodRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CALLMETHODREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_CallMethodRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_CallMethodRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CALLMETHODREQUEST]); }

/* ServerState */
static UA_INLINE UA_StatusCode UA_ServerState_encodeBinary(const UA_ServerState *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_SERVERSTATE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ServerState_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ServerState *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_SERVERSTATE]); }

/* UnregisterNodesRequest */
static UA_INLINE UA_StatusCode UA_UnregisterNodesRequest_encodeBinary(const UA_UnregisterNodesRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_UNREGISTERNODESREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_UnregisterNodesRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_UnregisterNodesRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_UNREGISTERNODESREQUEST]); }

/* ContentFilterElementResult */
static UA_INLINE UA_StatusCode UA_ContentFilterElementResult_encodeBinary(const UA_ContentFilterElementResult *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENTRESULT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ContentFilterElementResult_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ContentFilterElementResult *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENTRESULT]); }

/* QueryDataSet */
static UA_INLINE UA_StatusCode UA_QueryDataSet_encodeBinary(const UA_QueryDataSet *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_QUERYDATASET], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_QueryDataSet_decodeBinary(const UA_ByteString *src, size_t *offset, UA_QueryDataSet *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_QUERYDATASET]); }

/* SetPublishingModeRequest */
static UA_INLINE UA_StatusCode UA_SetPublishingModeRequest_encodeBinary(const UA_SetPublishingModeRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_SetPublishingModeRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_SetPublishingModeRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST]); }

/* TimestampsToReturn */
static UA_INLINE UA_StatusCode UA_TimestampsToReturn_encodeBinary(const UA_TimestampsToReturn *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_TIMESTAMPSTORETURN], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_TimestampsToReturn_decodeBinary(const UA_ByteString *src, size_t *offset, UA_TimestampsToReturn *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_TIMESTAMPSTORETURN]); }

/* CallRequest */
static UA_INLINE UA_StatusCode UA_CallRequest_encodeBinary(const UA_CallRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CALLREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_CallRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_CallRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CALLREQUEST]); }

/* MethodAttributes */
static UA_INLINE UA_StatusCode UA_MethodAttributes_encodeBinary(const UA_MethodAttributes *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_METHODATTRIBUTES], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_MethodAttributes_decodeBinary(const UA_ByteString *src, size_t *offset, UA_MethodAttributes *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_METHODATTRIBUTES]); }

/* DeleteReferencesItem */
static UA_INLINE UA_StatusCode UA_DeleteReferencesItem_encodeBinary(const UA_DeleteReferencesItem *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DELETEREFERENCESITEM], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DeleteReferencesItem_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DeleteReferencesItem *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DELETEREFERENCESITEM]); }

/* WriteValue */
static UA_INLINE UA_StatusCode UA_WriteValue_encodeBinary(const UA_WriteValue *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_WRITEVALUE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_WriteValue_decodeBinary(const UA_ByteString *src, size_t *offset, UA_WriteValue *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_WRITEVALUE]); }

/* MonitoredItemCreateResult */
static UA_INLINE UA_StatusCode UA_MonitoredItemCreateResult_encodeBinary(const UA_MonitoredItemCreateResult *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_MonitoredItemCreateResult_decodeBinary(const UA_ByteString *src, size_t *offset, UA_MonitoredItemCreateResult *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT]); }

/* MessageSecurityMode */
static UA_INLINE UA_StatusCode UA_MessageSecurityMode_encodeBinary(const UA_MessageSecurityMode *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_MESSAGESECURITYMODE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_MessageSecurityMode_decodeBinary(const UA_ByteString *src, size_t *offset, UA_MessageSecurityMode *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_MESSAGESECURITYMODE]); }

/* MonitoringParameters */
static UA_INLINE UA_StatusCode UA_MonitoringParameters_encodeBinary(const UA_MonitoringParameters *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_MONITORINGPARAMETERS], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_MonitoringParameters_decodeBinary(const UA_ByteString *src, size_t *offset, UA_MonitoringParameters *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_MONITORINGPARAMETERS]); }

/* SignatureData */
static UA_INLINE UA_StatusCode UA_SignatureData_encodeBinary(const UA_SignatureData *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_SIGNATUREDATA], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_SignatureData_decodeBinary(const UA_ByteString *src, size_t *offset, UA_SignatureData *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_SIGNATUREDATA]); }

/* ReferenceNode */
static UA_INLINE UA_StatusCode UA_ReferenceNode_encodeBinary(const UA_ReferenceNode *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_REFERENCENODE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ReferenceNode_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ReferenceNode *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_REFERENCENODE]); }

/* Argument */
static UA_INLINE UA_StatusCode UA_Argument_encodeBinary(const UA_Argument *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_ARGUMENT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_Argument_decodeBinary(const UA_ByteString *src, size_t *offset, UA_Argument *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_ARGUMENT]); }

/* UserIdentityToken */
static UA_INLINE UA_StatusCode UA_UserIdentityToken_encodeBinary(const UA_UserIdentityToken *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_USERIDENTITYTOKEN], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_UserIdentityToken_decodeBinary(const UA_ByteString *src, size_t *offset, UA_UserIdentityToken *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_USERIDENTITYTOKEN]); }

/* ObjectTypeAttributes */
static UA_INLINE UA_StatusCode UA_ObjectTypeAttributes_encodeBinary(const UA_ObjectTypeAttributes *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ObjectTypeAttributes_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ObjectTypeAttributes *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES]); }

/* SecurityTokenRequestType */
static UA_INLINE UA_StatusCode UA_SecurityTokenRequestType_encodeBinary(const UA_SecurityTokenRequestType *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_SECURITYTOKENREQUESTTYPE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_SecurityTokenRequestType_decodeBinary(const UA_ByteString *src, size_t *offset, UA_SecurityTokenRequestType *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_SECURITYTOKENREQUESTTYPE]); }

/* BuildInfo */
static UA_INLINE UA_StatusCode UA_BuildInfo_encodeBinary(const UA_BuildInfo *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BUILDINFO], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_BuildInfo_decodeBinary(const UA_ByteString *src, size_t *offset, UA_BuildInfo *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BUILDINFO]); }

/* NodeClass */
static UA_INLINE UA_StatusCode UA_NodeClass_encodeBinary(const UA_NodeClass *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_NODECLASS], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_NodeClass_decodeBinary(const UA_ByteString *src, size_t *offset, UA_NodeClass *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_NODECLASS]); }

/* ChannelSecurityToken */
static UA_INLINE UA_StatusCode UA_ChannelSecurityToken_encodeBinary(const UA_ChannelSecurityToken *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CHANNELSECURITYTOKEN], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ChannelSecurityToken_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ChannelSecurityToken *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CHANNELSECURITYTOKEN]); }

/* MonitoredItemNotification */
static UA_INLINE UA_StatusCode UA_MonitoredItemNotification_encodeBinary(const UA_MonitoredItemNotification *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_MONITOREDITEMNOTIFICATION], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_MonitoredItemNotification_decodeBinary(const UA_ByteString *src, size_t *offset, UA_MonitoredItemNotification *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMNOTIFICATION]); }

/* DeleteNodesItem */
static UA_INLINE UA_StatusCode UA_DeleteNodesItem_encodeBinary(const UA_DeleteNodesItem *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DELETENODESITEM], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DeleteNodesItem_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DeleteNodesItem *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DELETENODESITEM]); }

/* SubscriptionAcknowledgement */
static UA_INLINE UA_StatusCode UA_SubscriptionAcknowledgement_encodeBinary(const UA_SubscriptionAcknowledgement *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_SUBSCRIPTIONACKNOWLEDGEMENT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_SubscriptionAcknowledgement_decodeBinary(const UA_ByteString *src, size_t *offset, UA_SubscriptionAcknowledgement *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_SUBSCRIPTIONACKNOWLEDGEMENT]); }

/* ReadValueId */
static UA_INLINE UA_StatusCode UA_ReadValueId_encodeBinary(const UA_ReadValueId *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_READVALUEID], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ReadValueId_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ReadValueId *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_READVALUEID]); }

/* AnonymousIdentityToken */
static UA_INLINE UA_StatusCode UA_AnonymousIdentityToken_encodeBinary(const UA_AnonymousIdentityToken *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_AnonymousIdentityToken_decodeBinary(const UA_ByteString *src, size_t *offset, UA_AnonymousIdentityToken *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN]); }

/* DataTypeAttributes */
static UA_INLINE UA_StatusCode UA_DataTypeAttributes_encodeBinary(const UA_DataTypeAttributes *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DataTypeAttributes_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DataTypeAttributes *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES]); }

/* ResponseHeader */
static UA_INLINE UA_StatusCode UA_ResponseHeader_encodeBinary(const UA_ResponseHeader *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_RESPONSEHEADER], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ResponseHeader_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ResponseHeader *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_RESPONSEHEADER]); }

/* DeleteSubscriptionsRequest */
static UA_INLINE UA_StatusCode UA_DeleteSubscriptionsRequest_encodeBinary(const UA_DeleteSubscriptionsRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DeleteSubscriptionsRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DeleteSubscriptionsRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSREQUEST]); }

/* DataChangeNotification */
static UA_INLINE UA_StatusCode UA_DataChangeNotification_encodeBinary(const UA_DataChangeNotification *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DATACHANGENOTIFICATION], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DataChangeNotification_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DataChangeNotification *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DATACHANGENOTIFICATION]); }

/* DeleteMonitoredItemsResponse */
static UA_INLINE UA_StatusCode UA_DeleteMonitoredItemsResponse_encodeBinary(const UA_DeleteMonitoredItemsResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DeleteMonitoredItemsResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DeleteMonitoredItemsResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSRESPONSE]); }

/* RelativePath */
static UA_INLINE UA_StatusCode UA_RelativePath_encodeBinary(const UA_RelativePath *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_RELATIVEPATH], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_RelativePath_decodeBinary(const UA_ByteString *src, size_t *offset, UA_RelativePath *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_RELATIVEPATH]); }

/* RegisterNodesRequest */
static UA_INLINE UA_StatusCode UA_RegisterNodesRequest_encodeBinary(const UA_RegisterNodesRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_REGISTERNODESREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_RegisterNodesRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_RegisterNodesRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_REGISTERNODESREQUEST]); }

/* DeleteNodesRequest */
static UA_INLINE UA_StatusCode UA_DeleteNodesRequest_encodeBinary(const UA_DeleteNodesRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DELETENODESREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DeleteNodesRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DeleteNodesRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DELETENODESREQUEST]); }

/* PublishResponse */
static UA_INLINE UA_StatusCode UA_PublishResponse_encodeBinary(const UA_PublishResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_PUBLISHRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_PublishResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_PublishResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_PUBLISHRESPONSE]); }

/* MonitoredItemModifyRequest */
static UA_INLINE UA_StatusCode UA_MonitoredItemModifyRequest_encodeBinary(const UA_MonitoredItemModifyRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_MonitoredItemModifyRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_MonitoredItemModifyRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYREQUEST]); }

/* UserNameIdentityToken */
static UA_INLINE UA_StatusCode UA_UserNameIdentityToken_encodeBinary(const UA_UserNameIdentityToken *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_UserNameIdentityToken_decodeBinary(const UA_ByteString *src, size_t *offset, UA_UserNameIdentityToken *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN]); }

/* IdType */
static UA_INLINE UA_StatusCode UA_IdType_encodeBinary(const UA_IdType *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_IDTYPE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_IdType_decodeBinary(const UA_ByteString *src, size_t *offset, UA_IdType *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_IDTYPE]); }

/* UserTokenType */
static UA_INLINE UA_StatusCode UA_UserTokenType_encodeBinary(const UA_UserTokenType *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_USERTOKENTYPE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_UserTokenType_decodeBinary(const UA_ByteString *src, size_t *offset, UA_UserTokenType *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_USERTOKENTYPE]); }

/* NodeAttributes */
static UA_INLINE UA_StatusCode UA_NodeAttributes_encodeBinary(const UA_NodeAttributes *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_NODEATTRIBUTES], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_NodeAttributes_decodeBinary(const UA_ByteString *src, size_t *offset, UA_NodeAttributes *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_NODEATTRIBUTES]); }

/* ActivateSessionRequest */
static UA_INLINE UA_StatusCode UA_ActivateSessionRequest_encodeBinary(const UA_ActivateSessionRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_ACTIVATESESSIONREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ActivateSessionRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ActivateSessionRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_ACTIVATESESSIONREQUEST]); }

/* OpenSecureChannelResponse */
static UA_INLINE UA_StatusCode UA_OpenSecureChannelResponse_encodeBinary(const UA_OpenSecureChannelResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_OPENSECURECHANNELRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_OpenSecureChannelResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_OpenSecureChannelResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_OPENSECURECHANNELRESPONSE]); }

/* ApplicationType */
static UA_INLINE UA_StatusCode UA_ApplicationType_encodeBinary(const UA_ApplicationType *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_APPLICATIONTYPE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ApplicationType_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ApplicationType *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_APPLICATIONTYPE]); }

/* QueryNextResponse */
static UA_INLINE UA_StatusCode UA_QueryNextResponse_encodeBinary(const UA_QueryNextResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_QUERYNEXTRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_QueryNextResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_QueryNextResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_QUERYNEXTRESPONSE]); }

/* ActivateSessionResponse */
static UA_INLINE UA_StatusCode UA_ActivateSessionResponse_encodeBinary(const UA_ActivateSessionResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_ACTIVATESESSIONRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ActivateSessionResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ActivateSessionResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_ACTIVATESESSIONRESPONSE]); }

/* FilterOperator */
static UA_INLINE UA_StatusCode UA_FilterOperator_encodeBinary(const UA_FilterOperator *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_FILTEROPERATOR], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_FilterOperator_decodeBinary(const UA_ByteString *src, size_t *offset, UA_FilterOperator *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_FILTEROPERATOR]); }

/* QueryNextRequest */
static UA_INLINE UA_StatusCode UA_QueryNextRequest_encodeBinary(const UA_QueryNextRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_QUERYNEXTREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_QueryNextRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_QueryNextRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_QUERYNEXTREQUEST]); }

/* BrowseNextRequest */
static UA_INLINE UA_StatusCode UA_BrowseNextRequest_encodeBinary(const UA_BrowseNextRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BROWSENEXTREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_BrowseNextRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_BrowseNextRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BROWSENEXTREQUEST]); }

/* CreateSubscriptionRequest */
static UA_INLINE UA_StatusCode UA_CreateSubscriptionRequest_encodeBinary(const UA_CreateSubscriptionRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_CreateSubscriptionRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_CreateSubscriptionRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONREQUEST]); }

/* VariableTypeAttributes */
static UA_INLINE UA_StatusCode UA_VariableTypeAttributes_encodeBinary(const UA_VariableTypeAttributes *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_VARIABLETYPEATTRIBUTES], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_VariableTypeAttributes_decodeBinary(const UA_ByteString *src, size_t *offset, UA_VariableTypeAttributes *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_VARIABLETYPEATTRIBUTES]); }

/* BrowsePathResult */
static UA_INLINE UA_StatusCode UA_BrowsePathResult_encodeBinary(const UA_BrowsePathResult *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BROWSEPATHRESULT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_BrowsePathResult_decodeBinary(const UA_ByteString *src, size_t *offset, UA_BrowsePathResult *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BROWSEPATHRESULT]); }

/* ModifySubscriptionResponse */
static UA_INLINE UA_StatusCode UA_ModifySubscriptionResponse_encodeBinary(const UA_ModifySubscriptionResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ModifySubscriptionResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ModifySubscriptionResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE]); }

/* RegisterNodesResponse */
static UA_INLINE UA_StatusCode UA_RegisterNodesResponse_encodeBinary(const UA_RegisterNodesResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_REGISTERNODESRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_RegisterNodesResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_RegisterNodesResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_REGISTERNODESRESPONSE]); }

/* CloseSessionRequest */
static UA_INLINE UA_StatusCode UA_CloseSessionRequest_encodeBinary(const UA_CloseSessionRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CLOSESESSIONREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_CloseSessionRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_CloseSessionRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CLOSESESSIONREQUEST]); }

/* ModifySubscriptionRequest */
static UA_INLINE UA_StatusCode UA_ModifySubscriptionRequest_encodeBinary(const UA_ModifySubscriptionRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ModifySubscriptionRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ModifySubscriptionRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONREQUEST]); }

/* UserTokenPolicy */
static UA_INLINE UA_StatusCode UA_UserTokenPolicy_encodeBinary(const UA_UserTokenPolicy *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_USERTOKENPOLICY], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_UserTokenPolicy_decodeBinary(const UA_ByteString *src, size_t *offset, UA_UserTokenPolicy *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_USERTOKENPOLICY]); }

/* DeleteMonitoredItemsRequest */
static UA_INLINE UA_StatusCode UA_DeleteMonitoredItemsRequest_encodeBinary(const UA_DeleteMonitoredItemsRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DeleteMonitoredItemsRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DeleteMonitoredItemsRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSREQUEST]); }

/* ReferenceTypeAttributes */
static UA_INLINE UA_StatusCode UA_ReferenceTypeAttributes_encodeBinary(const UA_ReferenceTypeAttributes *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ReferenceTypeAttributes_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ReferenceTypeAttributes *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES]); }

/* BrowsePath */
static UA_INLINE UA_StatusCode UA_BrowsePath_encodeBinary(const UA_BrowsePath *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BROWSEPATH], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_BrowsePath_decodeBinary(const UA_ByteString *src, size_t *offset, UA_BrowsePath *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BROWSEPATH]); }

/* UnregisterNodesResponse */
static UA_INLINE UA_StatusCode UA_UnregisterNodesResponse_encodeBinary(const UA_UnregisterNodesResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_UNREGISTERNODESRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_UnregisterNodesResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_UnregisterNodesResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_UNREGISTERNODESRESPONSE]); }

/* WriteRequest */
static UA_INLINE UA_StatusCode UA_WriteRequest_encodeBinary(const UA_WriteRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_WRITEREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_WriteRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_WriteRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_WRITEREQUEST]); }

/* ObjectAttributes */
static UA_INLINE UA_StatusCode UA_ObjectAttributes_encodeBinary(const UA_ObjectAttributes *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ObjectAttributes_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ObjectAttributes *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES]); }

/* BrowseDescription */
static UA_INLINE UA_StatusCode UA_BrowseDescription_encodeBinary(const UA_BrowseDescription *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BROWSEDESCRIPTION], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_BrowseDescription_decodeBinary(const UA_ByteString *src, size_t *offset, UA_BrowseDescription *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BROWSEDESCRIPTION]); }

/* RepublishRequest */
static UA_INLINE UA_StatusCode UA_RepublishRequest_encodeBinary(const UA_RepublishRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_REPUBLISHREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_RepublishRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_RepublishRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_REPUBLISHREQUEST]); }

/* GetEndpointsRequest */
static UA_INLINE UA_StatusCode UA_GetEndpointsRequest_encodeBinary(const UA_GetEndpointsRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_GETENDPOINTSREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_GetEndpointsRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_GetEndpointsRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_GETENDPOINTSREQUEST]); }

/* PublishRequest */
static UA_INLINE UA_StatusCode UA_PublishRequest_encodeBinary(const UA_PublishRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_PUBLISHREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_PublishRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_PublishRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_PUBLISHREQUEST]); }

/* AddNodesResponse */
static UA_INLINE UA_StatusCode UA_AddNodesResponse_encodeBinary(const UA_AddNodesResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_ADDNODESRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_AddNodesResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_AddNodesResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_ADDNODESRESPONSE]); }

/* CloseSecureChannelResponse */
static UA_INLINE UA_StatusCode UA_CloseSecureChannelResponse_encodeBinary(const UA_CloseSecureChannelResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_CloseSecureChannelResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_CloseSecureChannelResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELRESPONSE]); }

/* ModifyMonitoredItemsRequest */
static UA_INLINE UA_StatusCode UA_ModifyMonitoredItemsRequest_encodeBinary(const UA_ModifyMonitoredItemsRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ModifyMonitoredItemsRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ModifyMonitoredItemsRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST]); }

/* FindServersRequest */
static UA_INLINE UA_StatusCode UA_FindServersRequest_encodeBinary(const UA_FindServersRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_FINDSERVERSREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_FindServersRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_FindServersRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_FINDSERVERSREQUEST]); }

/* ReferenceDescription */
static UA_INLINE UA_StatusCode UA_ReferenceDescription_encodeBinary(const UA_ReferenceDescription *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ReferenceDescription_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ReferenceDescription *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION]); }

/* SetPublishingModeResponse */
static UA_INLINE UA_StatusCode UA_SetPublishingModeResponse_encodeBinary(const UA_SetPublishingModeResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_SetPublishingModeResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_SetPublishingModeResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE]); }

/* ContentFilterResult */
static UA_INLINE UA_StatusCode UA_ContentFilterResult_encodeBinary(const UA_ContentFilterResult *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CONTENTFILTERRESULT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ContentFilterResult_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ContentFilterResult *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CONTENTFILTERRESULT]); }

/* AddReferencesItem */
static UA_INLINE UA_StatusCode UA_AddReferencesItem_encodeBinary(const UA_AddReferencesItem *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_ADDREFERENCESITEM], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_AddReferencesItem_decodeBinary(const UA_ByteString *src, size_t *offset, UA_AddReferencesItem *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_ADDREFERENCESITEM]); }

/* QueryDataDescription */
static UA_INLINE UA_StatusCode UA_QueryDataDescription_encodeBinary(const UA_QueryDataDescription *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_QUERYDATADESCRIPTION], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_QueryDataDescription_decodeBinary(const UA_ByteString *src, size_t *offset, UA_QueryDataDescription *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_QUERYDATADESCRIPTION]); }

/* CreateSubscriptionResponse */
static UA_INLINE UA_StatusCode UA_CreateSubscriptionResponse_encodeBinary(const UA_CreateSubscriptionResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_CreateSubscriptionResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_CreateSubscriptionResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONRESPONSE]); }

/* DeleteSubscriptionsResponse */
static UA_INLINE UA_StatusCode UA_DeleteSubscriptionsResponse_encodeBinary(const UA_DeleteSubscriptionsResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DeleteSubscriptionsResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DeleteSubscriptionsResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSRESPONSE]); }

/* WriteResponse */
static UA_INLINE UA_StatusCode UA_WriteResponse_encodeBinary(const UA_WriteResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_WRITERESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_WriteResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_WriteResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_WRITERESPONSE]); }

/* DeleteReferencesResponse */
static UA_INLINE UA_StatusCode UA_DeleteReferencesResponse_encodeBinary(const UA_DeleteReferencesResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DELETEREFERENCESRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DeleteReferencesResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DeleteReferencesResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DELETEREFERENCESRESPONSE]); }

/* CreateMonitoredItemsResponse */
static UA_INLINE UA_StatusCode UA_CreateMonitoredItemsResponse_encodeBinary(const UA_CreateMonitoredItemsResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_CreateMonitoredItemsResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_CreateMonitoredItemsResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSRESPONSE]); }

/* CallResponse */
static UA_INLINE UA_StatusCode UA_CallResponse_encodeBinary(const UA_CallResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CALLRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_CallResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_CallResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CALLRESPONSE]); }

/* DeleteNodesResponse */
static UA_INLINE UA_StatusCode UA_DeleteNodesResponse_encodeBinary(const UA_DeleteNodesResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DELETENODESRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DeleteNodesResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DeleteNodesResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DELETENODESRESPONSE]); }

/* RepublishResponse */
static UA_INLINE UA_StatusCode UA_RepublishResponse_encodeBinary(const UA_RepublishResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_REPUBLISHRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_RepublishResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_RepublishResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_REPUBLISHRESPONSE]); }

/* MonitoredItemCreateRequest */
static UA_INLINE UA_StatusCode UA_MonitoredItemCreateRequest_encodeBinary(const UA_MonitoredItemCreateRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATEREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_MonitoredItemCreateRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_MonitoredItemCreateRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_MONITOREDITEMCREATEREQUEST]); }

/* DeleteReferencesRequest */
static UA_INLINE UA_StatusCode UA_DeleteReferencesRequest_encodeBinary(const UA_DeleteReferencesRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_DELETEREFERENCESREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_DeleteReferencesRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_DeleteReferencesRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_DELETEREFERENCESREQUEST]); }

/* ModifyMonitoredItemsResponse */
static UA_INLINE UA_StatusCode UA_ModifyMonitoredItemsResponse_encodeBinary(const UA_ModifyMonitoredItemsResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ModifyMonitoredItemsResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ModifyMonitoredItemsResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE]); }

/* ReadResponse */
static UA_INLINE UA_StatusCode UA_ReadResponse_encodeBinary(const UA_ReadResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_READRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ReadResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ReadResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_READRESPONSE]); }

/* AddReferencesRequest */
static UA_INLINE UA_StatusCode UA_AddReferencesRequest_encodeBinary(const UA_AddReferencesRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_ADDREFERENCESREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_AddReferencesRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_AddReferencesRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_ADDREFERENCESREQUEST]); }

/* ReadRequest */
static UA_INLINE UA_StatusCode UA_ReadRequest_encodeBinary(const UA_ReadRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_READREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ReadRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ReadRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_READREQUEST]); }

/* OpenSecureChannelRequest */
static UA_INLINE UA_StatusCode UA_OpenSecureChannelRequest_encodeBinary(const UA_OpenSecureChannelRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_OPENSECURECHANNELREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_OpenSecureChannelRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_OpenSecureChannelRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_OPENSECURECHANNELREQUEST]); }

/* AddNodesItem */
static UA_INLINE UA_StatusCode UA_AddNodesItem_encodeBinary(const UA_AddNodesItem *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_ADDNODESITEM], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_AddNodesItem_decodeBinary(const UA_ByteString *src, size_t *offset, UA_AddNodesItem *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_ADDNODESITEM]); }

/* ApplicationDescription */
static UA_INLINE UA_StatusCode UA_ApplicationDescription_encodeBinary(const UA_ApplicationDescription *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ApplicationDescription_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ApplicationDescription *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]); }

/* NodeTypeDescription */
static UA_INLINE UA_StatusCode UA_NodeTypeDescription_encodeBinary(const UA_NodeTypeDescription *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_NODETYPEDESCRIPTION], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_NodeTypeDescription_decodeBinary(const UA_ByteString *src, size_t *offset, UA_NodeTypeDescription *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_NODETYPEDESCRIPTION]); }

/* FindServersResponse */
static UA_INLINE UA_StatusCode UA_FindServersResponse_encodeBinary(const UA_FindServersResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_FINDSERVERSRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_FindServersResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_FindServersResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_FINDSERVERSRESPONSE]); }

/* ServerStatusDataType */
static UA_INLINE UA_StatusCode UA_ServerStatusDataType_encodeBinary(const UA_ServerStatusDataType *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_SERVERSTATUSDATATYPE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ServerStatusDataType_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ServerStatusDataType *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_SERVERSTATUSDATATYPE]); }

/* AddReferencesResponse */
static UA_INLINE UA_StatusCode UA_AddReferencesResponse_encodeBinary(const UA_AddReferencesResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_ADDREFERENCESRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_AddReferencesResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_AddReferencesResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_ADDREFERENCESRESPONSE]); }

/* TranslateBrowsePathsToNodeIdsResponse */
static UA_INLINE UA_StatusCode UA_TranslateBrowsePathsToNodeIdsResponse_encodeBinary(const UA_TranslateBrowsePathsToNodeIdsResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_TranslateBrowsePathsToNodeIdsResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_TranslateBrowsePathsToNodeIdsResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE]); }

/* ContentFilterElement */
static UA_INLINE UA_StatusCode UA_ContentFilterElement_encodeBinary(const UA_ContentFilterElement *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ContentFilterElement_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ContentFilterElement *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CONTENTFILTERELEMENT]); }

/* TranslateBrowsePathsToNodeIdsRequest */
static UA_INLINE UA_StatusCode UA_TranslateBrowsePathsToNodeIdsRequest_encodeBinary(const UA_TranslateBrowsePathsToNodeIdsRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_TranslateBrowsePathsToNodeIdsRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_TranslateBrowsePathsToNodeIdsRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST]); }

/* CloseSessionResponse */
static UA_INLINE UA_StatusCode UA_CloseSessionResponse_encodeBinary(const UA_CloseSessionResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CLOSESESSIONRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_CloseSessionResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_CloseSessionResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CLOSESESSIONRESPONSE]); }

/* ServiceFault */
static UA_INLINE UA_StatusCode UA_ServiceFault_encodeBinary(const UA_ServiceFault *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_SERVICEFAULT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ServiceFault_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ServiceFault *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_SERVICEFAULT]); }

/* CreateMonitoredItemsRequest */
static UA_INLINE UA_StatusCode UA_CreateMonitoredItemsRequest_encodeBinary(const UA_CreateMonitoredItemsRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_CreateMonitoredItemsRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_CreateMonitoredItemsRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSREQUEST]); }

/* ContentFilter */
static UA_INLINE UA_StatusCode UA_ContentFilter_encodeBinary(const UA_ContentFilter *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CONTENTFILTER], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ContentFilter_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ContentFilter *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CONTENTFILTER]); }

/* QueryFirstResponse */
static UA_INLINE UA_StatusCode UA_QueryFirstResponse_encodeBinary(const UA_QueryFirstResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_QUERYFIRSTRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_QueryFirstResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_QueryFirstResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_QUERYFIRSTRESPONSE]); }

/* AddNodesRequest */
static UA_INLINE UA_StatusCode UA_AddNodesRequest_encodeBinary(const UA_AddNodesRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_ADDNODESREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_AddNodesRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_AddNodesRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_ADDNODESREQUEST]); }

/* BrowseRequest */
static UA_INLINE UA_StatusCode UA_BrowseRequest_encodeBinary(const UA_BrowseRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BROWSEREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_BrowseRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_BrowseRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BROWSEREQUEST]); }

/* BrowseResult */
static UA_INLINE UA_StatusCode UA_BrowseResult_encodeBinary(const UA_BrowseResult *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BROWSERESULT], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_BrowseResult_decodeBinary(const UA_ByteString *src, size_t *offset, UA_BrowseResult *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BROWSERESULT]); }

/* CreateSessionRequest */
static UA_INLINE UA_StatusCode UA_CreateSessionRequest_encodeBinary(const UA_CreateSessionRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CREATESESSIONREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_CreateSessionRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_CreateSessionRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CREATESESSIONREQUEST]); }

/* EndpointDescription */
static UA_INLINE UA_StatusCode UA_EndpointDescription_encodeBinary(const UA_EndpointDescription *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_EndpointDescription_decodeBinary(const UA_ByteString *src, size_t *offset, UA_EndpointDescription *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]); }

/* GetEndpointsResponse */
static UA_INLINE UA_StatusCode UA_GetEndpointsResponse_encodeBinary(const UA_GetEndpointsResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_GETENDPOINTSRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_GetEndpointsResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_GetEndpointsResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_GETENDPOINTSRESPONSE]); }

/* BrowseNextResponse */
static UA_INLINE UA_StatusCode UA_BrowseNextResponse_encodeBinary(const UA_BrowseNextResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_BrowseNextResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_BrowseNextResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE]); }

/* BrowseResponse */
static UA_INLINE UA_StatusCode UA_BrowseResponse_encodeBinary(const UA_BrowseResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_BROWSERESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_BrowseResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_BrowseResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_BROWSERESPONSE]); }

/* CreateSessionResponse */
static UA_INLINE UA_StatusCode UA_CreateSessionResponse_encodeBinary(const UA_CreateSessionResponse *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_CREATESESSIONRESPONSE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_CreateSessionResponse_decodeBinary(const UA_ByteString *src, size_t *offset, UA_CreateSessionResponse *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_CREATESESSIONRESPONSE]); }

/* QueryFirstRequest */
static UA_INLINE UA_StatusCode UA_QueryFirstRequest_encodeBinary(const UA_QueryFirstRequest *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TYPES[UA_TYPES_QUERYFIRSTREQUEST], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_QueryFirstRequest_decodeBinary(const UA_ByteString *src, size_t *offset, UA_QueryFirstRequest *dst) { return UA_decodeBinary(src, offset, dst, &UA_TYPES[UA_TYPES_QUERYFIRSTREQUEST]); }

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/build/src_generated/ua_transport_generated.h" ***********************************/

/* Generated from Opc.Ua.Types.bsd, Custom.Opc.Ua.Transport.bsd with script /home/travis/build/open62541/open62541/tools/generate_datatypes.py
 * on host testing-worker-linux-docker-8551a97d-3382-linux-4 by user travis at 2016-05-18 08:48:03 */


#ifdef __cplusplus
extern "C" {
#endif

#ifdef UA_INTERNAL
#endif


/**
 * Additional Data Type Definitions
 * ================================
 */

#define UA_TRANSPORT_COUNT 11
extern UA_EXPORT const UA_DataType UA_TRANSPORT[UA_TRANSPORT_COUNT];

/**
 * SecureConversationMessageAbortBody
 * ----------------------------------
 * Secure Conversation Message Abort Body */
typedef struct {
    UA_UInt32 error;
    UA_String reason;
} UA_SecureConversationMessageAbortBody;

#define UA_TRANSPORT_SECURECONVERSATIONMESSAGEABORTBODY 0
static UA_INLINE void UA_SecureConversationMessageAbortBody_init(UA_SecureConversationMessageAbortBody *p) { memset(p, 0, sizeof(UA_SecureConversationMessageAbortBody)); }
static UA_INLINE UA_SecureConversationMessageAbortBody * UA_SecureConversationMessageAbortBody_new(void) { return (UA_SecureConversationMessageAbortBody*) UA_new(&UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEABORTBODY]); }
static UA_INLINE UA_StatusCode UA_SecureConversationMessageAbortBody_copy(const UA_SecureConversationMessageAbortBody *src, UA_SecureConversationMessageAbortBody *dst) { return UA_copy(src, dst, &UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEABORTBODY]); }
static UA_INLINE void UA_SecureConversationMessageAbortBody_deleteMembers(UA_SecureConversationMessageAbortBody *p) { UA_deleteMembers(p, &UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEABORTBODY]); }
static UA_INLINE void UA_SecureConversationMessageAbortBody_delete(UA_SecureConversationMessageAbortBody *p) { UA_delete(p, &UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEABORTBODY]); }

/**
 * SecureConversationMessageFooter
 * -------------------------------
 * Secure Conversation Message Footer */
typedef struct {
    size_t paddingSize;
    UA_Byte *padding;
    UA_Byte signature;
} UA_SecureConversationMessageFooter;

#define UA_TRANSPORT_SECURECONVERSATIONMESSAGEFOOTER 1
static UA_INLINE void UA_SecureConversationMessageFooter_init(UA_SecureConversationMessageFooter *p) { memset(p, 0, sizeof(UA_SecureConversationMessageFooter)); }
static UA_INLINE UA_SecureConversationMessageFooter * UA_SecureConversationMessageFooter_new(void) { return (UA_SecureConversationMessageFooter*) UA_new(&UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEFOOTER]); }
static UA_INLINE UA_StatusCode UA_SecureConversationMessageFooter_copy(const UA_SecureConversationMessageFooter *src, UA_SecureConversationMessageFooter *dst) { return UA_copy(src, dst, &UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEFOOTER]); }
static UA_INLINE void UA_SecureConversationMessageFooter_deleteMembers(UA_SecureConversationMessageFooter *p) { UA_deleteMembers(p, &UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEFOOTER]); }
static UA_INLINE void UA_SecureConversationMessageFooter_delete(UA_SecureConversationMessageFooter *p) { UA_delete(p, &UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEFOOTER]); }

/**
 * TcpHelloMessage
 * ---------------
 * Hello Message */
typedef struct {
    UA_UInt32 protocolVersion;
    UA_UInt32 receiveBufferSize;
    UA_UInt32 sendBufferSize;
    UA_UInt32 maxMessageSize;
    UA_UInt32 maxChunkCount;
    UA_String endpointUrl;
} UA_TcpHelloMessage;

#define UA_TRANSPORT_TCPHELLOMESSAGE 2
static UA_INLINE void UA_TcpHelloMessage_init(UA_TcpHelloMessage *p) { memset(p, 0, sizeof(UA_TcpHelloMessage)); }
static UA_INLINE UA_TcpHelloMessage * UA_TcpHelloMessage_new(void) { return (UA_TcpHelloMessage*) UA_new(&UA_TRANSPORT[UA_TRANSPORT_TCPHELLOMESSAGE]); }
static UA_INLINE UA_StatusCode UA_TcpHelloMessage_copy(const UA_TcpHelloMessage *src, UA_TcpHelloMessage *dst) { return UA_copy(src, dst, &UA_TRANSPORT[UA_TRANSPORT_TCPHELLOMESSAGE]); }
static UA_INLINE void UA_TcpHelloMessage_deleteMembers(UA_TcpHelloMessage *p) { UA_deleteMembers(p, &UA_TRANSPORT[UA_TRANSPORT_TCPHELLOMESSAGE]); }
static UA_INLINE void UA_TcpHelloMessage_delete(UA_TcpHelloMessage *p) { UA_delete(p, &UA_TRANSPORT[UA_TRANSPORT_TCPHELLOMESSAGE]); }

/**
 * MessageType
 * -----------
 * Message Type and whether the message contains an intermediate chunk */
typedef enum { 
    UA_MESSAGETYPE_ACK = 0x4B4341,
    UA_MESSAGETYPE_HEL = 0x4C4548,
    UA_MESSAGETYPE_MSG = 0x47534D,
    UA_MESSAGETYPE_OPN = 0x4E504F,
    UA_MESSAGETYPE_CLO = 0x4F4C43
} UA_MessageType;

#define UA_TRANSPORT_MESSAGETYPE 3
static UA_INLINE void UA_MessageType_init(UA_MessageType *p) { memset(p, 0, sizeof(UA_MessageType)); }
static UA_INLINE UA_MessageType * UA_MessageType_new(void) { return (UA_MessageType*) UA_new(&UA_TRANSPORT[UA_TRANSPORT_MESSAGETYPE]); }
static UA_INLINE UA_StatusCode UA_MessageType_copy(const UA_MessageType *src, UA_MessageType *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_MessageType_deleteMembers(UA_MessageType *p) { }
static UA_INLINE void UA_MessageType_delete(UA_MessageType *p) { UA_delete(p, &UA_TRANSPORT[UA_TRANSPORT_MESSAGETYPE]); }

/**
 * AsymmetricAlgorithmSecurityHeader
 * ---------------------------------
 * Security Header */
typedef struct {
    UA_ByteString securityPolicyUri;
    UA_ByteString senderCertificate;
    UA_ByteString receiverCertificateThumbprint;
} UA_AsymmetricAlgorithmSecurityHeader;

#define UA_TRANSPORT_ASYMMETRICALGORITHMSECURITYHEADER 4
static UA_INLINE void UA_AsymmetricAlgorithmSecurityHeader_init(UA_AsymmetricAlgorithmSecurityHeader *p) { memset(p, 0, sizeof(UA_AsymmetricAlgorithmSecurityHeader)); }
static UA_INLINE UA_AsymmetricAlgorithmSecurityHeader * UA_AsymmetricAlgorithmSecurityHeader_new(void) { return (UA_AsymmetricAlgorithmSecurityHeader*) UA_new(&UA_TRANSPORT[UA_TRANSPORT_ASYMMETRICALGORITHMSECURITYHEADER]); }
static UA_INLINE UA_StatusCode UA_AsymmetricAlgorithmSecurityHeader_copy(const UA_AsymmetricAlgorithmSecurityHeader *src, UA_AsymmetricAlgorithmSecurityHeader *dst) { return UA_copy(src, dst, &UA_TRANSPORT[UA_TRANSPORT_ASYMMETRICALGORITHMSECURITYHEADER]); }
static UA_INLINE void UA_AsymmetricAlgorithmSecurityHeader_deleteMembers(UA_AsymmetricAlgorithmSecurityHeader *p) { UA_deleteMembers(p, &UA_TRANSPORT[UA_TRANSPORT_ASYMMETRICALGORITHMSECURITYHEADER]); }
static UA_INLINE void UA_AsymmetricAlgorithmSecurityHeader_delete(UA_AsymmetricAlgorithmSecurityHeader *p) { UA_delete(p, &UA_TRANSPORT[UA_TRANSPORT_ASYMMETRICALGORITHMSECURITYHEADER]); }

/**
 * TcpAcknowledgeMessage
 * ---------------------
 * Acknowledge Message */
typedef struct {
    UA_UInt32 protocolVersion;
    UA_UInt32 receiveBufferSize;
    UA_UInt32 sendBufferSize;
    UA_UInt32 maxMessageSize;
    UA_UInt32 maxChunkCount;
} UA_TcpAcknowledgeMessage;

#define UA_TRANSPORT_TCPACKNOWLEDGEMESSAGE 5
static UA_INLINE void UA_TcpAcknowledgeMessage_init(UA_TcpAcknowledgeMessage *p) { memset(p, 0, sizeof(UA_TcpAcknowledgeMessage)); }
static UA_INLINE UA_TcpAcknowledgeMessage * UA_TcpAcknowledgeMessage_new(void) { return (UA_TcpAcknowledgeMessage*) UA_new(&UA_TRANSPORT[UA_TRANSPORT_TCPACKNOWLEDGEMESSAGE]); }
static UA_INLINE UA_StatusCode UA_TcpAcknowledgeMessage_copy(const UA_TcpAcknowledgeMessage *src, UA_TcpAcknowledgeMessage *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_TcpAcknowledgeMessage_deleteMembers(UA_TcpAcknowledgeMessage *p) { }
static UA_INLINE void UA_TcpAcknowledgeMessage_delete(UA_TcpAcknowledgeMessage *p) { UA_delete(p, &UA_TRANSPORT[UA_TRANSPORT_TCPACKNOWLEDGEMESSAGE]); }

/**
 * SequenceHeader
 * --------------
 * Secure Layer Sequence Header */
typedef struct {
    UA_UInt32 sequenceNumber;
    UA_UInt32 requestId;
} UA_SequenceHeader;

#define UA_TRANSPORT_SEQUENCEHEADER 6
static UA_INLINE void UA_SequenceHeader_init(UA_SequenceHeader *p) { memset(p, 0, sizeof(UA_SequenceHeader)); }
static UA_INLINE UA_SequenceHeader * UA_SequenceHeader_new(void) { return (UA_SequenceHeader*) UA_new(&UA_TRANSPORT[UA_TRANSPORT_SEQUENCEHEADER]); }
static UA_INLINE UA_StatusCode UA_SequenceHeader_copy(const UA_SequenceHeader *src, UA_SequenceHeader *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_SequenceHeader_deleteMembers(UA_SequenceHeader *p) { }
static UA_INLINE void UA_SequenceHeader_delete(UA_SequenceHeader *p) { UA_delete(p, &UA_TRANSPORT[UA_TRANSPORT_SEQUENCEHEADER]); }

/**
 * TcpMessageHeader
 * ----------------
 * TCP Header */
typedef struct {
    UA_UInt32 messageTypeAndChunkType;
    UA_UInt32 messageSize;
} UA_TcpMessageHeader;

#define UA_TRANSPORT_TCPMESSAGEHEADER 7
static UA_INLINE void UA_TcpMessageHeader_init(UA_TcpMessageHeader *p) { memset(p, 0, sizeof(UA_TcpMessageHeader)); }
static UA_INLINE UA_TcpMessageHeader * UA_TcpMessageHeader_new(void) { return (UA_TcpMessageHeader*) UA_new(&UA_TRANSPORT[UA_TRANSPORT_TCPMESSAGEHEADER]); }
static UA_INLINE UA_StatusCode UA_TcpMessageHeader_copy(const UA_TcpMessageHeader *src, UA_TcpMessageHeader *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_TcpMessageHeader_deleteMembers(UA_TcpMessageHeader *p) { }
static UA_INLINE void UA_TcpMessageHeader_delete(UA_TcpMessageHeader *p) { UA_delete(p, &UA_TRANSPORT[UA_TRANSPORT_TCPMESSAGEHEADER]); }

/**
 * ChunkType
 * ---------
 * Type of the chunk */
typedef enum { 
    UA_CHUNKTYPE_FINAL = 0x46000000,
    UA_CHUNKTYPE_INTERMEDIATE = 0x43000000,
    UA_CHUNKTYPE_ABORT = 0x41000000
} UA_ChunkType;

#define UA_TRANSPORT_CHUNKTYPE 8
static UA_INLINE void UA_ChunkType_init(UA_ChunkType *p) { memset(p, 0, sizeof(UA_ChunkType)); }
static UA_INLINE UA_ChunkType * UA_ChunkType_new(void) { return (UA_ChunkType*) UA_new(&UA_TRANSPORT[UA_TRANSPORT_CHUNKTYPE]); }
static UA_INLINE UA_StatusCode UA_ChunkType_copy(const UA_ChunkType *src, UA_ChunkType *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_ChunkType_deleteMembers(UA_ChunkType *p) { }
static UA_INLINE void UA_ChunkType_delete(UA_ChunkType *p) { UA_delete(p, &UA_TRANSPORT[UA_TRANSPORT_CHUNKTYPE]); }

/**
 * SymmetricAlgorithmSecurityHeader
 * --------------------------------
 * Secure Layer Symmetric Algorithm Header */
typedef struct {
    UA_UInt32 tokenId;
} UA_SymmetricAlgorithmSecurityHeader;

#define UA_TRANSPORT_SYMMETRICALGORITHMSECURITYHEADER 9
static UA_INLINE void UA_SymmetricAlgorithmSecurityHeader_init(UA_SymmetricAlgorithmSecurityHeader *p) { memset(p, 0, sizeof(UA_SymmetricAlgorithmSecurityHeader)); }
static UA_INLINE UA_SymmetricAlgorithmSecurityHeader * UA_SymmetricAlgorithmSecurityHeader_new(void) { return (UA_SymmetricAlgorithmSecurityHeader*) UA_new(&UA_TRANSPORT[UA_TRANSPORT_SYMMETRICALGORITHMSECURITYHEADER]); }
static UA_INLINE UA_StatusCode UA_SymmetricAlgorithmSecurityHeader_copy(const UA_SymmetricAlgorithmSecurityHeader *src, UA_SymmetricAlgorithmSecurityHeader *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_SymmetricAlgorithmSecurityHeader_deleteMembers(UA_SymmetricAlgorithmSecurityHeader *p) { }
static UA_INLINE void UA_SymmetricAlgorithmSecurityHeader_delete(UA_SymmetricAlgorithmSecurityHeader *p) { UA_delete(p, &UA_TRANSPORT[UA_TRANSPORT_SYMMETRICALGORITHMSECURITYHEADER]); }

/**
 * SecureConversationMessageHeader
 * -------------------------------
 * Secure Layer Sequence Header */
typedef struct {
    UA_TcpMessageHeader messageHeader;
    UA_UInt32 secureChannelId;
} UA_SecureConversationMessageHeader;

#define UA_TRANSPORT_SECURECONVERSATIONMESSAGEHEADER 10
static UA_INLINE void UA_SecureConversationMessageHeader_init(UA_SecureConversationMessageHeader *p) { memset(p, 0, sizeof(UA_SecureConversationMessageHeader)); }
static UA_INLINE UA_SecureConversationMessageHeader * UA_SecureConversationMessageHeader_new(void) { return (UA_SecureConversationMessageHeader*) UA_new(&UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEHEADER]); }
static UA_INLINE UA_StatusCode UA_SecureConversationMessageHeader_copy(const UA_SecureConversationMessageHeader *src, UA_SecureConversationMessageHeader *dst) { *dst = *src; return UA_STATUSCODE_GOOD; }
static UA_INLINE void UA_SecureConversationMessageHeader_deleteMembers(UA_SecureConversationMessageHeader *p) { }
static UA_INLINE void UA_SecureConversationMessageHeader_delete(UA_SecureConversationMessageHeader *p) { UA_delete(p, &UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEHEADER]); }

#ifdef __cplusplus
} // extern "C"
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/build/src_generated/ua_transport_generated_encoding_binary.h" ***********************************/

/* Generated from Opc.Ua.Types.bsd, Custom.Opc.Ua.Transport.bsd with script /home/travis/build/open62541/open62541/tools/generate_datatypes.py
 * on host testing-worker-linux-docker-8551a97d-3382-linux-4 by user travis at 2016-05-18 08:48:03 */
 

/* SecureConversationMessageAbortBody */
static UA_INLINE UA_StatusCode UA_SecureConversationMessageAbortBody_encodeBinary(const UA_SecureConversationMessageAbortBody *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEABORTBODY], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_SecureConversationMessageAbortBody_decodeBinary(const UA_ByteString *src, size_t *offset, UA_SecureConversationMessageAbortBody *dst) { return UA_decodeBinary(src, offset, dst, &UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEABORTBODY]); }

/* SecureConversationMessageFooter */
static UA_INLINE UA_StatusCode UA_SecureConversationMessageFooter_encodeBinary(const UA_SecureConversationMessageFooter *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEFOOTER], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_SecureConversationMessageFooter_decodeBinary(const UA_ByteString *src, size_t *offset, UA_SecureConversationMessageFooter *dst) { return UA_decodeBinary(src, offset, dst, &UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEFOOTER]); }

/* TcpHelloMessage */
static UA_INLINE UA_StatusCode UA_TcpHelloMessage_encodeBinary(const UA_TcpHelloMessage *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TRANSPORT[UA_TRANSPORT_TCPHELLOMESSAGE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_TcpHelloMessage_decodeBinary(const UA_ByteString *src, size_t *offset, UA_TcpHelloMessage *dst) { return UA_decodeBinary(src, offset, dst, &UA_TRANSPORT[UA_TRANSPORT_TCPHELLOMESSAGE]); }

/* MessageType */
static UA_INLINE UA_StatusCode UA_MessageType_encodeBinary(const UA_MessageType *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TRANSPORT[UA_TRANSPORT_MESSAGETYPE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_MessageType_decodeBinary(const UA_ByteString *src, size_t *offset, UA_MessageType *dst) { return UA_decodeBinary(src, offset, dst, &UA_TRANSPORT[UA_TRANSPORT_MESSAGETYPE]); }

/* AsymmetricAlgorithmSecurityHeader */
static UA_INLINE UA_StatusCode UA_AsymmetricAlgorithmSecurityHeader_encodeBinary(const UA_AsymmetricAlgorithmSecurityHeader *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TRANSPORT[UA_TRANSPORT_ASYMMETRICALGORITHMSECURITYHEADER], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_AsymmetricAlgorithmSecurityHeader_decodeBinary(const UA_ByteString *src, size_t *offset, UA_AsymmetricAlgorithmSecurityHeader *dst) { return UA_decodeBinary(src, offset, dst, &UA_TRANSPORT[UA_TRANSPORT_ASYMMETRICALGORITHMSECURITYHEADER]); }

/* TcpAcknowledgeMessage */
static UA_INLINE UA_StatusCode UA_TcpAcknowledgeMessage_encodeBinary(const UA_TcpAcknowledgeMessage *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TRANSPORT[UA_TRANSPORT_TCPACKNOWLEDGEMESSAGE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_TcpAcknowledgeMessage_decodeBinary(const UA_ByteString *src, size_t *offset, UA_TcpAcknowledgeMessage *dst) { return UA_decodeBinary(src, offset, dst, &UA_TRANSPORT[UA_TRANSPORT_TCPACKNOWLEDGEMESSAGE]); }

/* SequenceHeader */
static UA_INLINE UA_StatusCode UA_SequenceHeader_encodeBinary(const UA_SequenceHeader *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TRANSPORT[UA_TRANSPORT_SEQUENCEHEADER], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_SequenceHeader_decodeBinary(const UA_ByteString *src, size_t *offset, UA_SequenceHeader *dst) { return UA_decodeBinary(src, offset, dst, &UA_TRANSPORT[UA_TRANSPORT_SEQUENCEHEADER]); }

/* TcpMessageHeader */
static UA_INLINE UA_StatusCode UA_TcpMessageHeader_encodeBinary(const UA_TcpMessageHeader *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TRANSPORT[UA_TRANSPORT_TCPMESSAGEHEADER], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_TcpMessageHeader_decodeBinary(const UA_ByteString *src, size_t *offset, UA_TcpMessageHeader *dst) { return UA_decodeBinary(src, offset, dst, &UA_TRANSPORT[UA_TRANSPORT_TCPMESSAGEHEADER]); }

/* ChunkType */
static UA_INLINE UA_StatusCode UA_ChunkType_encodeBinary(const UA_ChunkType *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TRANSPORT[UA_TRANSPORT_CHUNKTYPE], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_ChunkType_decodeBinary(const UA_ByteString *src, size_t *offset, UA_ChunkType *dst) { return UA_decodeBinary(src, offset, dst, &UA_TRANSPORT[UA_TRANSPORT_CHUNKTYPE]); }

/* SymmetricAlgorithmSecurityHeader */
static UA_INLINE UA_StatusCode UA_SymmetricAlgorithmSecurityHeader_encodeBinary(const UA_SymmetricAlgorithmSecurityHeader *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TRANSPORT[UA_TRANSPORT_SYMMETRICALGORITHMSECURITYHEADER], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_SymmetricAlgorithmSecurityHeader_decodeBinary(const UA_ByteString *src, size_t *offset, UA_SymmetricAlgorithmSecurityHeader *dst) { return UA_decodeBinary(src, offset, dst, &UA_TRANSPORT[UA_TRANSPORT_SYMMETRICALGORITHMSECURITYHEADER]); }

/* SecureConversationMessageHeader */
static UA_INLINE UA_StatusCode UA_SecureConversationMessageHeader_encodeBinary(const UA_SecureConversationMessageHeader *src, UA_ByteString *dst, size_t *offset) { return UA_encodeBinary(src, &UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEHEADER], NULL, NULL, dst, offset); }
static UA_INLINE UA_StatusCode UA_SecureConversationMessageHeader_decodeBinary(const UA_ByteString *src, size_t *offset, UA_SecureConversationMessageHeader *dst) { return UA_decodeBinary(src, offset, dst, &UA_TRANSPORT[UA_TRANSPORT_SECURECONVERSATIONMESSAGEHEADER]); }

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/ua_connection_internal.h" ***********************************/



/**
 * The network layer may receive chopped up messages since TCP is a streaming
 * protocol. Furthermore, the networklayer may operate on ringbuffers or
 * statically assigned memory.
 *
 * If an entire message is received, it is forwarded directly. But the memory
 * needs to be freed with the networklayer-specific mechanism. If a half message
 * is received, we copy it into a local buffer. Then, the stack-specific free
 * needs to be used.
 *
 * @param connection The connection
 * @param message The received message. The content may be overwritten when a
 *        previsouly received buffer is completed.
 * @param realloced The Boolean value is set to true if the outgoing message has
 *        been reallocated from the network layer.
 * @return Returns UA_STATUSCODE_GOOD or an error code. When an error occurs, the ingoing message
 *         and the current buffer in the connection are freed.
 */
UA_StatusCode
UA_Connection_completeMessages(UA_Connection *connection, UA_ByteString * UA_RESTRICT message,
                               UA_Boolean * UA_RESTRICT realloced);

void UA_EXPORT UA_Connection_detachSecureChannel(UA_Connection *connection);
void UA_EXPORT UA_Connection_attachSecureChannel(UA_Connection *connection, UA_SecureChannel *channel);


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/ua_securechannel.h" ***********************************/



struct UA_Session;
typedef struct UA_Session UA_Session;

struct SessionEntry {
    LIST_ENTRY(SessionEntry) pointers;
    UA_Session *session; // Just a pointer. The session is held in the session manager or the client
};

/* For chunked requests */
struct ChunkEntry {
    LIST_ENTRY(ChunkEntry) pointers;
    UA_UInt32 requestId;
    UA_Boolean invalid_message;
    UA_ByteString bytes;
};

/* For chunked responses */
typedef struct {
    UA_SecureChannel *channel;
    UA_UInt32 requestId;
    UA_UInt32 messageType;
    UA_UInt16 chunksSoFar;
    size_t messageSizeSoFar;
    UA_Boolean final;
    UA_Boolean abort;
} UA_ChunkInfo;

struct UA_SecureChannel {
    UA_MessageSecurityMode  securityMode;
    UA_ChannelSecurityToken securityToken; // the channelId is contained in the securityToken
    UA_ChannelSecurityToken nextSecurityToken; // the channelId is contained in the securityToken
    UA_AsymmetricAlgorithmSecurityHeader clientAsymAlgSettings;
    UA_AsymmetricAlgorithmSecurityHeader serverAsymAlgSettings;
    UA_ByteString  clientNonce;
    UA_ByteString  serverNonce;
    UA_UInt32      sequenceNumber;
    UA_Connection *connection;
    LIST_HEAD(session_pointerlist, SessionEntry) sessions;
    LIST_HEAD(chunk_pointerlist, ChunkEntry) chunks;
};

void UA_SecureChannel_init(UA_SecureChannel *channel);
void UA_SecureChannel_deleteMembersCleanup(UA_SecureChannel *channel);

UA_StatusCode UA_SecureChannel_generateNonce(UA_ByteString *nonce);

void UA_SecureChannel_attachSession(UA_SecureChannel *channel, UA_Session *session);
void UA_SecureChannel_detachSession(UA_SecureChannel *channel, UA_Session *session);
UA_Session * UA_SecureChannel_getSession(UA_SecureChannel *channel, UA_NodeId *token);

UA_StatusCode UA_SecureChannel_sendBinaryMessage(UA_SecureChannel *channel, UA_UInt32 requestId,
                                                  const void *content, const UA_DataType *contentType);

void UA_SecureChannel_revolveTokens(UA_SecureChannel *channel);


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_nodes.h" ***********************************/



/*
 * Most APIs take and return UA_EditNode and UA_ConstNode. By looking up the
 * nodeclass, nodes can be cast to their "true" class, i.e. UA_VariableNode,
 * UA_ObjectNode, and so on.
 */

#define UA_STANDARD_NODEMEMBERS                 \
    UA_NodeId nodeId;                           \
    UA_NodeClass nodeClass;                     \
    UA_QualifiedName browseName;                \
    UA_LocalizedText displayName;               \
    UA_LocalizedText description;               \
    UA_UInt32 writeMask;                        \
    UA_UInt32 userWriteMask;                    \
    size_t referencesSize;                      \
    UA_ReferenceNode *references;

typedef struct {
    UA_STANDARD_NODEMEMBERS
} UA_Node;

void UA_Node_deleteMembersAnyNodeClass(UA_Node *node);
UA_StatusCode UA_Node_copyAnyNodeClass(const UA_Node *src, UA_Node *dst);

/**************/
/* ObjectNode */
/**************/

typedef struct {
    UA_STANDARD_NODEMEMBERS
    UA_Byte eventNotifier;
    void *instanceHandle;
} UA_ObjectNode;

/******************/
/* ObjectTypeNode */
/******************/

typedef struct {
    UA_STANDARD_NODEMEMBERS
    UA_Boolean isAbstract;
    UA_ObjectLifecycleManagement lifecycleManagement;
} UA_ObjectTypeNode;

typedef enum {
    UA_VALUESOURCE_VARIANT,
    UA_VALUESOURCE_DATASOURCE
} UA_ValueSource;

/****************/
/* VariableNode */
/****************/

typedef struct {
    UA_STANDARD_NODEMEMBERS
    UA_Int32 valueRank; /**< n >= 1: the value is an array with the specified number of dimensions.
                             n = 0: the value is an array with one or more dimensions.
                             n = -1: the value is a scalar.
                             n = -2: the value can be a scalar or an array with any number of dimensions.
                             n = -3:  the value can be a scalar or a one dimensional array. */
    UA_ValueSource valueSource;
    union {
        struct {
        UA_Variant value;
        UA_ValueCallback callback;
        } variant;
        UA_DataSource dataSource;
    } value;
    /* <--- similar to variabletypenodes up to there--->*/
    UA_Byte accessLevel;
    UA_Byte userAccessLevel;
    UA_Double minimumSamplingInterval;
    UA_Boolean historizing;
} UA_VariableNode;

/********************/
/* VariableTypeNode */
/********************/

typedef struct {
    UA_STANDARD_NODEMEMBERS
    UA_Int32 valueRank;
    UA_ValueSource valueSource;
    union {
        struct {
            UA_Variant value;
            UA_ValueCallback callback;
        } variant;
        UA_DataSource dataSource;
    } value;
    /* <--- similar to variablenodes up to there--->*/
    UA_Boolean isAbstract;
} UA_VariableTypeNode;

/*********************/
/* ReferenceTypeNode */
/*********************/

typedef struct {
    UA_STANDARD_NODEMEMBERS
    UA_Boolean isAbstract;
    UA_Boolean symmetric;
    UA_LocalizedText inverseName;
} UA_ReferenceTypeNode;

/**************/
/* MethodNode */
/**************/

typedef struct {
    UA_STANDARD_NODEMEMBERS
    UA_Boolean executable;
    UA_Boolean userExecutable;
    void *methodHandle;
    UA_MethodCallback attachedMethod;
} UA_MethodNode;

/************/
/* ViewNode */
/************/

typedef struct {
    UA_STANDARD_NODEMEMBERS
    UA_Byte eventNotifier;
    /* <-- the same as objectnode until here --> */
    UA_Boolean containsNoLoops;
} UA_ViewNode;

/****************/
/* DataTypeNode */
/****************/

typedef struct {
    UA_STANDARD_NODEMEMBERS
    UA_Boolean isAbstract;
} UA_DataTypeNode;


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/ua_session.h" ***********************************/



#define MAXCONTINUATIONPOINTS 5

struct ContinuationPointEntry {
    LIST_ENTRY(ContinuationPointEntry) pointers;
    UA_ByteString        identifier;
    UA_BrowseDescription browseDescription;
    UA_UInt32            continuationIndex;
    UA_UInt32            maxReferences;
};

struct UA_Subscription;
typedef struct UA_Subscription UA_Subscription;

typedef struct UA_PublishResponseEntry {
    SIMPLEQ_ENTRY(UA_PublishResponseEntry) listEntry;
    UA_UInt32 requestId;
    UA_PublishResponse response;
} UA_PublishResponseEntry;

struct UA_Session {
    UA_ApplicationDescription clientDescription;
    UA_Boolean        activated;
    UA_String         sessionName;
    UA_NodeId         authenticationToken;
    UA_NodeId         sessionId;
    UA_UInt32         maxRequestMessageSize;
    UA_UInt32         maxResponseMessageSize;
    UA_Double         timeout; // [ms]
    UA_DateTime       validTill;
    UA_SecureChannel *channel;
    UA_UInt16 availableContinuationPoints;
    LIST_HEAD(ContinuationPointList, ContinuationPointEntry) continuationPoints;
#ifdef UA_ENABLE_SUBSCRIPTIONS
    UA_UInt32 lastSubscriptionID;
    LIST_HEAD(UA_ListOfUASubscriptions, UA_Subscription) serverSubscriptions;
    SIMPLEQ_HEAD(UA_ListOfQueuedPublishResponses, UA_PublishResponseEntry) responseQueue;
#endif
};

/* Local access to the services (for startup and maintenance) uses this Session
 * with all possible access rights (Session ID: 1) */
extern UA_Session adminSession;

void UA_Session_init(UA_Session *session);
void UA_Session_deleteMembersCleanup(UA_Session *session, UA_Server *server);

/* If any activity on a session happens, the timeout is extended */
void UA_Session_updateLifetime(UA_Session *session);

#ifdef UA_ENABLE_SUBSCRIPTIONS
void UA_Session_addSubscription(UA_Session *session, UA_Subscription *newSubscription);

UA_Subscription *
UA_Session_getSubscriptionByID(UA_Session *session, UA_UInt32 subscriptionID);

UA_StatusCode
UA_Session_deleteSubscription(UA_Server *server, UA_Session *session,
                              UA_UInt32 subscriptionID);

UA_UInt32
UA_Session_getUniqueSubscriptionID(UA_Session *session);
#endif



/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_subscription.h" ***********************************/



/*****************/
/* MonitoredItem */
/*****************/

typedef enum {
    UA_MONITOREDITEMTYPE_CHANGENOTIFY = 1,
    UA_MONITOREDITEMTYPE_STATUSNOTIFY = 2,
    UA_MONITOREDITEMTYPE_EVENTNOTIFY = 4
} UA_MonitoredItemType;

typedef struct MonitoredItem_queuedValue {
    TAILQ_ENTRY(MonitoredItem_queuedValue) listEntry;
    UA_UInt32 clientHandle;
    UA_DataValue value;
} MonitoredItem_queuedValue;

typedef struct UA_MonitoredItem {
    LIST_ENTRY(UA_MonitoredItem) listEntry;

    /* Settings */
    UA_Subscription *subscription;
    UA_UInt32 itemId;
    UA_MonitoredItemType monitoredItemType;
    UA_TimestampsToReturn timestampsToReturn;
    UA_MonitoringMode monitoringMode;
    UA_NodeId monitoredNodeId; 
    UA_UInt32 attributeID;
    UA_UInt32 clientHandle;
    UA_Double samplingInterval; // [ms]
    UA_UInt32 currentQueueSize;
    UA_UInt32 maxQueueSize;
    UA_Boolean discardOldest;
    UA_String indexRange;
    // TODO: dataEncoding is hardcoded to UA binary

    /* Sample Job */
    UA_Guid sampleJobGuid;
    UA_Boolean sampleJobIsRegistered;

    /* Sample Queue */
    UA_ByteString lastSampledValue;
    TAILQ_HEAD(QueueOfQueueDataValues, MonitoredItem_queuedValue) queue;
} UA_MonitoredItem;

UA_MonitoredItem *UA_MonitoredItem_new(void);
void MonitoredItem_delete(UA_Server *server, UA_MonitoredItem *monitoredItem);
UA_StatusCode MonitoredItem_registerSampleJob(UA_Server *server, UA_MonitoredItem *mon);
UA_StatusCode MonitoredItem_unregisterSampleJob(UA_Server *server, UA_MonitoredItem *mon);

/****************/
/* Subscription */
/****************/

typedef struct UA_NotificationMessageEntry {
    LIST_ENTRY(UA_NotificationMessageEntry) listEntry;
    UA_NotificationMessage message;
} UA_NotificationMessageEntry;

/* We use only a subset of the states defined in the standard */
typedef enum {
	/* UA_SUBSCRIPTIONSTATE_CLOSED */
	/* UA_SUBSCRIPTIONSTATE_CREATING */
	UA_SUBSCRIPTIONSTATE_NORMAL,
	UA_SUBSCRIPTIONSTATE_LATE,
    UA_SUBSCRIPTIONSTATE_KEEPALIVE
} UA_SubscriptionState;

struct UA_Subscription {
    LIST_ENTRY(UA_Subscription) listEntry;

    /* Settings */
    UA_Session *session;
    UA_UInt32 lifeTimeCount;
    UA_UInt32 maxKeepAliveCount;
    UA_Double publishingInterval;     // [ms] 
    UA_UInt32 subscriptionID;
    UA_UInt32 notificationsPerPublish;
    UA_Boolean publishingEnabled;
    UA_UInt32 priority;
    UA_UInt32 sequenceNumber;

    /* Runtime information */
	UA_SubscriptionState state;
    UA_UInt32 currentKeepAliveCount;
	UA_UInt32 currentLifetimeCount;

    /* Publish Job */
    UA_Guid publishJobGuid;
    UA_Boolean publishJobIsRegistered;

    LIST_HEAD(UA_ListOfUAMonitoredItems, UA_MonitoredItem) MonitoredItems;
    LIST_HEAD(UA_ListOfNotificationMessages, UA_NotificationMessageEntry) retransmissionQueue;
};

UA_Subscription *UA_Subscription_new(UA_Session *session, UA_UInt32 subscriptionID);
void UA_Subscription_deleteMembers(UA_Subscription *subscription, UA_Server *server);
UA_StatusCode Subscription_registerPublishJob(UA_Server *server, UA_Subscription *sub);
UA_StatusCode Subscription_unregisterPublishJob(UA_Server *server, UA_Subscription *sub);

UA_StatusCode
UA_Subscription_deleteMonitoredItem(UA_Server *server, UA_Subscription *sub,
                                    UA_UInt32 monitoredItemID);

UA_MonitoredItem *
UA_Subscription_getMonitoredItem(UA_Subscription *sub, UA_UInt32 monitoredItemID);

void UA_Subscription_publishCallback(UA_Server *server, UA_Subscription *sub);


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_nodestore.h" ***********************************/


#ifdef __cplusplus
extern "C" {
#endif


/**
 * Nodestore
 * =========
 * Stores nodes that can be indexed by their NodeId. Internally, it is based on
 * a hash-map implementation. */
struct UA_NodeStore;
typedef struct UA_NodeStore UA_NodeStore;

/**
 * Nodestore Lifecycle
 * ------------------- */
/* Create a new nodestore */
UA_NodeStore * UA_NodeStore_new(void);

/* Delete the nodestore and all nodes in it. Do not call from a read-side
   critical section (multithreading). */
void UA_NodeStore_delete(UA_NodeStore *ns);

/**
 * Node Lifecycle
 * ---------------
 *
 * The following definitions are used to create empty nodes of the different
 * node types. The memory is managed by the nodestore. Therefore, the node has
 * to be removed via a special deleteNode function. (If the new node is not
 * added to the nodestore.) */
/* Create an editable node of the given NodeClass. */
UA_Node * UA_NodeStore_newNode(UA_NodeClass nodeClass);
#define UA_NodeStore_newObjectNode() (UA_ObjectNode*)UA_NodeStore_newNode(UA_NODECLASS_OBJECT)
#define UA_NodeStore_newVariableNode() (UA_VariableNode*)UA_NodeStore_newNode(UA_NODECLASS_VARIABLE)
#define UA_NodeStore_newMethodNode() (UA_MethodNode*)UA_NodeStore_newNode(UA_NODECLASS_METHOD)
#define UA_NodeStore_newObjectTypeNode() (UA_ObjectTypeNode*)UA_NodeStore_newNode(UA_NODECLASS_OBJECTTYPE)
#define UA_NodeStore_newVariableTypeNode() (UA_VariableTypeNode*)UA_NodeStore_newNode(UA_NODECLASS_VARIABLETYPE)
#define UA_NodeStore_newReferenceTypeNode() (UA_ReferenceTypeNode*)UA_NodeStore_newNode(UA_NODECLASS_REFERENCETYPE)
#define UA_NodeStore_newDataTypeNode() (UA_DataTypeNode*)UA_NodeStore_newNode(UA_NODECLASS_DATATYPE)
#define UA_NodeStore_newViewNode() (UA_ViewNode*)UA_NodeStore_newNode(UA_NODECLASS_VIEW)

/* Delete an editable node. */
void UA_NodeStore_deleteNode(UA_Node *node);

/**
 * Insert / Get / Replace / Remove
 * ------------------------------- */
/* Inserts a new node into the nodestore. If the nodeid is zero, then a fresh
 * numeric nodeid from namespace 1 is assigned. If insertion fails, the node is
 * deleted. */
UA_StatusCode UA_NodeStore_insert(UA_NodeStore *ns, UA_Node *node);

/* The returned node is immutable. */
const UA_Node * UA_NodeStore_get(UA_NodeStore *ns, const UA_NodeId *nodeid);

/* Returns an editable copy of a node (needs to be deleted with the deleteNode
   function or inserted / replaced into the nodestore). */
UA_Node * UA_NodeStore_getCopy(UA_NodeStore *ns, const UA_NodeId *nodeid);

/* To replace a node, get an editable copy of the node, edit and replace with
 * this function. If the node was already replaced since the copy was made,
 * UA_STATUSCODE_BADINTERNALERROR is returned. If the nodeid is not found,
 * UA_STATUSCODE_BADNODEIDUNKNOWN is returned. In both error cases, the editable
 * node is deleted. */
UA_StatusCode UA_NodeStore_replace(UA_NodeStore *ns, UA_Node *node);

/* Remove a node in the nodestore. */
UA_StatusCode UA_NodeStore_remove(UA_NodeStore *ns, const UA_NodeId *nodeid);

/**
 * Iteration
 * ---------
 * The following definitions are used to call a callback for every node in the
 * nodestore. */
typedef void (*UA_NodeStore_nodeVisitor)(const UA_Node *node);
void UA_NodeStore_iterate(UA_NodeStore *ns, UA_NodeStore_nodeVisitor visitor);

#ifdef __cplusplus
} // extern "C"
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_session_manager.h" ***********************************/



typedef struct session_list_entry {
    LIST_ENTRY(session_list_entry) pointers;
    UA_Session session;
} session_list_entry;

typedef struct UA_SessionManager {
    LIST_HEAD(session_list, session_list_entry) sessions; // doubly-linked list of sessions
    UA_UInt32 maxSessionCount;
    UA_UInt32 lastSessionId;
    UA_UInt32 currentSessionCount;
    UA_UInt32 maxSessionLifeTime;    // time in [ms]
    UA_Server *server;
} UA_SessionManager;

UA_StatusCode
UA_SessionManager_init(UA_SessionManager *sessionManager, UA_UInt32 maxSessionCount,
                       UA_UInt32 maxSessionLifeTime, UA_UInt32 startSessionId, UA_Server *server);

void UA_SessionManager_deleteMembers(UA_SessionManager *sessionManager);

void UA_SessionManager_cleanupTimedOut(UA_SessionManager *sessionManager, UA_DateTime now);

UA_StatusCode
UA_SessionManager_createSession(UA_SessionManager *sessionManager, UA_SecureChannel *channel,
                                const UA_CreateSessionRequest *request, UA_Session **session);

UA_StatusCode
UA_SessionManager_removeSession(UA_SessionManager *sessionManager, const UA_NodeId *token);

UA_Session *
UA_SessionManager_getSession(UA_SessionManager *sessionManager, const UA_NodeId *token);


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_securechannel_manager.h" ***********************************/



typedef struct channel_list_entry {
    UA_SecureChannel channel;
    LIST_ENTRY(channel_list_entry) pointers;
} channel_list_entry;

typedef struct UA_SecureChannelManager {
    LIST_HEAD(channel_list, channel_list_entry) channels; // doubly-linked list of channels
    size_t maxChannelCount;
    size_t currentChannelCount;
    UA_UInt32 maxChannelLifetime;
    UA_MessageSecurityMode securityMode;
    UA_DateTime channelLifeTime;
    UA_UInt32 lastChannelId;
    UA_UInt32 lastTokenId;
    UA_Server *server;
} UA_SecureChannelManager;

UA_StatusCode
UA_SecureChannelManager_init(UA_SecureChannelManager *cm, size_t maxChannelCount,
                             UA_UInt32 tokenLifetime, UA_UInt32 startChannelId,
                             UA_UInt32 startTokenId, UA_Server *server);

void UA_SecureChannelManager_deleteMembers(UA_SecureChannelManager *cm);

void UA_SecureChannelManager_cleanupTimedOut(UA_SecureChannelManager *cm, UA_DateTime now);

UA_StatusCode
UA_SecureChannelManager_open(UA_SecureChannelManager *cm, UA_Connection *conn,
                             const UA_OpenSecureChannelRequest *request,
                             UA_OpenSecureChannelResponse *response);

UA_StatusCode
UA_SecureChannelManager_renew(UA_SecureChannelManager *cm, UA_Connection *conn,
                              const UA_OpenSecureChannelRequest *request,
                              UA_OpenSecureChannelResponse *response);

UA_SecureChannel *
UA_SecureChannelManager_get(UA_SecureChannelManager *cm, UA_UInt32 channelId);

UA_StatusCode
UA_SecureChannelManager_close(UA_SecureChannelManager *cm, UA_UInt32 channelId);


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_server_internal.h" ***********************************/



#define ANONYMOUS_POLICY "open62541-anonymous-policy"
#define USERNAME_POLICY "open62541-username-policy"

#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
/** Mapping of namespace-id and url to an external nodestore. For namespaces
    that have no mapping defined, the internal nodestore is used by default. */
typedef struct UA_ExternalNamespace {
	UA_UInt16 index;
	UA_String url;
	UA_ExternalNodeStore externalNodeStore;
} UA_ExternalNamespace;
#endif

#ifdef UA_ENABLE_MULTITHREADING
typedef struct {
    UA_Server *server;
    pthread_t thr;
    UA_UInt32 counter;
    volatile UA_Boolean running;
    char padding[64 - sizeof(void*) - sizeof(pthread_t) -
                 sizeof(UA_UInt32) - sizeof(UA_Boolean)]; // separate cache lines
} UA_Worker;
#endif

struct UA_Server {
    /* Meta */
    UA_DateTime startTime;
    size_t endpointDescriptionsSize;
    UA_EndpointDescription *endpointDescriptions;

    /* Security */
    UA_SecureChannelManager secureChannelManager;
    UA_SessionManager sessionManager;

    /* Address Space */
    UA_NodeStore *nodestore;

    size_t namespacesSize;
    UA_String *namespaces;

#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
    size_t externalNamespacesSize;
    UA_ExternalNamespace *externalNamespaces;
#endif
     
    /* Jobs with a repetition interval */
    LIST_HEAD(RepeatedJobsList, RepeatedJobs) repeatedJobs;
    
#ifdef UA_ENABLE_MULTITHREADING
    /* Dispatch queue head for the worker threads (the tail should not be in the same cache line) */
	struct cds_wfcq_head dispatchQueue_head;
    UA_Worker *workers; /* there are nThread workers in a running server */
    struct cds_lfs_stack mainLoopJobs; /* Work that shall be executed only in the main loop and not
                                          by worker threads */
    struct DelayedJobs *delayedJobs;
    pthread_cond_t dispatchQueue_condition; /* so the workers don't spin if the queue is empty */
	struct cds_wfcq_tail dispatchQueue_tail; /* Dispatch queue tail for the worker threads */
#endif

    /* Config is the last element so that MSVC allows the usernamePasswordLogins
       field with zero-sized array */
    UA_ServerConfig config;
};

typedef UA_StatusCode (*UA_EditNodeCallback)(UA_Server*, UA_Session*, UA_Node*, const void*);

/* Calls callback on the node. In the multithreaded case, the node is copied before and replaced in
   the nodestore. */
UA_StatusCode UA_Server_editNode(UA_Server *server, UA_Session *session, const UA_NodeId *nodeId,
                                 UA_EditNodeCallback callback, const void *data);

void UA_Server_processBinaryMessage(UA_Server *server, UA_Connection *connection, const UA_ByteString *msg);

UA_StatusCode UA_Server_delayedCallback(UA_Server *server, UA_ServerCallback callback, void *data);
UA_StatusCode UA_Server_delayedFree(UA_Server *server, void *data);
void UA_Server_deleteAllRepeatedJobs(UA_Server *server);

#ifdef UA_BUILD_UNIT_TESTS
UA_StatusCode parse_numericrange(const UA_String *str, UA_NumericRange *range);
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_services.h" ***********************************/


#ifdef __cplusplus
extern "C" {
#endif


/**
 * .. _services:
 *
 * Services
 * ========
 * The services defined in the OPC UA standard. */
/* Most services take as input the server, the current session and pointers to
   the request and response. The possible error codes are returned as part of
   the response. */
typedef void (*UA_Service)(UA_Server*, UA_Session*, const void*, void*);

/**
 * Discovery Service Set
 * ---------------------
 * This Service Set defines Services used to discover the Endpoints implemented
 * by a Server and to read the security configuration for those Endpoints. */
void Service_FindServers(UA_Server *server, UA_Session *session,
                         const UA_FindServersRequest *request,
                         UA_FindServersResponse *response);

/* Returns the Endpoints supported by a Server and all of the configuration
 * information required to establish a SecureChannel and a Session. */
void Service_GetEndpoints(UA_Server *server, UA_Session *session,
                          const UA_GetEndpointsRequest *request,
                          UA_GetEndpointsResponse *response);

/* Not Implemented: Service_RegisterServer */

/**
 * SecureChannel Service Set
 * -------------------------
 * This Service Set defines Services used to open a communication channel that
 * ensures the confidentiality and Integrity of all Messages exchanged with the
 * Server. */

/* Open or renew a SecureChannel that can be used to ensure Confidentiality and
 * Integrity for Message exchange during a Session. */
void Service_OpenSecureChannel(UA_Server *server, UA_Connection *connection,
                               const UA_OpenSecureChannelRequest *request,
                               UA_OpenSecureChannelResponse *response);

/** Used to terminate a SecureChannel. */
void Service_CloseSecureChannel(UA_Server *server, UA_UInt32 channelId);

/**
 * Session Service Set
 * -------------------
 * This Service Set defines Services for an application layer connection
 * establishment in the context of a Session. */

/* Used by an OPC UA Client to create a Session and the Server returns two
 * values which uniquely identify the Session. The first value is the sessionId
 * which is used to identify the Session in the audit logs and in the Server's
 * address space. The second is the authenticationToken which is used to
 * associate an incoming request with a Session. */
void Service_CreateSession(UA_Server *server, UA_Session *session,
                           const UA_CreateSessionRequest *request,
                           UA_CreateSessionResponse *response);

/* Used by the Client to submit its SoftwareCertificates to the Server for
 * validation and to specify the identity of the user associated with the
 * Session. This Service request shall be issued by the Client before it issues
 * any other Service request after CreateSession. Failure to do so shall cause
 * the Server to close the Session. */
void Service_ActivateSession(UA_Server *server, UA_Session *session,
                             const UA_ActivateSessionRequest *request,
                             UA_ActivateSessionResponse *response);

/* Used to terminate a Session. */
void Service_CloseSession(UA_Server *server, UA_Session *session,
                          const UA_CloseSessionRequest *request,
                          UA_CloseSessionResponse *response);

/* Not Implemented: Service_Cancel */

/**
 * NodeManagement Service Set
 * --------------------------
 * This Service Set defines Services to add and delete AddressSpace Nodes and
 * References between them. All added Nodes continue to exist in the
 * AddressSpace even if the Client that created them disconnects from the
 * Server. */

/* Used to add one or more Nodes into the AddressSpace hierarchy. */
void Service_AddNodes(UA_Server *server, UA_Session *session,
                      const UA_AddNodesRequest *request,
                      UA_AddNodesResponse *response);

void Service_AddNodes_single(UA_Server *server, UA_Session *session,
                             const UA_AddNodesItem *item, UA_AddNodesResult *result,
                             UA_InstantiationCallback *instantiationCallback);

/* Add an existing node. The node is assumed to be "finished", i.e. no
 * instantiation from inheritance is necessary */
void Service_AddNodes_existing(UA_Server *server, UA_Session *session, UA_Node *node,
                               const UA_NodeId *parentNodeId, const UA_NodeId *referenceTypeId,
                               UA_AddNodesResult *result);

/* Used to add one or more References to one or more Nodes. */
void Service_AddReferences(UA_Server *server, UA_Session *session,
                           const UA_AddReferencesRequest *request,
                           UA_AddReferencesResponse *response);

UA_StatusCode Service_AddReferences_single(UA_Server *server, UA_Session *session,
                                           const UA_AddReferencesItem *item);

/* Used to delete one or more Nodes from the AddressSpace. */
void Service_DeleteNodes(UA_Server *server, UA_Session *session,
                         const UA_DeleteNodesRequest *request,
                         UA_DeleteNodesResponse *response);

UA_StatusCode Service_DeleteNodes_single(UA_Server *server, UA_Session *session,
                                         const UA_NodeId *nodeId,
                                         UA_Boolean deleteReferences);

/* Used to delete one or more References of a Node. */
void Service_DeleteReferences(UA_Server *server, UA_Session *session,
                              const UA_DeleteReferencesRequest *request,
                              UA_DeleteReferencesResponse *response);

UA_StatusCode Service_DeleteReferences_single(UA_Server *server, UA_Session *session,
                                              const UA_DeleteReferencesItem *item);

/**
 * View Service Set
 * ----------------
 * Clients use the browse Services of the View Service Set to navigate through
 * the AddressSpace or through a View which is a subset of the AddressSpace. */

/* Used to discover the References of a specified Node. The browse can be
 * further limited by the use of a View. This Browse Service also supports a
 * primitive filtering capability. */
void Service_Browse(UA_Server *server, UA_Session *session,
                    const UA_BrowseRequest *request,
                    UA_BrowseResponse *response);

void Service_Browse_single(UA_Server *server, UA_Session *session,
                           struct ContinuationPointEntry *cp, const UA_BrowseDescription *descr,
                           UA_UInt32 maxrefs, UA_BrowseResult *result);

/* Used to request the next set of Browse or BrowseNext response information
 * that is too large to be sent in a single response. "Too large" in this
 * context means that the Server is not able to return a larger response or that
 * the number of results to return exceeds the maximum number of results to
 * return that was specified by the Client in the original Browse request. */
void Service_BrowseNext(UA_Server *server, UA_Session *session,
                        const UA_BrowseNextRequest *request,
                        UA_BrowseNextResponse *response);

void UA_Server_browseNext_single(UA_Server *server, UA_Session *session,
                                 UA_Boolean releaseContinuationPoint,
                                 const UA_ByteString *continuationPoint,
                                 UA_BrowseResult *result);

/* Used to translate textual node paths to their respective ids. */
void Service_TranslateBrowsePathsToNodeIds(UA_Server *server, UA_Session *session,
                                           const UA_TranslateBrowsePathsToNodeIdsRequest *request,
                                           UA_TranslateBrowsePathsToNodeIdsResponse *response);

void Service_TranslateBrowsePathsToNodeIds_single(UA_Server *server, UA_Session *session,
                                                  const UA_BrowsePath *path,
                                                  UA_BrowsePathResult *result);

/* Used by Clients to register the Nodes that they know they will access
 * repeatedly (e.g. Write, Call). It allows Servers to set up anything needed so
 * that the access operations will be more efficient. */
void Service_RegisterNodes(UA_Server *server, UA_Session *session,
                           const UA_RegisterNodesRequest *request,
                           UA_RegisterNodesResponse *response);

/* This Service is used to unregister NodeIds that have been obtained via the
 * RegisterNodes service. */
void Service_UnregisterNodes(UA_Server *server, UA_Session *session,
                             const UA_UnregisterNodesRequest *request,
                             UA_UnregisterNodesResponse *response);
/**
 * Query Service Set
 * -----------------
 * This Service Set is used to issue a Query to a Server. OPC UA Query is
 * generic in that it provides an underlying storage mechanism independent Query
 * capability that can be used to access a wide variety of OPC UA data stores
 * and information management systems. OPC UA Query permits a Client to access
 * data maintained by a Server without any knowledge of the logical schema used
 * for internal storage of the data. Knowledge of the AddressSpace is
 * sufficient. */
/* Not Implemented: Service_QueryFirst */
/* Not Impelemented: Service_QueryNext */

/**
 * Attribute Service Set
 * ---------------------
 * This Service Set provides Services to access Attributes that are part of
 * Nodes. */

/* Used to read one or more Attributes of one or more Nodes. For constructed
 * Attribute values whose elements are indexed, such as an array, this Service
 * allows Clients to read the entire set of indexed values as a composite, to
 * read individual elements or to read ranges of elements of the composite. */
void Service_Read(UA_Server *server, UA_Session *session,
                  const UA_ReadRequest *request,
                  UA_ReadResponse *response);

void Service_Read_single(UA_Server *server, UA_Session *session,
                         UA_TimestampsToReturn timestamps,
                         const UA_ReadValueId *id, UA_DataValue *v);

/* Used to write one or more Attributes of one or more Nodes. For constructed
 * Attribute values whose elements are indexed, such as an array, this Service
 * allows Clients to write the entire set of indexed values as a composite, to
 * write individual elements or to write ranges of elements of the composite. */
void Service_Write(UA_Server *server, UA_Session *session,
                   const UA_WriteRequest *request,
                   UA_WriteResponse *response);

UA_StatusCode Service_Write_single(UA_Server *server, UA_Session *session,
                                   const UA_WriteValue *wvalue);

/* Not Implemented: Service_HistoryRead */
/* Not Implemented: Service_HistoryUpdate */

/**
 * Method Service Set
 * ------------------
 * The Method Service Set defines the means to invoke methods. A method shall be
 * a component of an Object. */
#ifdef UA_ENABLE_METHODCALLS
/* Used to call (invoke) a list of Methods. Each method call is invoked within
 * the context of an existing Session. If the Session is terminated, the results
 * of the method's execution cannot be returned to the Client and are
 * discarded. */
void Service_Call(UA_Server *server, UA_Session *session,
                  const UA_CallRequest *request,
                  UA_CallResponse *response);

void Service_Call_single(UA_Server *server, UA_Session *session,
                         const UA_CallMethodRequest *request,
                         UA_CallMethodResult *result);
#endif

/**
 * MonitoredItem Service Set
 * -------------------------
 * Clients define MonitoredItems to subscribe to data and Events. Each
 * MonitoredItem identifies the item to be monitored and the Subscription to use
 * to send Notifications. The item to be monitored may be any Node Attribute. */
#ifdef UA_ENABLE_SUBSCRIPTIONS

/* Used to create and add one or more MonitoredItems to a Subscription. A
 * MonitoredItem is deleted automatically by the Server when the Subscription is
 * deleted. Deleting a MonitoredItem causes its entire set of triggered item
 * links to be deleted, but has no effect on the MonitoredItems referenced by
 * the triggered items. */
void Service_CreateMonitoredItems(UA_Server *server, UA_Session *session,
                                  const UA_CreateMonitoredItemsRequest *request, 
                                  UA_CreateMonitoredItemsResponse *response);

/* Used to remove one or more MonitoredItems of a Subscription. When a
 * MonitoredItem is deleted, its triggered item links are also deleted. */
void Service_DeleteMonitoredItems(UA_Server *server, UA_Session *session,
                                  const UA_DeleteMonitoredItemsRequest *request,
                                  UA_DeleteMonitoredItemsResponse *response);

void Service_ModifyMonitoredItems(UA_Server *server, UA_Session *session,
                                  const UA_ModifyMonitoredItemsRequest *request,
                                  UA_ModifyMonitoredItemsResponse *response);
/* Not Implemented: Service_SetMonitoringMode */
/* Not Implemented: Service_SetTriggering */

#endif
                                      
/**
 * Subscription Service Set
 * ------------------------
 * Subscriptions are used to report Notifications to the Client. */
#ifdef UA_ENABLE_SUBSCRIPTIONS

/* Used to create a Subscription. Subscriptions monitor a set of MonitoredItems
 * for Notifications and return them to the Client in response to Publish
 * requests. */
void Service_CreateSubscription(UA_Server *server, UA_Session *session,
                                const UA_CreateSubscriptionRequest *request,
                                UA_CreateSubscriptionResponse *response);

/* Used to modify a Subscription. */
void Service_ModifySubscription(UA_Server *server, UA_Session *session,
                                const UA_ModifySubscriptionRequest *request,
                                UA_ModifySubscriptionResponse *response);

/* Used to enable sending of Notifications on one or more Subscriptions. */
void Service_SetPublishingMode(UA_Server *server, UA_Session *session,
	                           const UA_SetPublishingModeRequest *request,
	                           UA_SetPublishingModeResponse *response);

/* Used for two purposes. First, it is used to acknowledge the receipt of
 * NotificationMessages for one or more Subscriptions. Second, it is used to
 * request the Server to return a NotificationMessage or a keep-alive
 * Message.
 *
 * Note that the service signature is an exception and does not contain a
 * pointer to a PublishResponse. That is because the service queues up publish
 * requests internally and sends responses asynchronously based on timeouts. */
void Service_Publish(UA_Server *server, UA_Session *session,
                     const UA_PublishRequest *request, UA_UInt32 requestId);

/* Requests the Subscription to republish a NotificationMessage from its
 * retransmission queue. */
void Service_Republish(UA_Server *server, UA_Session *session,
                       const UA_RepublishRequest *request,
                       UA_RepublishResponse *response);

/* Invoked to delete one or more Subscriptions that belong to the Client's
 * Session. */
void Service_DeleteSubscriptions(UA_Server *server, UA_Session *session,
                                 const UA_DeleteSubscriptionsRequest *request,
                                 UA_DeleteSubscriptionsResponse *response);

/* Not Implemented: Service_TransferSubscription */

#endif

#ifdef __cplusplus
} // extern "C"
#endif


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/client/ua_client_internal.h" ***********************************/



/**************************/
/* Subscriptions Handling */
/**************************/

#ifdef UA_ENABLE_SUBSCRIPTIONS

typedef struct UA_Client_NotificationsAckNumber_s {
    UA_SubscriptionAcknowledgement subAck;
    LIST_ENTRY(UA_Client_NotificationsAckNumber_s) listEntry;
} UA_Client_NotificationsAckNumber;

typedef struct UA_Client_MonitoredItem_s {
    UA_UInt32  MonitoredItemId;
    UA_UInt32  MonitoringMode;
    UA_NodeId  monitoredNodeId;
    UA_UInt32  AttributeID;
    UA_UInt32  ClientHandle;
    UA_Double  SamplingInterval;
    UA_UInt32  QueueSize;
    UA_Boolean DiscardOldest;
    void       (*handler)(UA_UInt32 monId, UA_DataValue *value, void *context);
    void       *handlerContext;
    LIST_ENTRY(UA_Client_MonitoredItem_s)  listEntry;
} UA_Client_MonitoredItem;

typedef struct UA_Client_Subscription_s {
    UA_UInt32 LifeTime;
    UA_UInt32 KeepAliveCount;
    UA_Double PublishingInterval;
    UA_UInt32 SubscriptionID;
    UA_UInt32 NotificationsPerPublish;
    UA_UInt32 Priority;
    LIST_ENTRY(UA_Client_Subscription_s) listEntry;
    LIST_HEAD(UA_ListOfClientMonitoredItems, UA_Client_MonitoredItem_s) MonitoredItems;
} UA_Client_Subscription;

#endif

/**********/
/* Client */
/**********/

typedef enum {
    UA_CLIENTAUTHENTICATION_NONE,
    UA_CLIENTAUTHENTICATION_USERNAME
} UA_Client_Authentication;

struct UA_Client {
    /* State */
    UA_ClientState state;

    /* Connection */
    UA_Connection connection;
    UA_SecureChannel channel;
    UA_String endpointUrl;
    UA_UInt32 requestId;

    /* Authentication */
    UA_Client_Authentication authenticationMethod;
    UA_String username;
    UA_String password;

    /* Session */
    UA_UserTokenPolicy token;
    UA_NodeId sessionId;
    UA_NodeId authenticationToken;
    UA_UInt32 requestHandle;
    
#ifdef UA_ENABLE_SUBSCRIPTIONS
    UA_UInt32 monitoredItemHandles;
    LIST_HEAD(UA_ListOfUnacknowledgedNotificationNumbers, UA_Client_NotificationsAckNumber_s) pendingNotificationsAcks;
    LIST_HEAD(UA_ListOfClientSubscriptionItems, UA_Client_Subscription_s) subscriptions;
#endif
    
    /* Config */
    UA_ClientConfig config;
    UA_DateTime scRenewAt;
};


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_nodestore_hash.inc" ***********************************/

typedef UA_UInt32 hash_t;

static hash_t mod(hash_t h, hash_t size) { return h % size; }
static hash_t mod2(hash_t h, hash_t size) { return 1 + (h % (size - 2)); }

/* Based on Murmur-Hash 3 by Austin Appleby (public domain, freely usable) */
static hash_t hash_array(const UA_Byte *data, UA_UInt32 len, UA_UInt32 seed) {
    if(data == NULL)
        return 0;

    const int32_t nblocks = (int32_t)(len / 4);
    const uint32_t *blocks;
    static const uint32_t c1 = 0xcc9e2d51;
    static const uint32_t c2 = 0x1b873593;
    static const uint32_t r1 = 15;
    static const uint32_t r2 = 13;
    static const uint32_t m  = 5;
    static const uint32_t n  = 0xe6546b64;
    hash_t hash = seed;
    /* Somce compilers emit a warning when casting from a byte array to ints. */
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4 || defined(__clang__))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif
    blocks = (const uint32_t *)data;
#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4 || defined(__clang__))
#pragma GCC diagnostic pop
#endif
    for(int32_t i = 0;i < nblocks;i++) {
        uint32_t k = blocks[i];
        k    *= c1;
        k     = (k << r1) | (k >> (32 - r1));
        k    *= c2;
        hash ^= k;
        hash  = ((hash << r2) | (hash >> (32 - r2))) * m + n;
    }

    const uint8_t *tail = (const uint8_t *)(data + nblocks * 4);
    uint32_t k1 = 0;
    switch(len & 3) {
    case 3:
        k1 ^= (uint32_t)(tail[2] << 16);
    case 2:
        k1 ^= (uint32_t)(tail[1] << 8);
    case 1:
        k1   ^= tail[0];
        k1   *= c1;
        k1    = (k1 << r1) | (k1 >> (32 - r1));
        k1   *= c2;
        hash ^= k1;
    }

    hash ^= len;
    hash ^= (hash >> 16);
    hash *= 0x85ebca6b;
    hash ^= (hash >> 13);
    hash *= 0xc2b2ae35;
    hash ^= (hash >> 16);
    return hash;
}

static hash_t hash(const UA_NodeId *n) {
    switch(n->identifierType) {
    case UA_NODEIDTYPE_NUMERIC:
        /*  Knuth's multiplicative hashing */
        return (hash_t)((n->identifier.numeric + n->namespaceIndex) * 2654435761); // mod(2^32) is implicit
    case UA_NODEIDTYPE_STRING:
        return hash_array(n->identifier.string.data, (UA_UInt32)n->identifier.string.length,
                          n->namespaceIndex);
    case UA_NODEIDTYPE_GUID:
        return hash_array((const UA_Byte*)&(n->identifier.guid), sizeof(UA_Guid), n->namespaceIndex);
    case UA_NODEIDTYPE_BYTESTRING:
        return hash_array((const UA_Byte*)n->identifier.byteString.data,
                          (UA_UInt32)n->identifier.byteString.length, n->namespaceIndex);
    default:
        UA_assert(false);
        return 0;
    }
}


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/ua_types.c" ***********************************/



/* static variables */
UA_EXPORT const UA_String UA_STRING_NULL = {.length = 0, .data = NULL };
UA_EXPORT const UA_ByteString UA_BYTESTRING_NULL = {.length = 0, .data = NULL };
UA_EXPORT const UA_NodeId UA_NODEID_NULL = {0, UA_NODEIDTYPE_NUMERIC, {0}};
UA_EXPORT const UA_ExpandedNodeId UA_EXPANDEDNODEID_NULL = {
    .nodeId = { .namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 0 },
    .namespaceUri = {.length = 0, .data = NULL}, .serverIndex = 0 };

/***************************/
/* Random Number Generator */
/***************************/
static UA_THREAD_LOCAL pcg32_random_t UA_rng = PCG32_INITIALIZER;

UA_EXPORT void UA_random_seed(UA_UInt64 seed) {
    pcg32_srandom_r(&UA_rng, seed, (uint64_t)UA_DateTime_now());
}

/*****************/
/* Builtin Types */
/*****************/
UA_EXPORT UA_UInt32 UA_UInt32_random(void) {
    return (UA_UInt32)pcg32_random_r(&UA_rng);
}

UA_String UA_String_fromChars(char const src[]) {
    UA_String str = UA_STRING_NULL;
    size_t length = strlen(src);
    if(length > 0) {
        str.data = UA_malloc(length);
        if(!str.data)
            return str;
    } else
        str.data = UA_EMPTY_ARRAY_SENTINEL;
    memcpy(str.data, src, length);
    str.length = length;
    return str;
}

UA_Boolean UA_String_equal(const UA_String *string1, const UA_String *string2) {
    if(string1->length != string2->length)
        return false;
    UA_Int32 is = memcmp((char const*)string1->data, (char const*)string2->data, string1->length);
    return (is == 0) ? true : false;
}

/* DateTime */
UA_DateTime UA_DateTime_now(void) {
#ifdef _WIN32
    /* Windows filetime has the same definition as UA_DateTime */
    FILETIME ft;
    SYSTEMTIME st;
    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &ft);
    ULARGE_INTEGER ul;
    ul.LowPart = ft.dwLowDateTime;
    ul.HighPart = ft.dwHighDateTime;
    return (UA_DateTime)ul.QuadPart;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * UA_SEC_TO_DATETIME) + (tv.tv_usec * UA_USEC_TO_DATETIME) + UA_DATETIME_UNIX_EPOCH;
#endif
}

UA_DateTime UA_DateTime_nowMonotonic(void) {
#ifdef _WIN32
    LARGE_INTEGER freq, ticks;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&ticks);
    UA_Double ticks2dt = UA_SEC_TO_DATETIME;
    ticks2dt /= freq.QuadPart;
    return (UA_DateTime)(ticks.QuadPart * ticks2dt);
#elif defined(__APPLE__) || defined(__MACH__) // OS X does not have clock_gettime, use clock_get_time
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    return (mts.tv_sec * UA_SEC_TO_DATETIME) + (mts.tv_nsec / 100);
#else
    struct timespec ts;
#ifdef __CYGWIN__    
    clock_gettime(CLOCK_MONOTONIC, &ts);
#else
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#endif
    return (ts.tv_sec * UA_SEC_TO_DATETIME) + (ts.tv_nsec / 100);
#endif
}

UA_DateTimeStruct UA_DateTime_toStruct(UA_DateTime t) {
    /* Calculating the the milli-, micro- and nanoseconds */
    UA_DateTimeStruct dateTimeStruct;
    dateTimeStruct.nanoSec  = (UA_UInt16)((t % 10) * 100);
    dateTimeStruct.microSec = (UA_UInt16)((t % 10000) / 10);
    dateTimeStruct.milliSec = (UA_UInt16)((t % 10000000) / 10000);

    /* Calculating the unix time with #include <time.h> */
    time_t secSinceUnixEpoch = (time_t)((t - UA_DATETIME_UNIX_EPOCH) / UA_SEC_TO_DATETIME);
    struct tm ts;
    memset(&ts, 0, sizeof(struct tm));
    __secs_to_tm(secSinceUnixEpoch, &ts);
    dateTimeStruct.sec    = (UA_UInt16)ts.tm_sec;
    dateTimeStruct.min    = (UA_UInt16)ts.tm_min;
    dateTimeStruct.hour   = (UA_UInt16)ts.tm_hour;
    dateTimeStruct.day    = (UA_UInt16)ts.tm_mday;
    dateTimeStruct.month  = (UA_UInt16)(ts.tm_mon + 1);
    dateTimeStruct.year   = (UA_UInt16)(ts.tm_year + 1900);
    return dateTimeStruct;
}

static void printNumber(UA_UInt16 n, UA_Byte *pos, size_t digits) {
    for(size_t i = digits; i > 0; i--) {
        pos[i-1] = (UA_Byte)((n % 10) + '0');
        n = n / 10;
    }
}

UA_String UA_DateTime_toString(UA_DateTime t) {
    UA_String str = UA_STRING_NULL;
    // length of the string is 31 (plus \0 at the end)
    if(!(str.data = UA_malloc(32)))
        return str;
    str.length = 31;
    UA_DateTimeStruct tSt = UA_DateTime_toStruct(t);
    printNumber(tSt.month, str.data, 2);
    str.data[2] = '/';
    printNumber(tSt.day, &str.data[3], 2);
    str.data[5] = '/';
    printNumber(tSt.year, &str.data[6], 4);
    str.data[10] = ' ';
    printNumber(tSt.hour, &str.data[11], 2);
    str.data[13] = ':';
    printNumber(tSt.min, &str.data[14], 2);
    str.data[16] = ':';
    printNumber(tSt.sec, &str.data[17], 2);
    str.data[19] = '.';
    printNumber(tSt.milliSec, &str.data[20], 3);
    str.data[23] = '.';
    printNumber(tSt.microSec, &str.data[24], 3);
    str.data[27] = '.';
    printNumber(tSt.nanoSec, &str.data[28], 3);
    return str;
}

/* Guid */
UA_Boolean UA_Guid_equal(const UA_Guid *g1, const UA_Guid *g2) {
    if(memcmp(g1, g2, sizeof(UA_Guid)) == 0)
        return true;
    return false;
}

UA_Guid UA_Guid_random(void) {
    UA_Guid result;
    result.data1 = (UA_UInt32)pcg32_random_r(&UA_rng);
    UA_UInt32 r = (UA_UInt32)pcg32_random_r(&UA_rng);
    result.data2 = (UA_UInt16) r;
    result.data3 = (UA_UInt16) (r >> 16);
    r = (UA_UInt32)pcg32_random_r(&UA_rng);
    result.data4[0] = (UA_Byte)r;
    result.data4[1] = (UA_Byte)(r >> 4);
    result.data4[2] = (UA_Byte)(r >> 8);
    result.data4[3] = (UA_Byte)(r >> 12);
    r = (UA_UInt32)pcg32_random_r(&UA_rng);
    result.data4[4] = (UA_Byte)r;
    result.data4[5] = (UA_Byte)(r >> 4);
    result.data4[6] = (UA_Byte)(r >> 8);
    result.data4[7] = (UA_Byte)(r >> 12);
    return result;
}

/* ByteString */
UA_StatusCode UA_ByteString_allocBuffer(UA_ByteString *bs, size_t length) {
    if(!(bs->data = UA_malloc(length)))
        return UA_STATUSCODE_BADOUTOFMEMORY;
    bs->length = length;
    return UA_STATUSCODE_GOOD;
}

/* NodeId */
static void NodeId_deleteMembers(UA_NodeId *p, const UA_DataType *_) {
    switch(p->identifierType) {
    case UA_NODEIDTYPE_STRING:
    case UA_NODEIDTYPE_BYTESTRING:
        UA_free((void*)((uintptr_t)p->identifier.byteString.data & ~(uintptr_t)UA_EMPTY_ARRAY_SENTINEL));
        p->identifier.byteString = UA_BYTESTRING_NULL;
        break;
    default: break;
    }
}

static UA_StatusCode NodeId_copy(UA_NodeId const *src, UA_NodeId *dst, const UA_DataType *_) {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    switch(src->identifierType) {
    case UA_NODEIDTYPE_NUMERIC:
        *dst = *src;
        return UA_STATUSCODE_GOOD;
    case UA_NODEIDTYPE_STRING:
        retval |= UA_String_copy(&src->identifier.string, &dst->identifier.string);
        break;
    case UA_NODEIDTYPE_GUID:
        retval |= UA_Guid_copy(&src->identifier.guid, &dst->identifier.guid);
        break;
    case UA_NODEIDTYPE_BYTESTRING:
        retval |= UA_ByteString_copy(&src->identifier.byteString, &dst->identifier.byteString);
        break;
    default:
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    dst->namespaceIndex = src->namespaceIndex;
    dst->identifierType = src->identifierType;
    if(retval != UA_STATUSCODE_GOOD)
        NodeId_deleteMembers(dst, NULL);
    return retval;
}

UA_Boolean UA_NodeId_equal(const UA_NodeId *n1, const UA_NodeId *n2) {
	if(n1->namespaceIndex != n2->namespaceIndex || n1->identifierType!=n2->identifierType)
        return false;
    switch(n1->identifierType) {
    case UA_NODEIDTYPE_NUMERIC:
        if(n1->identifier.numeric == n2->identifier.numeric)
            return true;
        else
            return false;
    case UA_NODEIDTYPE_STRING:
        return UA_String_equal(&n1->identifier.string, &n2->identifier.string);
    case UA_NODEIDTYPE_GUID:
        return UA_Guid_equal(&n1->identifier.guid, &n2->identifier.guid);
    case UA_NODEIDTYPE_BYTESTRING:
        return UA_ByteString_equal(&n1->identifier.byteString, &n2->identifier.byteString);
    }
    return false;
}

/* ExpandedNodeId */
static void ExpandedNodeId_deleteMembers(UA_ExpandedNodeId *p, const UA_DataType *_) {
    NodeId_deleteMembers(&p->nodeId, _);
    UA_String_deleteMembers(&p->namespaceUri);
}

static UA_StatusCode
ExpandedNodeId_copy(UA_ExpandedNodeId const *src, UA_ExpandedNodeId *dst, const UA_DataType *_) {
    UA_StatusCode retval = NodeId_copy(&src->nodeId, &dst->nodeId, NULL);
    retval |= UA_String_copy(&src->namespaceUri, &dst->namespaceUri);
    dst->serverIndex = src->serverIndex;
    if(retval != UA_STATUSCODE_GOOD)
        ExpandedNodeId_deleteMembers(dst, NULL);
    return retval;
}

/* ExtensionObject */
static void ExtensionObject_deleteMembers(UA_ExtensionObject *p, const UA_DataType *_) {
    switch(p->encoding) {
    case UA_EXTENSIONOBJECT_ENCODED_NOBODY:
    case UA_EXTENSIONOBJECT_ENCODED_BYTESTRING:
    case UA_EXTENSIONOBJECT_ENCODED_XML:
        NodeId_deleteMembers(&p->content.encoded.typeId, NULL);
        UA_free((void*)((uintptr_t)p->content.encoded.body.data & ~(uintptr_t)UA_EMPTY_ARRAY_SENTINEL));
        p->content.encoded.body = UA_BYTESTRING_NULL;
        break;
    case UA_EXTENSIONOBJECT_DECODED:
        if(!p->content.decoded.data)
            break;
        UA_delete(p->content.decoded.data, p->content.decoded.type);
        p->content.decoded.data = NULL;
        p->content.decoded.type = NULL;
        break;
    case UA_EXTENSIONOBJECT_DECODED_NODELETE:
        p->content.decoded.type = NULL;
    default:
        break;
    }
}

static UA_StatusCode
ExtensionObject_copy(UA_ExtensionObject const *src, UA_ExtensionObject *dst, const UA_DataType *_) {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    switch(src->encoding) {
    case UA_EXTENSIONOBJECT_ENCODED_NOBODY:
    case UA_EXTENSIONOBJECT_ENCODED_BYTESTRING:
    case UA_EXTENSIONOBJECT_ENCODED_XML:
        dst->encoding = src->encoding;
        retval = NodeId_copy(&src->content.encoded.typeId, &dst->content.encoded.typeId, NULL);
        retval |= UA_ByteString_copy(&src->content.encoded.body, &dst->content.encoded.body);
        break;
    case UA_EXTENSIONOBJECT_DECODED:
    case UA_EXTENSIONOBJECT_DECODED_NODELETE:
        if(!src->content.decoded.type || !src->content.decoded.data)
            return UA_STATUSCODE_BADINTERNALERROR;
        dst->encoding = UA_EXTENSIONOBJECT_DECODED;
        dst->content.decoded.type = src->content.decoded.type;
        retval = UA_Array_copy(src->content.decoded.data, 1,
            &dst->content.decoded.data, src->content.decoded.type);
        break;
    default:
        break;
    }
    return retval;
}

/* Variant */
static void Variant_deletemembers(UA_Variant *p, const UA_DataType *_) {
    if(p->storageType != UA_VARIANT_DATA)
        return;
    if(p->data > UA_EMPTY_ARRAY_SENTINEL) {
        if(p->arrayLength == 0)
            p->arrayLength = 1;
        UA_Array_delete(p->data, p->arrayLength, p->type);
        p->data = NULL;
        p->arrayLength = 0;
    }
    if(p->arrayDimensions) {
        UA_Array_delete(p->arrayDimensions, p->arrayDimensionsSize, &UA_TYPES[UA_TYPES_INT32]);
        p->arrayDimensions = NULL;
        p->arrayDimensionsSize = 0;
    }
}

static UA_StatusCode
Variant_copy(UA_Variant const *src, UA_Variant *dst, const UA_DataType *_) {
    size_t length = src->arrayLength;
    if(UA_Variant_isScalar(src))
        length = 1;
    UA_StatusCode retval = UA_Array_copy(src->data, length, &dst->data, src->type);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    dst->arrayLength = src->arrayLength;
    dst->type = src->type;
    if(src->arrayDimensions) {
        retval = UA_Array_copy(src->arrayDimensions, src->arrayDimensionsSize,
            (void**)&dst->arrayDimensions, &UA_TYPES[UA_TYPES_INT32]);
        if(retval == UA_STATUSCODE_GOOD)
            dst->arrayDimensionsSize = src->arrayDimensionsSize;
        else
            Variant_deletemembers(dst, NULL);
    }
    return retval;
}

/**
 * Test if a range is compatible with a variant. If yes, the following values are set:
 * - total: how many elements are in the range
 * - block: how big is each contiguous block of elements in the variant that maps into the range
 * - stride: how many elements are between the blocks (beginning to beginning)
 * - first: where does the first block begin
 */
static UA_StatusCode
processRangeDefinition(const UA_Variant *v, const UA_NumericRange range, size_t *total,
                       size_t *block, size_t *stride, size_t *first) {
    /* Test the integrity of the source variant dimensions */
    size_t dims_count = 1;
    UA_UInt32 elements = 1;
#if(MAX_SIZE > 0xffffffff) /* 64bit only */
    if(v->arrayLength > UA_UINT32_MAX)
        return UA_STATUSCODE_BADINTERNALERROR;
#endif
    UA_UInt32 arrayLength = (UA_UInt32)v->arrayLength;
    const UA_UInt32 *dims = &arrayLength;
    if(v->arrayDimensionsSize > 0) {
        dims_count = v->arrayDimensionsSize;
        dims = (UA_UInt32*)v->arrayDimensions;
        for(size_t i = 0; i < dims_count; i++) {
            /* dimensions can have negative size similar to array lengths */
            if(v->arrayDimensions[i] < 0)
                return UA_STATUSCODE_BADINDEXRANGEINVALID;
            elements *= dims[i];
        }
        if(elements != v->arrayLength)
            return UA_STATUSCODE_BADINTERNALERROR;
    }

    /* Test the integrity of the range */
    size_t count = 1;
    if(range.dimensionsSize != dims_count)
        return UA_STATUSCODE_BADINDEXRANGEINVALID;
    for(size_t i = 0; i < dims_count; i++) {
        if(range.dimensions[i].min > range.dimensions[i].max)
            return UA_STATUSCODE_BADINDEXRANGENODATA;
        if(range.dimensions[i].max >= dims[i])
            return UA_STATUSCODE_BADINDEXRANGEINVALID;
        count *= (range.dimensions[i].max - range.dimensions[i].min) + 1;
    }

    /* Compute the stride length and the position of the first element */
    size_t b = 1, s = elements, f = 0;
    size_t running_dimssize = 1;
    UA_Boolean found_contiguous = false;
    for(size_t k = dims_count - 1; ; k--) {
        if(!found_contiguous && (range.dimensions[k].min != 0 || range.dimensions[k].max + 1 != dims[k])) {
            found_contiguous = true;
            b = (range.dimensions[k].max - range.dimensions[k].min + 1) * running_dimssize;
            s = dims[k] * running_dimssize;
        } 
        f += running_dimssize * range.dimensions[k].min;
        running_dimssize *= dims[k];
        if(k == 0)
            break;
    }
    *total = count;
    *block = b;
    *stride = s;
    *first = f;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
UA_Variant_copyRange(const UA_Variant *src, UA_Variant *dst, const UA_NumericRange range) {
    size_t count, block, stride, first;
    UA_StatusCode retval = processRangeDefinition(src, range, &count, &block, &stride, &first);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    UA_Variant_init(dst);
    size_t elem_size = src->type->memSize;
    dst->data = UA_malloc(elem_size * count);
    if(!dst->data)
        return UA_STATUSCODE_BADOUTOFMEMORY;

    /* Copy the range */
    size_t block_count = count / block;
    uintptr_t nextdst = (uintptr_t)dst->data;
    uintptr_t nextsrc = (uintptr_t)src->data + (elem_size * first);
    if(src->type->fixedSize) {
        for(size_t i = 0; i < block_count; i++) {
            memcpy((void*)nextdst, (void*)nextsrc, elem_size * block);
            nextdst += block * elem_size;
            nextsrc += stride * elem_size;
        }
    } else {
        for(size_t i = 0; i < block_count; i++) {
            for(size_t j = 0; j < block && retval == UA_STATUSCODE_GOOD; j++) {
                retval = UA_copy((const void*)nextsrc, (void*)nextdst, src->type);
                nextdst += elem_size;
                nextsrc += elem_size;
            }
            nextsrc += (stride - block) * elem_size;
        }
        if(retval != UA_STATUSCODE_GOOD) {
            size_t copied = ((nextdst - elem_size) - (uintptr_t)dst->data) / elem_size;
            UA_Array_delete(dst->data, copied, src->type);
            dst->data = NULL;
            return retval;
        }
    }
    dst->arrayLength = count;
    dst->type = src->type;

    /* Copy the range dimensions */
    if(src->arrayDimensionsSize > 0) {
        dst->arrayDimensions = UA_Array_new(src->arrayDimensionsSize, &UA_TYPES[UA_TYPES_UINT32]);
        if(!dst->arrayDimensions) {
            Variant_deletemembers(dst, NULL);
            return UA_STATUSCODE_BADOUTOFMEMORY;
        }
        dst->arrayDimensionsSize = src->arrayDimensionsSize;
        for(size_t k = 0; k < src->arrayDimensionsSize; k++)
            dst->arrayDimensions[k] = (UA_Int32)(range.dimensions[k].max - range.dimensions[k].min + 1);
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
UA_Variant_setRange(UA_Variant *v, void * UA_RESTRICT array, size_t arraySize, const UA_NumericRange range) {
    size_t count, block, stride, first;
    UA_StatusCode retval = processRangeDefinition(v, range, &count, &block, &stride, &first);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    if(count != arraySize)
        return UA_STATUSCODE_BADINDEXRANGEINVALID;

    size_t block_count = count / block;
    size_t elem_size = v->type->memSize;
    uintptr_t nextdst = (uintptr_t)v->data + (first * elem_size);
    uintptr_t nextsrc = (uintptr_t)array;
    for(size_t i = 0; i < block_count; i++) {
        if(!v->type->fixedSize) {
            for(size_t j = 0; j < block; j++) {
                UA_deleteMembers((void*)nextdst, v->type);
                nextdst += elem_size;
            }
            nextdst -= block * elem_size;
        }
        memcpy((void*)nextdst, (void*)nextsrc, elem_size * block);
        nextsrc += block * elem_size;
        nextdst += stride * elem_size;
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
UA_Variant_setRangeCopy(UA_Variant *v, const void *array, size_t arraySize, const UA_NumericRange range) {
    size_t count, block, stride, first;
    UA_StatusCode retval = processRangeDefinition(v, range, &count, &block, &stride, &first);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    if(count != arraySize)
        return UA_STATUSCODE_BADINDEXRANGEINVALID;

    size_t block_count = count / block;
    size_t elem_size = v->type->memSize;
    uintptr_t nextdst = (uintptr_t)v->data + (first * elem_size);
    uintptr_t nextsrc = (uintptr_t)array;
    if(v->type->fixedSize) {
        for(size_t i = 0; i < block_count; i++) {
            memcpy((void*)nextdst, (void*)nextsrc, elem_size * block);
            nextsrc += block * elem_size;
            nextdst += stride * elem_size;
        }
    } else {
        for(size_t i = 0; i < block_count; i++) {
            for(size_t j = 0; j < block; j++) {
                UA_deleteMembers((void*)nextdst, v->type);
                retval |= UA_copy((void*)nextsrc, (void*)nextdst, v->type);
                nextdst += elem_size;
                nextsrc += elem_size;
            }
            nextdst += (stride - block) * elem_size;
        }
    }
    return retval;
}

void UA_Variant_setScalar(UA_Variant *v, void * UA_RESTRICT p, const UA_DataType *type) {
    UA_Variant_init(v);
    v->type = type;
    v->arrayLength = 0;
    v->data = p;
}

UA_StatusCode UA_Variant_setScalarCopy(UA_Variant *v, const void *p, const UA_DataType *type) {
    void *new = UA_malloc(type->memSize);
    if(!new)
        return UA_STATUSCODE_BADOUTOFMEMORY;
    UA_StatusCode retval = UA_copy(p, new, type);
	if(retval != UA_STATUSCODE_GOOD) {
		UA_free(new);
        //cppcheck-suppress memleak
		return retval;
	}
    UA_Variant_setScalar(v, new, type);
    //cppcheck-suppress memleak
    return UA_STATUSCODE_GOOD;
}

void
UA_Variant_setArray(UA_Variant *v, void * UA_RESTRICT array,
                    size_t arraySize, const UA_DataType *type) {
    UA_Variant_init(v);
    v->data = array;
    v->arrayLength = arraySize;
    v->type = type;
}

UA_StatusCode
UA_Variant_setArrayCopy(UA_Variant *v, const void *array,
                        size_t arraySize, const UA_DataType *type) {
    UA_Variant_init(v);
    UA_StatusCode retval = UA_Array_copy(array, arraySize, &v->data, type);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    v->arrayLength = arraySize; 
    v->type = type;
    return UA_STATUSCODE_GOOD;
}

/* LocalizedText */
static void LocalizedText_deleteMembers(UA_LocalizedText *p, const UA_DataType *_) {
    UA_String_deleteMembers(&p->locale);
    UA_String_deleteMembers(&p->text);
}

static UA_StatusCode
LocalizedText_copy(UA_LocalizedText const *src, UA_LocalizedText *dst, const UA_DataType *_) {
    UA_StatusCode retval = UA_String_copy(&src->locale, &dst->locale);
    retval |= UA_String_copy(&src->text, &dst->text);
    return retval;
}

/* DataValue */
static void DataValue_deleteMembers(UA_DataValue *p, const UA_DataType *_) {
    Variant_deletemembers(&p->value, NULL);
}

static UA_StatusCode
DataValue_copy(UA_DataValue const *src, UA_DataValue *dst, const UA_DataType *_) {
    memcpy(dst, src, sizeof(UA_DataValue));
    UA_Variant_init(&dst->value);
    UA_StatusCode retval = Variant_copy(&src->value, &dst->value, NULL);
    if(retval != UA_STATUSCODE_GOOD)
        DataValue_deleteMembers(dst, NULL);
    return retval;
}

/* DiagnosticInfo */
static void DiagnosticInfo_deleteMembers(UA_DiagnosticInfo *p, const UA_DataType *_) {
    UA_String_deleteMembers(&p->additionalInfo);
    if(p->hasInnerDiagnosticInfo && p->innerDiagnosticInfo) {
        DiagnosticInfo_deleteMembers(p->innerDiagnosticInfo, NULL);
        UA_free(p->innerDiagnosticInfo);
        p->innerDiagnosticInfo = NULL;
        p->hasInnerDiagnosticInfo = false;
    }
}

static UA_StatusCode
DiagnosticInfo_copy(UA_DiagnosticInfo const *src, UA_DiagnosticInfo *dst, const UA_DataType *_) {
    memcpy(dst, src, sizeof(UA_DiagnosticInfo));
    UA_String_init(&dst->additionalInfo);
    dst->innerDiagnosticInfo = NULL;
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    if(src->hasAdditionalInfo)
       retval = UA_String_copy(&src->additionalInfo, &dst->additionalInfo);
    if(src->hasInnerDiagnosticInfo && src->innerDiagnosticInfo) {
        if((dst->innerDiagnosticInfo = UA_malloc(sizeof(UA_DiagnosticInfo)))) {
            retval |= DiagnosticInfo_copy(src->innerDiagnosticInfo, dst->innerDiagnosticInfo, NULL);
            dst->hasInnerDiagnosticInfo = true;
        } else {
            dst->hasInnerDiagnosticInfo = false;
            retval |= UA_STATUSCODE_BADOUTOFMEMORY;
        }
    }
    if(retval != UA_STATUSCODE_GOOD)
        DiagnosticInfo_deleteMembers(dst, NULL);
    return retval;
}

/*******************/
/* Structure Types */
/*******************/

void * UA_new(const UA_DataType *type) {
    void *p = UA_calloc(1, type->memSize);
    return p;
}

static UA_StatusCode copyByte(const void *src, void *dst, const UA_DataType *_) {
    memcpy(dst, src, sizeof(UA_Byte));
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode copy2Byte(const void *src, void *dst, const UA_DataType *_) {
    memcpy(dst, src, sizeof(UA_UInt16));
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode copy4Byte(const void *src, void *dst, const UA_DataType *_) {
    memcpy(dst, src, sizeof(UA_UInt32));
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode copy8Byte(const void *src, void *dst, const UA_DataType *_) {
    memcpy(dst, src, sizeof(UA_UInt64));
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode copyFixedSize(const void *src, void *dst, const UA_DataType *type) {
    memcpy(dst, src, type->memSize);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode copyNoInit(const void *src, void *dst, const UA_DataType *type);

typedef UA_StatusCode (*UA_copySignature)(const void *src, void *dst, const UA_DataType *type);
static const UA_copySignature copyJumpTable[UA_BUILTIN_TYPES_COUNT + 1] = {
    (UA_copySignature)copyByte, // Boolean
    (UA_copySignature)copyByte, // SByte
    (UA_copySignature)copyByte, // Byte
    (UA_copySignature)copy2Byte, // Int16
    (UA_copySignature)copy2Byte, // UInt16 
    (UA_copySignature)copy4Byte, // Int32 
    (UA_copySignature)copy4Byte, // UInt32 
    (UA_copySignature)copy8Byte, // Int64
    (UA_copySignature)copy8Byte, // UInt64 
    (UA_copySignature)copy4Byte, // Float 
    (UA_copySignature)copy8Byte, // Double 
    (UA_copySignature)copyNoInit, // String
    (UA_copySignature)copy8Byte, // DateTime
    (UA_copySignature)copyFixedSize, // Guid 
    (UA_copySignature)copyNoInit, // ByteString
    (UA_copySignature)copyNoInit, // XmlElement
    (UA_copySignature)NodeId_copy,
    (UA_copySignature)ExpandedNodeId_copy,
    (UA_copySignature)copy4Byte, // StatusCode
    (UA_copySignature)copyNoInit, // QualifiedName
    (UA_copySignature)LocalizedText_copy, // LocalizedText
    (UA_copySignature)ExtensionObject_copy,
    (UA_copySignature)DataValue_copy,
    (UA_copySignature)Variant_copy,
    (UA_copySignature)DiagnosticInfo_copy,
    (UA_copySignature)copyNoInit // all others
};

static UA_StatusCode copyNoInit(const void *src, void *dst, const UA_DataType *type) {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    uintptr_t ptrs = (uintptr_t)src;
    uintptr_t ptrd = (uintptr_t)dst;
    UA_Byte membersSize = type->membersSize;
    for(size_t i = 0; i < membersSize; i++) {
        const UA_DataTypeMember *member = &type->members[i];
        const UA_DataType *typelists[2] = { UA_TYPES, &type[-type->typeIndex] };
        const UA_DataType *memberType = &typelists[!member->namespaceZero][member->memberTypeIndex];
        if(!member->isArray) {
            ptrs += member->padding;
            ptrd += member->padding;
            size_t fi = memberType->builtin ? memberType->typeIndex : UA_BUILTIN_TYPES_COUNT;
            retval |= copyJumpTable[fi]((const void*)ptrs, (void*)ptrd, memberType);
            ptrs += memberType->memSize;
            ptrd += memberType->memSize;
        } else {
            ptrs += member->padding;
            ptrd += member->padding;
            size_t *dst_size = (size_t*)ptrd;
            const size_t size = *((const size_t*)ptrs);
            ptrs += sizeof(size_t);
            ptrd += sizeof(size_t);
            retval |= UA_Array_copy(*(void* const*)ptrs, size, (void**)ptrd, memberType);
            *dst_size = size;
            if(retval != UA_STATUSCODE_GOOD)
                *dst_size = 0;
            ptrs += sizeof(void*);
            ptrd += sizeof(void*);
        }
    }
    if(retval != UA_STATUSCODE_GOOD)
        UA_deleteMembers(dst, type);
    return retval;
}

UA_StatusCode UA_copy(const void *src, void *dst, const UA_DataType *type) {
    memset(dst, 0, type->memSize);
    return copyNoInit(src, dst, type);
}

typedef void (*UA_deleteMembersSignature)(void *p, const UA_DataType *type);
static void nopDeleteMembers(void *p, const UA_DataType *type) { }

static const UA_deleteMembersSignature deleteMembersJumpTable[UA_BUILTIN_TYPES_COUNT + 1] = {
    (UA_deleteMembersSignature)nopDeleteMembers, // Boolean
    (UA_deleteMembersSignature)nopDeleteMembers, // SByte
    (UA_deleteMembersSignature)nopDeleteMembers, // Byte
    (UA_deleteMembersSignature)nopDeleteMembers, // Int16
    (UA_deleteMembersSignature)nopDeleteMembers, // UInt16 
    (UA_deleteMembersSignature)nopDeleteMembers, // Int32 
    (UA_deleteMembersSignature)nopDeleteMembers, // UInt32 
    (UA_deleteMembersSignature)nopDeleteMembers, // Int64
    (UA_deleteMembersSignature)nopDeleteMembers, // UInt64 
    (UA_deleteMembersSignature)nopDeleteMembers, // Float 
    (UA_deleteMembersSignature)nopDeleteMembers, // Double 
    (UA_deleteMembersSignature)UA_deleteMembers, // String
    (UA_deleteMembersSignature)nopDeleteMembers, // DateTime
    (UA_deleteMembersSignature)nopDeleteMembers, // Guid 
    (UA_deleteMembersSignature)UA_deleteMembers, // ByteString
    (UA_deleteMembersSignature)UA_deleteMembers, // XmlElement
    (UA_deleteMembersSignature)NodeId_deleteMembers,
    (UA_deleteMembersSignature)ExpandedNodeId_deleteMembers, // ExpandedNodeId
    (UA_deleteMembersSignature)nopDeleteMembers, // StatusCode
    (UA_deleteMembersSignature)UA_deleteMembers, // QualifiedName
    (UA_deleteMembersSignature)LocalizedText_deleteMembers, // LocalizedText
    (UA_deleteMembersSignature)ExtensionObject_deleteMembers,
    (UA_deleteMembersSignature)DataValue_deleteMembers,
    (UA_deleteMembersSignature)Variant_deletemembers,
    (UA_deleteMembersSignature)DiagnosticInfo_deleteMembers,
    (UA_deleteMembersSignature)UA_deleteMembers,
};

void UA_deleteMembers(void *p, const UA_DataType *type) {
    uintptr_t ptr = (uintptr_t)p;
    UA_Byte membersSize = type->membersSize;
    for(size_t i = 0; i < membersSize; i++) {
        const UA_DataTypeMember *member = &type->members[i];
        const UA_DataType *typelists[2] = { UA_TYPES, &type[-type->typeIndex] };
        const UA_DataType *memberType = &typelists[!member->namespaceZero][member->memberTypeIndex];
        if(!member->isArray) {
            ptr += member->padding;
            size_t fi = memberType->builtin ? memberType->typeIndex : UA_BUILTIN_TYPES_COUNT;
            deleteMembersJumpTable[fi]((void*)ptr, memberType);
            ptr += memberType->memSize;
        } else {
            ptr += member->padding;
            size_t length = *(size_t*)ptr;
            *(size_t*)ptr = 0;
            ptr += sizeof(size_t);
            UA_Array_delete(*(void**)ptr, length, memberType);
            *(void**)ptr = NULL;
            ptr += sizeof(void*);
        }
    }
}

void UA_delete(void *p, const UA_DataType *type) {
    UA_deleteMembers(p, type);
    UA_free(p);
}

/******************/
/* Array Handling */
/******************/

void * UA_Array_new(size_t size, const UA_DataType *type) {
    if(size == 0)
        return UA_EMPTY_ARRAY_SENTINEL;
    return UA_calloc(size, type->memSize);
}

UA_StatusCode
UA_Array_copy(const void *src, size_t src_size, void **dst, const UA_DataType *type) {
    if(src_size == 0) {
        if(src == NULL)
            *dst = NULL;
        else
            *dst= UA_EMPTY_ARRAY_SENTINEL;
        return UA_STATUSCODE_GOOD;
    }

    /* calloc, so we don't have to check retval in every iteration of copying */
    *dst = UA_calloc(src_size, type->memSize);
    if(!*dst)
        return UA_STATUSCODE_BADOUTOFMEMORY;

    if(type->fixedSize) {
        memcpy(*dst, src, type->memSize * src_size);
        return UA_STATUSCODE_GOOD;
    }

    uintptr_t ptrs = (uintptr_t)src;
    uintptr_t ptrd = (uintptr_t)*dst;
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    for(size_t i = 0; i < src_size; i++) {
        retval |= UA_copy((void*)ptrs, (void*)ptrd, type);
        ptrs += type->memSize;
        ptrd += type->memSize;
    }
    if(retval != UA_STATUSCODE_GOOD) {
        UA_Array_delete(*dst, src_size, type);
        *dst = NULL;
    }
    return retval;
}

void UA_Array_delete(void *p, size_t size, const UA_DataType *type) {
    if(!type->fixedSize) {
        uintptr_t ptr = (uintptr_t)p;
        for(size_t i = 0; i < size; i++) {
            UA_deleteMembers((void*)ptr, type);
            ptr += type->memSize;
        }
    }
    UA_free((void*)((uintptr_t)p & ~(uintptr_t)UA_EMPTY_ARRAY_SENTINEL));
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/ua_types_encoding_binary.c" ***********************************/


/* We give pointers to the current position and the last position in the buffer
   instead of a string with an offset. */
typedef UA_Byte * UA_RESTRICT * const bufpos;
typedef UA_Byte const * bufend;

/* Jumptables for de-/encoding and computing the buffer length */
typedef UA_StatusCode (*UA_encodeBinarySignature)(const void *UA_RESTRICT src, const UA_DataType *type, bufpos pos, bufend end);
static const UA_encodeBinarySignature encodeBinaryJumpTable[UA_BUILTIN_TYPES_COUNT + 1];

typedef UA_StatusCode (*UA_decodeBinarySignature)(bufpos pos, bufend end, void *UA_RESTRICT dst, const UA_DataType *type);
static const UA_decodeBinarySignature decodeBinaryJumpTable[UA_BUILTIN_TYPES_COUNT + 1];

typedef size_t (*UA_calcSizeBinarySignature)(const void *UA_RESTRICT p, const UA_DataType *contenttype);
static const UA_calcSizeBinarySignature calcSizeBinaryJumpTable[UA_BUILTIN_TYPES_COUNT + 1];

/* Thread-local buffers used for exchanging the buffer for chunking */
UA_THREAD_LOCAL UA_ByteString *encodeBuf; /* the original buffer */
UA_THREAD_LOCAL UA_exchangeEncodeBuffer exchangeBufferCallback;
UA_THREAD_LOCAL void *exchangeBufferCallbackHandle;

static UA_StatusCode exchangeBuffer(bufpos pos, bufend *end) {
    if(!exchangeBufferCallback)
        return UA_STATUSCODE_BADENCODINGERROR;
    size_t offset = ((uintptr_t)*pos - (uintptr_t)encodeBuf->data) / sizeof(UA_Byte);
    UA_StatusCode retval = exchangeBufferCallback(exchangeBufferCallbackHandle, encodeBuf, offset);
    /* set pos and end in order to continue encoding */
    *pos = encodeBuf->data;
    *end = &encodeBuf->data[encodeBuf->length];
    return retval;
}

/*****************/
/* Integer Types */
/*****************/

/* The following en/decoding functions are used only when the architecture isn't
   little-endian. */
static void UA_encode16(const UA_UInt16 v, UA_Byte buf[2]) {
    buf[0] = (UA_Byte)v; buf[1] = (UA_Byte)(v >> 8);
}
static void UA_decode16(const UA_Byte buf[2], UA_UInt16 *v) {
    *v = (UA_UInt16)((UA_UInt16)buf[0] + (((UA_UInt16)buf[1]) << 8));
}
static void UA_encode32(const UA_UInt32 v, UA_Byte buf[4]) {
    buf[0] = (UA_Byte)v;         buf[1] = (UA_Byte)(v >> 8);
    buf[2] = (UA_Byte)(v >> 16); buf[3] = (UA_Byte)(v >> 24);
}
static void UA_decode32(const UA_Byte buf[4], UA_UInt32 *v) {
    *v = (UA_UInt32)((UA_UInt32)buf[0] + (((UA_UInt32)buf[1]) << 8) +
                    (((UA_UInt32)buf[2]) << 16) + (((UA_UInt32)buf[3]) << 24));
}
static void UA_encode64(const UA_UInt64 v, UA_Byte buf[8]) {
    buf[0] = (UA_Byte)v;         buf[1] = (UA_Byte)(v >> 8);
    buf[2] = (UA_Byte)(v >> 16); buf[3] = (UA_Byte)(v >> 24);
    buf[4] = (UA_Byte)(v >> 32); buf[5] = (UA_Byte)(v >> 40);
    buf[6] = (UA_Byte)(v >> 48); buf[7] = (UA_Byte)(v >> 56);
}
static void UA_decode64(const UA_Byte buf[8], UA_UInt64 *v) {
    *v = (UA_UInt64)((UA_UInt64)buf[0] + (((UA_UInt64)buf[1]) << 8) +
                    (((UA_UInt64)buf[2]) << 16) + (((UA_UInt64)buf[3]) << 24) +
                    (((UA_UInt64)buf[4]) << 32) + (((UA_UInt64)buf[5]) << 40) +
                    (((UA_UInt64)buf[6]) << 48) + (((UA_UInt64)buf[7]) << 56));
}

/* Boolean */
static UA_StatusCode
Boolean_encodeBinary(const UA_Boolean *src, const UA_DataType *_, bufpos pos, bufend end) {
    if(*pos + sizeof(UA_Boolean) > end)
        return UA_STATUSCODE_BADENCODINGERROR;
    **pos = *(const UA_Byte*)src;
    (*pos)++;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
Boolean_decodeBinary(bufpos pos, bufend end, UA_Boolean *dst, const UA_DataType *_) {
    if(*pos + sizeof(UA_Boolean) > end)
        return UA_STATUSCODE_BADDECODINGERROR;
    *dst = (**pos > 0) ? true : false;
    (*pos)++;
    return UA_STATUSCODE_GOOD;
}

/* Byte */
static UA_StatusCode
Byte_encodeBinary(const UA_Byte *src, const UA_DataType *_, bufpos pos, bufend end) {
    if(*pos + sizeof(UA_Byte) > end)
        return UA_STATUSCODE_BADENCODINGERROR;
    **pos = *(const UA_Byte*)src;
    (*pos)++;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
Byte_decodeBinary(bufpos pos, bufend end, UA_Byte *dst, const UA_DataType *_) {
    if(*pos + sizeof(UA_Byte) > end)
        return UA_STATUSCODE_BADDECODINGERROR;
    *dst = **pos;
    (*pos)++;
    return UA_STATUSCODE_GOOD;
}

/* UInt16 */
static UA_StatusCode
UInt16_encodeBinary(UA_UInt16 const *src, const UA_DataType *_, bufpos pos, bufend end) {
    if(*pos + sizeof(UA_UInt16) > end)
        return UA_STATUSCODE_BADENCODINGERROR;
#if UA_BINARY_OVERLAYABLE_INTEGER
    memcpy(*pos, src, sizeof(UA_UInt16));
#else
    UA_encode16(*src, *pos);
#endif
    (*pos) += 2;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE UA_StatusCode
Int16_encodeBinary(UA_Int16 const *src, const UA_DataType *_, bufpos pos, bufend end) {
    return UInt16_encodeBinary((const UA_UInt16*)src, NULL, pos, end);
}

static UA_StatusCode
UInt16_decodeBinary(bufpos pos, bufend end, UA_UInt16 *dst, const UA_DataType *_) {
    if(*pos + sizeof(UA_UInt16) > end)
        return UA_STATUSCODE_BADDECODINGERROR;
#if UA_BINARY_OVERLAYABLE_INTEGER
    memcpy(dst, *pos, sizeof(UA_UInt16));
#else
    UA_decode16(*pos, dst);
#endif
    (*pos) += 2;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE UA_StatusCode
Int16_decodeBinary(bufpos pos, bufend end, UA_Int16 *dst) {
    return UInt16_decodeBinary(pos, end, (UA_UInt16*)dst, NULL);
}

/* UInt32 */
static UA_StatusCode
UInt32_encodeBinary(UA_UInt32 const *src, const UA_DataType *_, bufpos pos, bufend end) {
    if(*pos + sizeof(UA_UInt32) > end)
        return UA_STATUSCODE_BADENCODINGERROR;
#if UA_BINARY_OVERLAYABLE_INTEGER
    memcpy(*pos, src, sizeof(UA_UInt32));
#else
    UA_encode32(*src, *pos);
#endif
    (*pos) += 4;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE UA_StatusCode
Int32_encodeBinary(UA_Int32 const *src, bufpos pos, bufend end) {
    return UInt32_encodeBinary((const UA_UInt32*)src, NULL, pos, end);
}

static UA_INLINE UA_StatusCode
StatusCode_encodeBinary(UA_StatusCode const *src, bufpos pos, bufend end) {
    return UInt32_encodeBinary((const UA_UInt32*)src, NULL, pos, end);
}

static UA_StatusCode
UInt32_decodeBinary(bufpos pos, bufend end, UA_UInt32 *dst, const UA_DataType *_) {
    if(*pos + sizeof(UA_UInt32) > end)
        return UA_STATUSCODE_BADDECODINGERROR;
#if UA_BINARY_OVERLAYABLE_INTEGER
    memcpy(dst, *pos, sizeof(UA_UInt32));
#else
    UA_decode32(*pos, dst);
#endif
    (*pos) += 4;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE UA_StatusCode
Int32_decodeBinary(bufpos pos, bufend end, UA_Int32 *dst) {
    return UInt32_decodeBinary(pos, end, (UA_UInt32*)dst, NULL);
}

static UA_INLINE UA_StatusCode
StatusCode_decodeBinary(bufpos pos, bufend end, UA_StatusCode *dst) {
    return UInt32_decodeBinary(pos, end, (UA_UInt32*)dst, NULL);
}

/* UInt64 */
static UA_StatusCode
UInt64_encodeBinary(UA_UInt64 const *src, const UA_DataType *_, bufpos pos, bufend end) {
    if(*pos + sizeof(UA_UInt64) > end)
        return UA_STATUSCODE_BADENCODINGERROR;
#if UA_BINARY_OVERLAYABLE_INTEGER
    memcpy(*pos, src, sizeof(UA_UInt64));
#else
    UA_encode64(*src, *pos);
#endif
    (*pos) += 8;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE UA_StatusCode
Int64_encodeBinary(UA_Int64 const *src, bufpos pos, bufend end) {
    return UInt64_encodeBinary((const UA_UInt64*)src, NULL, pos, end);
}

static UA_INLINE UA_StatusCode
DateTime_encodeBinary(UA_DateTime const *src, bufpos pos, bufend end) {
    return UInt64_encodeBinary((const UA_UInt64*)src, NULL, pos, end);
}

static UA_StatusCode
UInt64_decodeBinary(bufpos pos, bufend end, UA_UInt64 *dst, const UA_DataType *_) {
    if(*pos + sizeof(UA_UInt64) > end)
        return UA_STATUSCODE_BADDECODINGERROR;
#if UA_BINARY_OVERLAYABLE_INTEGER
    memcpy(dst, *pos, sizeof(UA_UInt64));
#else
    UA_decode64(*pos, dst);
#endif
    (*pos) += 8;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE UA_StatusCode
Int64_decodeBinary(bufpos pos, bufend end, UA_Int64 *dst) {
    return UInt64_decodeBinary(pos, end, (UA_UInt64*)dst, NULL);
}

static UA_INLINE UA_StatusCode
DateTime_decodeBinary(bufpos pos, bufend end, UA_DateTime *dst) {
    return UInt64_decodeBinary(pos, end, (UA_UInt64*)dst, NULL);
}

/************************/
/* Floating Point Types */
/************************/

#if UA_BINARY_OVERLAYABLE_FLOAT
# define Float_encodeBinary UInt32_encodeBinary
# define Float_decodeBinary UInt32_decodeBinary
# define Double_encodeBinary UInt64_encodeBinary
# define Double_decodeBinary UInt64_decodeBinary
#else

#include <math.h>

/* Handling of IEEE754 floating point values was taken from Beej's Guide to
   Network Programming (http://beej.us/guide/bgnet/) and enhanced to cover the
   edge cases +/-0, +/-inf and nan. */
static uint64_t pack754(long double f, unsigned bits, unsigned expbits) {
    unsigned significandbits = bits - expbits - 1;
    long double fnorm;
    long long sign;
    if (f < 0) { sign = 1; fnorm = -f; }
    else { sign = 0; fnorm = f; }
    int shift = 0;
    while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
    while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
    fnorm = fnorm - 1.0;
    long long significand = (long long)(fnorm * ((float)(1LL<<significandbits) + 0.5f));
    long long exp = shift + ((1<<(expbits-1)) - 1);
    return (uint64_t)((sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand);
}

static long double unpack754(uint64_t i, unsigned bits, unsigned expbits) {
    unsigned significandbits = bits - expbits - 1;
    long double result = (long double)(i&(uint64_t)((1LL<<significandbits)-1));
    result /= (1LL<<significandbits);
    result += 1.0f;
    unsigned bias = (unsigned)(1<<(expbits-1)) - 1;
    long long shift = (long long)((i>>significandbits) & (uint64_t)((1LL<<expbits)-1)) - bias;
    while(shift > 0) { result *= 2.0; shift--; }
    while(shift < 0) { result /= 2.0; shift++; }
    result *= ((i>>(bits-1))&1)? -1.0: 1.0;
    return result;
}

/* Float */
#define FLOAT_NAN 0xffc00000
#define FLOAT_INF 0x7f800000
#define FLOAT_NEG_INF 0xff800000
#define FLOAT_NEG_ZERO 0x80000000

static UA_StatusCode
Float_encodeBinary(UA_Float const *src, const UA_DataType *_, bufpos pos, bufend end) {
    UA_Float f = *src;
    UA_UInt32 encoded;
    //cppcheck-suppress duplicateExpression
    if(f != f) encoded = FLOAT_NAN;
    else if(f == 0.0f) encoded = signbit(f) ? FLOAT_NEG_ZERO : 0;
    //cppcheck-suppress duplicateExpression
    else if(f/f != f/f) encoded = f > 0 ? FLOAT_INF : FLOAT_NEG_INF;
    else encoded = (UA_UInt32)pack754(f, 32, 8);
    return UInt32_encodeBinary(&encoded, NULL, pos, end);
}

static UA_StatusCode
Float_decodeBinary(bufpos pos, bufend end, UA_Float *dst, const UA_DataType *_) {
    UA_UInt32 decoded;
    UA_StatusCode retval = UInt32_decodeBinary(pos, end, &decoded, NULL);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    if(decoded == 0) *dst = 0.0f;
    else if(decoded == FLOAT_NEG_ZERO) *dst = -0.0f;
    else if(decoded == FLOAT_INF) *dst = INFINITY;
    else if(decoded == FLOAT_NEG_INF) *dst = -INFINITY;
    if((decoded >= 0x7f800001 && decoded <= 0x7fffffff) ||
       (decoded >= 0xff800001 && decoded <= 0xffffffff)) *dst = NAN;
    else *dst = (UA_Float)unpack754(decoded, 32, 8);
    return UA_STATUSCODE_GOOD;
}

/* Double */
#define DOUBLE_NAN 0xfff8000000000000L
#define DOUBLE_INF 0x7ff0000000000000L
#define DOUBLE_NEG_INF 0xfff0000000000000L
#define DOUBLE_NEG_ZERO 0x8000000000000000L

static UA_StatusCode
Double_encodeBinary(UA_Double const *src, const UA_DataType *_, bufpos pos, bufend end) {
    UA_Double d = *src;
    UA_UInt64 encoded;
    //cppcheck-suppress duplicateExpression
    if(d != d) encoded = DOUBLE_NAN;
    else if(d == 0.0) encoded = signbit(d) ? DOUBLE_NEG_ZERO : 0;
    //cppcheck-suppress duplicateExpression
    else if(d/d != d/d) encoded = d > 0 ? DOUBLE_INF : DOUBLE_NEG_INF;
    else encoded = pack754(d, 64, 11);
    return UInt64_encodeBinary(&encoded, NULL, pos, end);
}

static UA_StatusCode
Double_decodeBinary(bufpos pos, bufend end, UA_Double *dst, const UA_DataType *_) {
    UA_UInt64 decoded;
    UA_StatusCode retval = UInt64_decodeBinary(pos, end, &decoded, NULL);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    if(decoded == 0) *dst = 0.0;
    else if(decoded == DOUBLE_NEG_ZERO) *dst = -0.0;
    else if(decoded == DOUBLE_INF) *dst = INFINITY;
    else if(decoded == DOUBLE_NEG_INF) *dst = -INFINITY;
    //cppcheck-suppress redundantCondition
    if((decoded >= 0x7ff0000000000001L && decoded <= 0x7fffffffffffffffL) ||
       (decoded >= 0xfff0000000000001L && decoded <= 0xffffffffffffffffL)) *dst = NAN;
    else *dst = (UA_Double)unpack754(decoded, 64, 11);
    return UA_STATUSCODE_GOOD;
}

#endif

/******************/
/* Array Handling */
/******************/

static UA_StatusCode
Array_encodeBinary(const void *src, size_t length, const UA_DataType *type, bufpos pos, bufend end) {
    UA_Int32 signed_length = -1;
    if(length > UA_INT32_MAX)
        return UA_STATUSCODE_BADINTERNALERROR;
    if(length > 0)
        signed_length = (UA_Int32)length;
    else if(src == UA_EMPTY_ARRAY_SENTINEL)
        signed_length = 0;
    UA_StatusCode retval = Int32_encodeBinary(&signed_length, pos, end);
    if(retval != UA_STATUSCODE_GOOD || length == 0)
        return retval;

    if(type->overlayable) {
        size_t i = 0; /* the number of already encoded elements */
        while(end < *pos + (type->memSize * (length-i))) {
            /* not enough space, need to exchange the buffer */
            size_t elements = ((uintptr_t)end - (uintptr_t)*pos) / (sizeof(UA_Byte) * type->memSize);
            memcpy(*pos, src, type->memSize * elements);
            *pos += type->memSize * elements;
            i += elements;
            retval = exchangeBuffer(pos, &end);
            if(retval != UA_STATUSCODE_GOOD)
                return retval;
        }
        /* encode the remaining elements */
        memcpy(*pos, src, type->memSize * (length-i));
        *pos += type->memSize * (length-i);
        return UA_STATUSCODE_GOOD;
    }

    uintptr_t ptr = (uintptr_t)src;
    size_t encode_index = type->builtin ? type->typeIndex : UA_BUILTIN_TYPES_COUNT;
    for(size_t i = 0; i < length && retval == UA_STATUSCODE_GOOD; i++) {
        UA_Byte *oldpos = *pos;
        retval = encodeBinaryJumpTable[encode_index]((const void*)ptr, type, pos, end);
        ptr += type->memSize;
        if(retval == UA_STATUSCODE_BADENCODINGERROR) {
            /* exchange the buffer and try to encode the same element once more */
            *pos = oldpos;
            retval = exchangeBuffer(pos, &end);
            /* Repeat encoding of the same element */
            ptr -= type->memSize;
            i--;
        }
    }
    return retval;
}

static UA_StatusCode
Array_decodeBinary(bufpos pos, bufend end, UA_Int32 signed_length, void *UA_RESTRICT *UA_RESTRICT dst,
                   size_t *out_length, const UA_DataType *type) {
    *out_length = 0;
    if(signed_length <= 0) {
        *dst = NULL;
        if(signed_length == 0)
            *dst = UA_EMPTY_ARRAY_SENTINEL;
        return UA_STATUSCODE_GOOD;
    }
    size_t length = (size_t)signed_length;

    /* filter out arrays that can obviously not be parsed, because the message
       is too small */
    if(*pos + ((type->memSize * length) / 32) > end)
        return UA_STATUSCODE_BADDECODINGERROR;

    *dst = UA_calloc(1, type->memSize * length);
    if(!*dst)
        return UA_STATUSCODE_BADOUTOFMEMORY;

    if(type->overlayable) {
        if(end < *pos + (type->memSize * length))
            return UA_STATUSCODE_BADDECODINGERROR;
        memcpy(*dst, *pos, type->memSize * length);
        (*pos) += type->memSize * length;
        *out_length = length;
        return UA_STATUSCODE_GOOD;
    }

    uintptr_t ptr = (uintptr_t)*dst;
    size_t decode_index = type->builtin ? type->typeIndex : UA_BUILTIN_TYPES_COUNT;
    for(size_t i = 0; i < length; i++) {
        UA_StatusCode retval = decodeBinaryJumpTable[decode_index](pos, end, (void*)ptr, type);
        if(retval != UA_STATUSCODE_GOOD) {
            UA_Array_delete(*dst, i, type);
            *dst = NULL;
            return retval;
        }
        ptr += type->memSize;
    }
    *out_length = length;
    return UA_STATUSCODE_GOOD;
}

/*****************/
/* Builtin Types */
/*****************/

static UA_StatusCode
String_encodeBinary(UA_String const *src, const UA_DataType *_, bufpos pos, bufend end) {
    return Array_encodeBinary(src->data, src->length, &UA_TYPES[UA_TYPES_BYTE], pos, end);
}

static UA_INLINE UA_StatusCode
ByteString_encodeBinary(UA_ByteString const *src, bufpos pos, bufend end) {
    return String_encodeBinary((const UA_String*)src, NULL, pos, end);
}

static UA_StatusCode
String_decodeBinary(bufpos pos, bufend end, UA_String *dst, const UA_DataType *_) {
    UA_Int32 signed_length;
    UA_StatusCode retval = Int32_decodeBinary(pos, end, &signed_length);
    if(retval != UA_STATUSCODE_GOOD)
        return UA_STATUSCODE_BADINTERNALERROR;
    if(signed_length <= 0) {
        if(signed_length == 0)
            dst->data = UA_EMPTY_ARRAY_SENTINEL;
        else
            dst->data = NULL;
        return UA_STATUSCODE_GOOD;
    }
    size_t length = (size_t)signed_length;
    if(*pos + length > end)
        return UA_STATUSCODE_BADDECODINGERROR;
    dst->data = UA_malloc(length);
    if(!dst->data)
        return UA_STATUSCODE_BADOUTOFMEMORY;
    memcpy(dst->data, *pos, length);
    dst->length = length;
    *pos += length;
    return UA_STATUSCODE_GOOD;
}

static UA_INLINE UA_StatusCode
ByteString_decodeBinary(bufpos pos, bufend end, UA_ByteString *dst) {
    return String_decodeBinary(pos, end, (UA_ByteString*)dst, NULL);
}

/* Guid */
static UA_StatusCode
Guid_encodeBinary(UA_Guid const *src, const UA_DataType *_, bufpos pos, bufend end) {
    UA_StatusCode retval = UInt32_encodeBinary(&src->data1, NULL, pos, end);
    retval |= UInt16_encodeBinary(&src->data2, NULL, pos, end);
    retval |= UInt16_encodeBinary(&src->data3, NULL, pos, end);
    for(UA_Int32 i = 0; i < 8; i++)
        retval |= Byte_encodeBinary(&src->data4[i], NULL, pos, end);
    return retval;
}

static UA_StatusCode
Guid_decodeBinary(bufpos pos, bufend end, UA_Guid *dst, const UA_DataType *_) {
    UA_StatusCode retval = UInt32_decodeBinary(pos, end, &dst->data1, NULL);
    retval |= UInt16_decodeBinary(pos, end, &dst->data2, NULL);
    retval |= UInt16_decodeBinary(pos, end, &dst->data3, NULL);
    for(size_t i = 0; i < 8; i++)
        retval |= Byte_decodeBinary(pos, end, &dst->data4[i], NULL);
    if(retval != UA_STATUSCODE_GOOD)
        UA_Guid_deleteMembers(dst);
    return retval;
}

/* NodeId */
#define UA_NODEIDTYPE_NUMERIC_TWOBYTE 0
#define UA_NODEIDTYPE_NUMERIC_FOURBYTE 1
#define UA_NODEIDTYPE_NUMERIC_COMPLETE 2

static UA_StatusCode
NodeId_encodeBinary(UA_NodeId const *src, const UA_DataType *_, bufpos pos, bufend end) {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    // temporary variables for endian-save code
    UA_Byte srcByte;
    UA_UInt16 srcUInt16;
    UA_UInt32 srcUInt32;
    switch (src->identifierType) {
    case UA_NODEIDTYPE_NUMERIC:
        if(src->identifier.numeric > UA_UINT16_MAX || src->namespaceIndex > UA_BYTE_MAX) {
            srcByte = UA_NODEIDTYPE_NUMERIC_COMPLETE;
            retval |= Byte_encodeBinary(&srcByte, NULL, pos, end);
            retval |= UInt16_encodeBinary(&src->namespaceIndex, NULL, pos, end);
            srcUInt32 = src->identifier.numeric;
            retval |= UInt32_encodeBinary(&srcUInt32, NULL, pos, end);
        } else if(src->identifier.numeric > UA_BYTE_MAX || src->namespaceIndex > 0) {
            srcByte = UA_NODEIDTYPE_NUMERIC_FOURBYTE;
            retval |= Byte_encodeBinary(&srcByte, NULL, pos, end);
            srcByte = (UA_Byte)src->namespaceIndex;
            srcUInt16 = (UA_UInt16)src->identifier.numeric;
            retval |= Byte_encodeBinary(&srcByte, NULL, pos, end);
            retval |= UInt16_encodeBinary(&srcUInt16, NULL, pos, end);
        } else {
            srcByte = UA_NODEIDTYPE_NUMERIC_TWOBYTE;
            retval |= Byte_encodeBinary(&srcByte, NULL, pos, end);
            srcByte = (UA_Byte)src->identifier.numeric;
            retval |= Byte_encodeBinary(&srcByte, NULL, pos, end);
        }
        break;
    case UA_NODEIDTYPE_STRING:
        srcByte = UA_NODEIDTYPE_STRING;
        retval |= Byte_encodeBinary(&srcByte, NULL, pos, end);
        retval |= UInt16_encodeBinary(&src->namespaceIndex, NULL, pos, end);
        retval |= String_encodeBinary(&src->identifier.string, NULL, pos, end);
        break;
    case UA_NODEIDTYPE_GUID:
        srcByte = UA_NODEIDTYPE_GUID;
        retval |= Byte_encodeBinary(&srcByte, NULL, pos, end);
        retval |= UInt16_encodeBinary(&src->namespaceIndex, NULL, pos, end);
        retval |= Guid_encodeBinary(&src->identifier.guid, NULL, pos, end);
        break;
    case UA_NODEIDTYPE_BYTESTRING:
        srcByte = UA_NODEIDTYPE_BYTESTRING;
        retval |= Byte_encodeBinary(&srcByte, NULL, pos, end);
        retval |= UInt16_encodeBinary(&src->namespaceIndex, NULL, pos, end);
        retval |= ByteString_encodeBinary(&src->identifier.byteString, pos, end);
        break;
    default:
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    return retval;
}

static UA_StatusCode
NodeId_decodeBinary(bufpos pos, bufend end, UA_NodeId *dst, const UA_DataType *_) {
    UA_Byte dstByte = 0, encodingByte = 0;
    UA_UInt16 dstUInt16 = 0;
    UA_StatusCode retval = Byte_decodeBinary(pos, end, &encodingByte, NULL);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    switch (encodingByte) {
    case UA_NODEIDTYPE_NUMERIC_TWOBYTE:
        dst->identifierType = UA_NODEIDTYPE_NUMERIC;
        retval = Byte_decodeBinary(pos, end, &dstByte, NULL);
        dst->identifier.numeric = dstByte;
        dst->namespaceIndex = 0;
        break;
    case UA_NODEIDTYPE_NUMERIC_FOURBYTE:
        dst->identifierType = UA_NODEIDTYPE_NUMERIC;
        retval |= Byte_decodeBinary(pos, end, &dstByte, NULL);
        dst->namespaceIndex = dstByte;
        retval |= UInt16_decodeBinary(pos, end, &dstUInt16, NULL);
        dst->identifier.numeric = dstUInt16;
        break;
    case UA_NODEIDTYPE_NUMERIC_COMPLETE:
        dst->identifierType = UA_NODEIDTYPE_NUMERIC;
        retval |= UInt16_decodeBinary(pos, end, &dst->namespaceIndex, NULL);
        retval |= UInt32_decodeBinary(pos, end, &dst->identifier.numeric, NULL);
        break;
    case UA_NODEIDTYPE_STRING:
        dst->identifierType = UA_NODEIDTYPE_STRING;
        retval |= UInt16_decodeBinary(pos, end, &dst->namespaceIndex, NULL);
        retval |= String_decodeBinary(pos, end, &dst->identifier.string, NULL);
        break;
    case UA_NODEIDTYPE_GUID:
        dst->identifierType = UA_NODEIDTYPE_GUID;
        retval |= UInt16_decodeBinary(pos, end, &dst->namespaceIndex, NULL);
        retval |= Guid_decodeBinary(pos, end, &dst->identifier.guid, NULL);
        break;
    case UA_NODEIDTYPE_BYTESTRING:
        dst->identifierType = UA_NODEIDTYPE_BYTESTRING;
        retval |= UInt16_decodeBinary(pos, end, &dst->namespaceIndex, NULL);
        retval |= ByteString_decodeBinary(pos, end, &dst->identifier.byteString);
        break;
    default:
        retval |= UA_STATUSCODE_BADINTERNALERROR; // the client sends an encodingByte we do not recognize
        break;
    }
    if(retval != UA_STATUSCODE_GOOD)
        UA_NodeId_deleteMembers(dst);
    return retval;
}

/* ExpandedNodeId */
#define UA_EXPANDEDNODEID_NAMESPACEURI_FLAG 0x80
#define UA_EXPANDEDNODEID_SERVERINDEX_FLAG 0x40

static UA_StatusCode
ExpandedNodeId_encodeBinary(UA_ExpandedNodeId const *src, const UA_DataType *_,
                            bufpos pos, bufend end) {
    UA_Byte *start = *pos;
    UA_StatusCode retval = NodeId_encodeBinary(&src->nodeId, NULL, pos, end);
    if(src->namespaceUri.length > 0) {
        retval |= String_encodeBinary(&src->namespaceUri, NULL, pos, end);
        *start |= UA_EXPANDEDNODEID_NAMESPACEURI_FLAG;
    }
    if(src->serverIndex > 0) {
        retval |= UInt32_encodeBinary(&src->serverIndex, NULL, pos, end);
        *start |= UA_EXPANDEDNODEID_SERVERINDEX_FLAG;
    }
    return retval;
}

static UA_StatusCode
ExpandedNodeId_decodeBinary(bufpos pos, bufend end, UA_ExpandedNodeId *dst, const UA_DataType *_) {
    if(*pos >= end)
        return UA_STATUSCODE_BADDECODINGERROR;
    UA_Byte encodingByte = **pos;
    **pos = encodingByte & (UA_Byte)~(UA_EXPANDEDNODEID_NAMESPACEURI_FLAG | UA_EXPANDEDNODEID_SERVERINDEX_FLAG);
    UA_StatusCode retval = NodeId_decodeBinary(pos, end, &dst->nodeId, NULL);
    if(encodingByte & UA_EXPANDEDNODEID_NAMESPACEURI_FLAG) {
        dst->nodeId.namespaceIndex = 0;
        retval |= String_decodeBinary(pos, end, &dst->namespaceUri, NULL);
    }
    if(encodingByte & UA_EXPANDEDNODEID_SERVERINDEX_FLAG)
        retval |= UInt32_decodeBinary(pos, end, &dst->serverIndex, NULL);
    if(retval != UA_STATUSCODE_GOOD)
        UA_ExpandedNodeId_deleteMembers(dst);
    return retval;
}

/* LocalizedText */
#define UA_LOCALIZEDTEXT_ENCODINGMASKTYPE_LOCALE 0x01
#define UA_LOCALIZEDTEXT_ENCODINGMASKTYPE_TEXT 0x02

static UA_StatusCode
LocalizedText_encodeBinary(UA_LocalizedText const *src, const UA_DataType *_, bufpos pos, bufend end) {
    UA_Byte encodingMask = 0;
    if(src->locale.data)
        encodingMask |= UA_LOCALIZEDTEXT_ENCODINGMASKTYPE_LOCALE;
    if(src->text.data)
        encodingMask |= UA_LOCALIZEDTEXT_ENCODINGMASKTYPE_TEXT;
    UA_StatusCode retval = Byte_encodeBinary(&encodingMask, NULL, pos, end);
    if(encodingMask & UA_LOCALIZEDTEXT_ENCODINGMASKTYPE_LOCALE)
        retval |= String_encodeBinary(&src->locale, NULL, pos, end);
    if(encodingMask & UA_LOCALIZEDTEXT_ENCODINGMASKTYPE_TEXT)
        retval |= String_encodeBinary(&src->text, NULL, pos, end);
    return retval;
}

static UA_StatusCode
LocalizedText_decodeBinary(bufpos pos, bufend end, UA_LocalizedText *dst, const UA_DataType *_) {
    UA_Byte encodingMask = 0;
    UA_StatusCode retval = Byte_decodeBinary(pos, end, &encodingMask, NULL);
    if(encodingMask & UA_LOCALIZEDTEXT_ENCODINGMASKTYPE_LOCALE)
        retval |= String_decodeBinary(pos, end, &dst->locale, NULL);
    if(encodingMask & UA_LOCALIZEDTEXT_ENCODINGMASKTYPE_TEXT)
        retval |= String_decodeBinary(pos, end, &dst->text, NULL);
    if(retval != UA_STATUSCODE_GOOD)
        UA_LocalizedText_deleteMembers(dst);
    return retval;
}

/* ExtensionObject */
static UA_StatusCode
ExtensionObject_encodeBinary(UA_ExtensionObject const *src, const UA_DataType *_, bufpos pos, bufend end) {
    UA_StatusCode retval;
    UA_Byte encoding = src->encoding;
    if(encoding > UA_EXTENSIONOBJECT_ENCODED_XML) {
        if(!src->content.decoded.type || !src->content.decoded.data)
            return UA_STATUSCODE_BADENCODINGERROR;
        UA_NodeId typeId = src->content.decoded.type->typeId;
        if(typeId.identifierType != UA_NODEIDTYPE_NUMERIC)
            return UA_STATUSCODE_BADENCODINGERROR;
        typeId.identifier.numeric += UA_ENCODINGOFFSET_BINARY;
        encoding = UA_EXTENSIONOBJECT_ENCODED_BYTESTRING;
        retval = NodeId_encodeBinary(&typeId, NULL, pos, end);
        retval |= Byte_encodeBinary(&encoding, NULL, pos, end);
        UA_Byte *old_pos = *pos; // jump back to encode the length
        (*pos) += 4;
        const UA_DataType *type = src->content.decoded.type;
        size_t encode_index = type->builtin ? type->typeIndex : UA_BUILTIN_TYPES_COUNT;
        retval |= encodeBinaryJumpTable[encode_index](src->content.decoded.data, type, pos, end);
        UA_Int32 length = (UA_Int32)(((uintptr_t)*pos - (uintptr_t)old_pos) / sizeof(UA_Byte)) - 4;
        retval |= Int32_encodeBinary(&length, &old_pos, end);
    } else {
        retval = NodeId_encodeBinary(&src->content.encoded.typeId, NULL, pos, end);
        retval |= Byte_encodeBinary(&encoding, NULL, pos, end);
        switch (src->encoding) {
        case UA_EXTENSIONOBJECT_ENCODED_NOBODY:
            break;
        case UA_EXTENSIONOBJECT_ENCODED_BYTESTRING:
        case UA_EXTENSIONOBJECT_ENCODED_XML:
            retval |= ByteString_encodeBinary(&src->content.encoded.body, pos, end);
            break;
        default:
            return UA_STATUSCODE_BADINTERNALERROR;
        }
    }
    return retval;
}

static UA_StatusCode findDataType(const UA_NodeId *typeId, const UA_DataType **findtype) {
    for(size_t i = 0; i < UA_TYPES_COUNT; i++) {
        if(UA_NodeId_equal(typeId, &UA_TYPES[i].typeId)) {
            *findtype = &UA_TYPES[i];
            return UA_STATUSCODE_GOOD;
        }
    }
    return UA_STATUSCODE_BADNODEIDUNKNOWN;
}

static UA_StatusCode
ExtensionObject_decodeBinary(bufpos pos, bufend end, UA_ExtensionObject *dst, const UA_DataType *_) {
    UA_Byte encoding = 0;
    UA_NodeId typeId;
    UA_NodeId_init(&typeId);
    UA_StatusCode retval = NodeId_decodeBinary(pos, end, &typeId, NULL);
    retval |= Byte_decodeBinary(pos, end, &encoding, NULL);
    if(typeId.namespaceIndex != 0 || typeId.identifierType != UA_NODEIDTYPE_NUMERIC)
        retval = UA_STATUSCODE_BADDECODINGERROR;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_NodeId_deleteMembers(&typeId);
        return retval;
    }

    if(encoding == UA_EXTENSIONOBJECT_ENCODED_NOBODY) {
        dst->encoding = encoding;
        dst->content.encoded.typeId = typeId;
        dst->content.encoded.body = UA_BYTESTRING_NULL;
    } else if(encoding == UA_EXTENSIONOBJECT_ENCODED_XML) {
        dst->encoding = encoding;
        dst->content.encoded.typeId = typeId;
        retval = ByteString_decodeBinary(pos, end, &dst->content.encoded.body);
    } else {
        /* try to decode the content */
        const UA_DataType *type = NULL;
        /* helping clang analyzer, typeId is numeric */
        UA_assert(typeId.identifier.byteString.data == NULL);
        UA_assert(typeId.identifier.string.data == NULL);
        typeId.identifier.numeric -= UA_ENCODINGOFFSET_BINARY;
        findDataType(&typeId, &type);
        if(type) {
            (*pos) += 4; /* jump over the length (todo: check if length matches) */
            dst->content.decoded.data = UA_new(type);
            size_t decode_index = type->builtin ? type->typeIndex : UA_BUILTIN_TYPES_COUNT;
            if(dst->content.decoded.data) {
                dst->content.decoded.type = type;
                dst->encoding = UA_EXTENSIONOBJECT_DECODED;
                retval = decodeBinaryJumpTable[decode_index](pos, end, dst->content.decoded.data, type);
            } else
                retval = UA_STATUSCODE_BADOUTOFMEMORY;
        } else {
            retval = ByteString_decodeBinary(pos, end, &dst->content.encoded.body);
            dst->encoding = UA_EXTENSIONOBJECT_ENCODED_BYTESTRING;
            dst->content.encoded.typeId = typeId;
        }
    }
    if(retval != UA_STATUSCODE_GOOD)
        UA_ExtensionObject_deleteMembers(dst);
    return retval;
}

/* Variant */
enum UA_VARIANT_ENCODINGMASKTYPE {
    UA_VARIANT_ENCODINGMASKTYPE_TYPEID_MASK = 0x3F,        // bits 0:5
    UA_VARIANT_ENCODINGMASKTYPE_DIMENSIONS  = (0x01 << 6), // bit 6
    UA_VARIANT_ENCODINGMASKTYPE_ARRAY       = (0x01 << 7)  // bit 7
};

static UA_StatusCode
Variant_encodeBinary(UA_Variant const *src, const UA_DataType *_, bufpos pos, bufend end) {
    if(!src->type)
        return UA_STATUSCODE_BADINTERNALERROR;
    const UA_Boolean isArray = src->arrayLength > 0 || src->data <= UA_EMPTY_ARRAY_SENTINEL;
    const UA_Boolean hasDimensions = isArray && src->arrayDimensionsSize > 0;
    const UA_Boolean isBuiltin = src->type->builtin;

    /* Encode the encodingbyte */
    UA_Byte encodingByte = 0;
    if(isArray) {
        encodingByte |= UA_VARIANT_ENCODINGMASKTYPE_ARRAY;
        if(hasDimensions)
            encodingByte |= UA_VARIANT_ENCODINGMASKTYPE_DIMENSIONS;
    }
    if(isBuiltin)
        encodingByte |= UA_VARIANT_ENCODINGMASKTYPE_TYPEID_MASK & (UA_Byte) (src->type->typeIndex + 1);
    else
        encodingByte |= UA_VARIANT_ENCODINGMASKTYPE_TYPEID_MASK & (UA_Byte) 22; /* ExtensionObject */
    UA_StatusCode retval = Byte_encodeBinary(&encodingByte, NULL, pos, end);

    /* Encode the content */
    if(isBuiltin) {
        if(!isArray) {
            size_t encode_index = src->type->typeIndex;
            retval |= encodeBinaryJumpTable[encode_index](src->data, src->type, pos, end);
        } else
            retval |= Array_encodeBinary(src->data, src->arrayLength, src->type, pos, end);
    } else {
        /* Wrap not-builtin elements into an extensionobject */
        if(src->arrayDimensionsSize > UA_INT32_MAX)
            return UA_STATUSCODE_BADINTERNALERROR;
        size_t length = 1;
        if(isArray) {
            length = src->arrayLength;
            UA_Int32 encodedLength = (UA_Int32)src->arrayLength;
            retval |= Int32_encodeBinary(&encodedLength, pos, end);
        }
        UA_ExtensionObject eo;
        UA_ExtensionObject_init(&eo);
        eo.encoding = UA_EXTENSIONOBJECT_DECODED;
        eo.content.decoded.type = src->type;
        const UA_UInt16 memSize = src->type->memSize;
        uintptr_t ptr = (uintptr_t)src->data;
        for(size_t i = 0; i < length && retval == UA_STATUSCODE_GOOD; i++) {
            UA_Byte *oldpos = *pos;
            eo.content.decoded.data = (void*)ptr;
            retval |= ExtensionObject_encodeBinary(&eo, NULL, pos, end);
            ptr += memSize;
            if(retval == UA_STATUSCODE_BADENCODINGERROR) {
                /* exchange/send with the current buffer with chunking */
                *pos = oldpos;
                retval = exchangeBuffer(pos, &end);
                /* encode the same element in the next iteration */
                i--;
                ptr -= memSize;
            }
        }
    }

    /* Encode the dimensions */
    if(hasDimensions)
        retval |= Array_encodeBinary(src->arrayDimensions, src->arrayDimensionsSize,
                                     &UA_TYPES[UA_TYPES_INT32], pos, end);
    return retval;
}

/* The resulting variant always has the storagetype UA_VARIANT_DATA. Currently,
 we only support ns0 types (todo: attach typedescriptions to datatypenodes) */
static UA_StatusCode
Variant_decodeBinary(bufpos pos, bufend end, UA_Variant *dst, const UA_DataType *_) {
    UA_Byte encodingByte;
    UA_StatusCode retval = Byte_decodeBinary(pos, end, &encodingByte, NULL);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    UA_Boolean isArray = encodingByte & UA_VARIANT_ENCODINGMASKTYPE_ARRAY;
    size_t typeIndex = (size_t)((encodingByte & UA_VARIANT_ENCODINGMASKTYPE_TYPEID_MASK) - 1);
    if(typeIndex > 24) /* the type must be builtin (maybe wrapped in an extensionobject) */
        return UA_STATUSCODE_BADDECODINGERROR;

    if(isArray) {
        /* an array */
        dst->type = &UA_TYPES[typeIndex];
        UA_Int32 signedLength = 0;
        retval |= Int32_decodeBinary(pos, end, &signedLength);
        if(retval != UA_STATUSCODE_GOOD)
            return retval;
        retval = Array_decodeBinary(pos, end, signedLength, &dst->data, &dst->arrayLength, dst->type);
    } else if (typeIndex != UA_TYPES_EXTENSIONOBJECT) {
        /* a builtin type */
        dst->type = &UA_TYPES[typeIndex];
        retval = Array_decodeBinary(pos, end, 1, &dst->data, &dst->arrayLength, dst->type);
        dst->arrayLength = 0;
    } else {
        /* a single extensionobject */
        UA_Byte *old_pos = *pos;
        UA_NodeId typeId;
        UA_NodeId_init(&typeId);
        retval = NodeId_decodeBinary(pos, end, &typeId, NULL);
        if(retval != UA_STATUSCODE_GOOD)
            return retval;

        UA_Byte eo_encoding;
        retval = Byte_decodeBinary(pos, end, &eo_encoding, NULL);
        if(retval != UA_STATUSCODE_GOOD) {
            UA_NodeId_deleteMembers(&typeId);
            return retval;
        }

        /* search for the datatype. use extensionobject if nothing is found */
        dst->type = &UA_TYPES[UA_TYPES_EXTENSIONOBJECT];
        if(typeId.namespaceIndex == 0 && eo_encoding == UA_EXTENSIONOBJECT_ENCODED_BYTESTRING &&
           findDataType(&typeId, &dst->type) == UA_STATUSCODE_GOOD)
            *pos = old_pos;
        UA_NodeId_deleteMembers(&typeId);

        /* decode the type */
        dst->data = UA_calloc(1, dst->type->memSize);
        if(dst->data) {
            size_t decode_index = dst->type->builtin ? dst->type->typeIndex : UA_BUILTIN_TYPES_COUNT;
            retval = decodeBinaryJumpTable[decode_index](pos, end, dst->data, dst->type);
            if(retval != UA_STATUSCODE_GOOD) {
                UA_free(dst->data);
                dst->data = NULL;
            }
        } else
            retval = UA_STATUSCODE_BADOUTOFMEMORY;
    }

    /* array dimensions */
    if(isArray && (encodingByte & UA_VARIANT_ENCODINGMASKTYPE_DIMENSIONS)) {
        UA_Int32 signed_length = 0;
        retval |= Int32_decodeBinary(pos, end, &signed_length);
        if(retval == UA_STATUSCODE_GOOD)
            retval = Array_decodeBinary(pos, end, signed_length, (void**)&dst->arrayDimensions,
                                        &dst->arrayDimensionsSize, &UA_TYPES[UA_TYPES_INT32]);
    }
    if(retval != UA_STATUSCODE_GOOD)
        UA_Variant_deleteMembers(dst);
    return retval;
}

/* DataValue */
static UA_StatusCode
DataValue_encodeBinary(UA_DataValue const *src, const UA_DataType *_, bufpos pos, bufend end) {
    UA_Byte encodingMask = (UA_Byte)
        (src->hasValue | (src->hasStatus << 1) | (src->hasSourceTimestamp << 2) |
         (src->hasServerTimestamp << 3) | (src->hasSourcePicoseconds << 4) |
         (src->hasServerPicoseconds << 5));
    UA_StatusCode retval = Byte_encodeBinary(&encodingMask, NULL, pos, end);
    if(src->hasValue)
        retval |= Variant_encodeBinary(&src->value, NULL, pos, end);
    if(src->hasStatus)
        retval |= StatusCode_encodeBinary(&src->status, pos, end);
    if(src->hasSourceTimestamp)
        retval |= DateTime_encodeBinary(&src->sourceTimestamp, pos, end);
    if(src->hasSourcePicoseconds)
        retval |= UInt16_encodeBinary(&src->sourcePicoseconds, NULL, pos, end);
    if(src->hasServerTimestamp)
        retval |= DateTime_encodeBinary(&src->serverTimestamp, pos, end);
    if(src->hasServerPicoseconds)
        retval |= UInt16_encodeBinary(&src->serverPicoseconds, NULL, pos, end);
    return retval;
}

#define MAX_PICO_SECONDS 999
static UA_StatusCode
DataValue_decodeBinary(bufpos pos, bufend end, UA_DataValue *dst, const UA_DataType *_) {
    UA_Byte encodingMask;
    UA_StatusCode retval = Byte_decodeBinary(pos, end, &encodingMask, NULL);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    if(encodingMask & 0x01) {
        dst->hasValue = true;
        retval |= Variant_decodeBinary(pos, end, &dst->value, NULL);
    }
    if(encodingMask & 0x02) {
        dst->hasStatus = true;
        retval |= StatusCode_decodeBinary(pos, end, &dst->status);
    }
    if(encodingMask & 0x04) {
        dst->hasSourceTimestamp = true;
        retval |= DateTime_decodeBinary(pos, end, &dst->sourceTimestamp);
    }
    if(encodingMask & 0x08) {
        dst->hasServerTimestamp = true;
        retval |= DateTime_decodeBinary(pos, end, &dst->serverTimestamp);
    }
    if(encodingMask & 0x10) {
        dst->hasSourcePicoseconds = true;
        retval |= UInt16_decodeBinary(pos, end, &dst->sourcePicoseconds, NULL);
        if(dst->sourcePicoseconds > MAX_PICO_SECONDS)
            dst->sourcePicoseconds = MAX_PICO_SECONDS;
    }
    if(encodingMask & 0x20) {
        dst->hasServerPicoseconds = true;
        retval |= UInt16_decodeBinary(pos, end, &dst->serverPicoseconds, NULL);
        if(dst->serverPicoseconds > MAX_PICO_SECONDS)
            dst->serverPicoseconds = MAX_PICO_SECONDS;
    }
    if(retval != UA_STATUSCODE_GOOD)
        UA_DataValue_deleteMembers(dst);
    return retval;
}

/* DiagnosticInfo */
static UA_StatusCode
DiagnosticInfo_encodeBinary(const UA_DiagnosticInfo *src, const UA_DataType *_, bufpos pos, bufend end) {
    UA_Byte encodingMask = (UA_Byte)
        (src->hasSymbolicId | (src->hasNamespaceUri << 1) | (src->hasLocalizedText << 2) |
         (src->hasLocale << 3) | (src->hasAdditionalInfo << 4) | (src->hasInnerDiagnosticInfo << 5));
    UA_StatusCode retval = Byte_encodeBinary(&encodingMask, NULL, pos, end);
    if(src->hasSymbolicId)
        retval |= Int32_encodeBinary(&src->symbolicId, pos, end);
    if(src->hasNamespaceUri)
        retval |= Int32_encodeBinary(&src->namespaceUri, pos, end);
    if(src->hasLocalizedText)
        retval |= Int32_encodeBinary(&src->localizedText, pos, end);
    if(src->hasLocale)
        retval |= Int32_encodeBinary(&src->locale, pos, end);
    if(src->hasAdditionalInfo)
        retval |= String_encodeBinary(&src->additionalInfo, NULL, pos, end);
    if(src->hasInnerStatusCode)
        retval |= StatusCode_encodeBinary(&src->innerStatusCode, pos, end);
    if(src->hasInnerDiagnosticInfo)
        retval |= DiagnosticInfo_encodeBinary(src->innerDiagnosticInfo, NULL, pos, end);
    return retval;
}

static UA_StatusCode
DiagnosticInfo_decodeBinary(bufpos pos, bufend end, UA_DiagnosticInfo *dst, const UA_DataType *_) {
    UA_Byte encodingMask;
    UA_StatusCode retval = Byte_decodeBinary(pos, end, &encodingMask, NULL);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    if(encodingMask & 0x01) {
        dst->hasSymbolicId = true;
        retval |= Int32_decodeBinary(pos, end, &dst->symbolicId);
    }
    if(encodingMask & 0x02) {
        dst->hasNamespaceUri = true;
        retval |= Int32_decodeBinary(pos, end, &dst->namespaceUri);
    }
    if(encodingMask & 0x04) {
        dst->hasLocalizedText = true;
        retval |= Int32_decodeBinary(pos, end, &dst->localizedText);
    }
    if(encodingMask & 0x08) {
        dst->hasLocale = true;
        retval |= Int32_decodeBinary(pos, end, &dst->locale);
    }
    if(encodingMask & 0x10) {
        dst->hasAdditionalInfo = true;
        retval |= String_decodeBinary(pos, end, &dst->additionalInfo, NULL);
    }
    if(encodingMask & 0x20) {
        dst->hasInnerStatusCode = true;
        retval |= StatusCode_decodeBinary(pos, end, &dst->innerStatusCode);
    }
    if(encodingMask & 0x40) {
        dst->hasInnerDiagnosticInfo = true;
        /* innerDiagnosticInfo is a pointer to struct, therefore allocate */
        dst->innerDiagnosticInfo = UA_calloc(1, sizeof(UA_DiagnosticInfo));
        if(dst->innerDiagnosticInfo)
            retval |= DiagnosticInfo_decodeBinary(pos, end, dst->innerDiagnosticInfo, NULL);
        else {
            dst->hasInnerDiagnosticInfo = false;
            retval |= UA_STATUSCODE_BADOUTOFMEMORY;
        }
    }
    if(retval != UA_STATUSCODE_GOOD)
        UA_DiagnosticInfo_deleteMembers(dst);
    return retval;
}

/********************/
/* Structured Types */
/********************/

static UA_StatusCode
UA_encodeBinaryInternal(const void *src, const UA_DataType *type, bufpos pos, bufend end) {
    uintptr_t ptr = (uintptr_t)src;
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_Byte membersSize = type->membersSize;
    const UA_DataType *typelists[2] = { UA_TYPES, &type[-type->typeIndex] };
    for(size_t i = 0; i < membersSize && retval == UA_STATUSCODE_GOOD; i++) {
        const UA_DataTypeMember *member = &type->members[i];
        const UA_DataType *membertype = &typelists[!member->namespaceZero][member->memberTypeIndex];
        if(!member->isArray) {
            ptr += member->padding;
            size_t encode_index = membertype->builtin ? membertype->typeIndex : UA_BUILTIN_TYPES_COUNT;
            size_t memSize = membertype->memSize;
            UA_Byte *oldpos = *pos;
            retval |= encodeBinaryJumpTable[encode_index]((const void*)ptr, membertype, pos, end);
            ptr += memSize;
            if(retval == UA_STATUSCODE_BADENCODINGERROR) {
                /* exchange/send the buffer and try to encode the same type once more */
                *pos = oldpos;
                retval = exchangeBuffer(pos, &end);
                /* re-encode the same member on the new buffer */
                ptr -= member->padding + memSize;
                i--;
            }
        } else {
            ptr += member->padding;
            const size_t length = *((const size_t*)ptr);
            ptr += sizeof(size_t);
            retval |= Array_encodeBinary(*(void *UA_RESTRICT const *)ptr, length, membertype, pos, end);
            ptr += sizeof(void*);
        }
    }
    return retval;
}

static const UA_encodeBinarySignature encodeBinaryJumpTable[UA_BUILTIN_TYPES_COUNT + 1] = {
    (UA_encodeBinarySignature)Boolean_encodeBinary,
    (UA_encodeBinarySignature)Byte_encodeBinary, // SByte
    (UA_encodeBinarySignature)Byte_encodeBinary,
    (UA_encodeBinarySignature)UInt16_encodeBinary, // Int16
    (UA_encodeBinarySignature)UInt16_encodeBinary,
    (UA_encodeBinarySignature)UInt32_encodeBinary, // Int32
    (UA_encodeBinarySignature)UInt32_encodeBinary,
    (UA_encodeBinarySignature)UInt64_encodeBinary, // Int64
    (UA_encodeBinarySignature)UInt64_encodeBinary,
    (UA_encodeBinarySignature)Float_encodeBinary,
    (UA_encodeBinarySignature)Double_encodeBinary,
    (UA_encodeBinarySignature)String_encodeBinary,
    (UA_encodeBinarySignature)UInt64_encodeBinary, // DateTime
    (UA_encodeBinarySignature)Guid_encodeBinary,
    (UA_encodeBinarySignature)String_encodeBinary, // ByteString
    (UA_encodeBinarySignature)String_encodeBinary, // XmlElement
    (UA_encodeBinarySignature)NodeId_encodeBinary,
    (UA_encodeBinarySignature)ExpandedNodeId_encodeBinary,
    (UA_encodeBinarySignature)UInt32_encodeBinary, // StatusCode
    (UA_encodeBinarySignature)UA_encodeBinaryInternal, // QualifiedName
    (UA_encodeBinarySignature)LocalizedText_encodeBinary,
    (UA_encodeBinarySignature)ExtensionObject_encodeBinary,
    (UA_encodeBinarySignature)DataValue_encodeBinary,
    (UA_encodeBinarySignature)Variant_encodeBinary,
    (UA_encodeBinarySignature)DiagnosticInfo_encodeBinary,
    (UA_encodeBinarySignature)UA_encodeBinaryInternal,
};

UA_StatusCode
UA_encodeBinary(const void *src, const UA_DataType *type, UA_exchangeEncodeBuffer callback,
                void *handle, UA_ByteString *dst, size_t *offset) {
    UA_Byte *pos = &dst->data[*offset];
    UA_Byte *end = &dst->data[dst->length];
    encodeBuf = dst;
    exchangeBufferCallback = callback;
    exchangeBufferCallbackHandle = handle;
    UA_StatusCode retval = UA_encodeBinaryInternal(src, type, &pos, end);
    *offset = (size_t)(pos - dst->data) / sizeof(UA_Byte);
    return retval;
}

static UA_StatusCode
UA_decodeBinaryInternal(bufpos pos, bufend end, void *dst, const UA_DataType *type) {
    uintptr_t ptr = (uintptr_t)dst;
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_Byte membersSize = type->membersSize;
    const UA_DataType *typelists[2] = { UA_TYPES, &type[-type->typeIndex] };
    for(size_t i = 0; i < membersSize; i++) {
        const UA_DataTypeMember *member = &type->members[i];
        const UA_DataType *membertype = &typelists[!member->namespaceZero][member->memberTypeIndex];
        if(!member->isArray) {
            ptr += member->padding;
            size_t fi = membertype->builtin ? membertype->typeIndex : UA_BUILTIN_TYPES_COUNT;
            size_t memSize = membertype->memSize;
            retval |= decodeBinaryJumpTable[fi](pos, end, (void *UA_RESTRICT)ptr, membertype);
            ptr += memSize;
        } else {
            ptr += member->padding;
            size_t *length = (size_t*)ptr;
            ptr += sizeof(size_t);
            UA_Int32 slength = -1;
            retval |= Int32_decodeBinary(pos, end, &slength);
            retval |= Array_decodeBinary(pos, end, slength, (void *UA_RESTRICT *UA_RESTRICT)ptr,
                                         length, membertype);
            ptr += sizeof(void*);
        }
    }
    if(retval != UA_STATUSCODE_GOOD)
        UA_deleteMembers(dst, type);
    return retval;
}

static const UA_decodeBinarySignature decodeBinaryJumpTable[UA_BUILTIN_TYPES_COUNT + 1] = {
    (UA_decodeBinarySignature)Boolean_decodeBinary,
    (UA_decodeBinarySignature)Byte_decodeBinary, // SByte
    (UA_decodeBinarySignature)Byte_decodeBinary,
    (UA_decodeBinarySignature)UInt16_decodeBinary, // Int16
    (UA_decodeBinarySignature)UInt16_decodeBinary,
    (UA_decodeBinarySignature)UInt32_decodeBinary, // Int32
    (UA_decodeBinarySignature)UInt32_decodeBinary,
    (UA_decodeBinarySignature)UInt64_decodeBinary, // Int64
    (UA_decodeBinarySignature)UInt64_decodeBinary,
    (UA_decodeBinarySignature)Float_decodeBinary,
    (UA_decodeBinarySignature)Double_decodeBinary,
    (UA_decodeBinarySignature)String_decodeBinary,
    (UA_decodeBinarySignature)UInt64_decodeBinary, // DateTime
    (UA_decodeBinarySignature)Guid_decodeBinary,
    (UA_decodeBinarySignature)String_decodeBinary, // ByteString
    (UA_decodeBinarySignature)String_decodeBinary, // XmlElement
    (UA_decodeBinarySignature)NodeId_decodeBinary,
    (UA_decodeBinarySignature)ExpandedNodeId_decodeBinary,
    (UA_decodeBinarySignature)UInt32_decodeBinary, // StatusCode
    (UA_decodeBinarySignature)UA_decodeBinaryInternal, // QualifiedName
    (UA_decodeBinarySignature)LocalizedText_decodeBinary,
    (UA_decodeBinarySignature)ExtensionObject_decodeBinary,
    (UA_decodeBinarySignature)DataValue_decodeBinary,
    (UA_decodeBinarySignature)Variant_decodeBinary,
    (UA_decodeBinarySignature)DiagnosticInfo_decodeBinary,
    (UA_decodeBinarySignature)UA_decodeBinaryInternal
};

UA_StatusCode
UA_decodeBinary(const UA_ByteString *src, size_t *offset, void *dst, const UA_DataType *type) {
    memset(dst, 0, type->memSize); // init
    UA_Byte *pos = &src->data[*offset];
    UA_Byte *end = &src->data[src->length];
    UA_StatusCode retval = UA_decodeBinaryInternal(&pos, end, dst, type);
    *offset = (size_t)(pos - src->data) / sizeof(UA_Byte);
    return retval;
}

/******************/
/* CalcSizeBinary */
/******************/

static size_t
Array_calcSizeBinary(const void *src, size_t length, const UA_DataType *type) {
    size_t s = 4; // length
    if(type->overlayable) {
        s += type->memSize * length;
        return s;
    }
    uintptr_t ptr = (uintptr_t)src;
    size_t encode_index = type->builtin ? type->typeIndex : UA_BUILTIN_TYPES_COUNT;
    for(size_t i = 0; i < length; i++) {
        s += calcSizeBinaryJumpTable[encode_index]((const void*)ptr, type);
        ptr += type->memSize;
    }
    return s;
}

static size_t calcSizeBinaryMemSize(const void *UA_RESTRICT p, const UA_DataType *type) {
    return type->memSize;
}

static size_t String_calcSizeBinary(const UA_String *UA_RESTRICT p, const UA_DataType *_) {
    return 4 + p->length;
}

static size_t Guid_calcSizeBinary(const UA_Guid *UA_RESTRICT p, const UA_DataType *_) {
    return 16;
}

static size_t
NodeId_calcSizeBinary(const UA_NodeId *UA_RESTRICT src, const UA_DataType *_) {
    size_t s = 1; // encoding byte
    switch (src->identifierType) {
    case UA_NODEIDTYPE_NUMERIC:
        if(src->identifier.numeric > UA_UINT16_MAX || src->namespaceIndex > UA_BYTE_MAX) {
            s += 6;
        } else if(src->identifier.numeric > UA_BYTE_MAX || src->namespaceIndex > 0) {
            s += 3;
        } else {
            s += 1;
        }
        break;
    case UA_NODEIDTYPE_BYTESTRING:
    case UA_NODEIDTYPE_STRING:
        s += 2;
        s += String_calcSizeBinary(&src->identifier.string, NULL);
        break;
    case UA_NODEIDTYPE_GUID:
        s += 18;
        break;
    default:
        return 0;
    }
    return s;
}

static size_t
ExpandedNodeId_calcSizeBinary(const UA_ExpandedNodeId *src, const UA_DataType *_) {
    size_t s = NodeId_calcSizeBinary(&src->nodeId, NULL);
    if(src->namespaceUri.length > 0)
        s += String_calcSizeBinary(&src->namespaceUri, NULL);
    if(src->serverIndex > 0)
        s += 4;
    return s;
}

static size_t
LocalizedText_calcSizeBinary(const UA_LocalizedText *src, UA_DataType *_) {
    size_t s = 1; // encoding byte
    if(src->locale.data)
        s += String_calcSizeBinary(&src->locale, NULL);
    if(src->text.data)
        s += String_calcSizeBinary(&src->text, NULL);
    return s;
}

static size_t
ExtensionObject_calcSizeBinary(const UA_ExtensionObject *src, UA_DataType *_) {
    size_t s = 1; // encoding byte
    if(src->encoding > UA_EXTENSIONOBJECT_ENCODED_XML) {
        if(!src->content.decoded.type || !src->content.decoded.data)
            return 0;
        if(src->content.decoded.type->typeId.identifierType != UA_NODEIDTYPE_NUMERIC)
            return 0;
        s += NodeId_calcSizeBinary(&src->content.decoded.type->typeId, NULL);
        s += 4; // length
        const UA_DataType *type = src->content.decoded.type;
        size_t encode_index = type->builtin ? type->typeIndex : UA_BUILTIN_TYPES_COUNT;
        s += calcSizeBinaryJumpTable[encode_index](src->content.decoded.data, type);
    } else {
        s += NodeId_calcSizeBinary(&src->content.encoded.typeId, NULL);
        switch (src->encoding) {
        case UA_EXTENSIONOBJECT_ENCODED_NOBODY:
            break;
        case UA_EXTENSIONOBJECT_ENCODED_BYTESTRING:
        case UA_EXTENSIONOBJECT_ENCODED_XML:
            s += String_calcSizeBinary(&src->content.encoded.body, NULL);
            break;
        default:
            return 0;
        }
    }
    return s;
}

static size_t
Variant_calcSizeBinary(UA_Variant const *src, UA_DataType *_) {
    size_t s = 1; // encoding byte

    if(!src->type)
        return 0;
    UA_Boolean isArray = src->arrayLength > 0 || src->data <= UA_EMPTY_ARRAY_SENTINEL;
    UA_Boolean hasDimensions = isArray && src->arrayDimensionsSize > 0;
    UA_Boolean isBuiltin = src->type->builtin;

    UA_NodeId typeId;
    UA_NodeId_init(&typeId);
    size_t encode_index = src->type->typeIndex;
    if(!isBuiltin) {
        encode_index = UA_BUILTIN_TYPES_COUNT;
        typeId = src->type->typeId;
        if(typeId.identifierType != UA_NODEIDTYPE_NUMERIC)
            return 0;
    }

    size_t length = src->arrayLength;
    if(isArray) {
        s += 4;
    } else
        length = 1;

    uintptr_t ptr = (uintptr_t)src->data;
    size_t memSize = src->type->memSize;
    for(size_t i = 0; i < length; i++) {
        if(!isBuiltin) {
            /* The type is wrapped inside an extensionobject */
            s += NodeId_calcSizeBinary(&typeId, NULL);
            s += 1 + 4; // encoding byte + length
        }
        s += calcSizeBinaryJumpTable[encode_index]((const void*)ptr, src->type);
        ptr += memSize;
    }

    if(hasDimensions)
        s += Array_calcSizeBinary(src->arrayDimensions, src->arrayDimensionsSize,
                                  &UA_TYPES[UA_TYPES_INT32]);
    return s;
}

static size_t
DataValue_calcSizeBinary(const UA_DataValue *src, UA_DataType *_) {
    size_t s = 1; // encoding byte
    if(src->hasValue)
        s += Variant_calcSizeBinary(&src->value, NULL);
    if(src->hasStatus)
        s += 4;
    if(src->hasSourceTimestamp)
        s += 8;
    if(src->hasSourcePicoseconds)
        s += 2;
    if(src->hasServerTimestamp)
        s += 8;
    if(src->hasServerPicoseconds)
        s += 2;
    return s;
}

static size_t
DiagnosticInfo_calcSizeBinary(const UA_DiagnosticInfo *src, UA_DataType *_) {
    size_t s = 1; // encoding byte
    if(src->hasSymbolicId)
        s += 4;
    if(src->hasNamespaceUri)
        s += 4;
    if(src->hasLocalizedText)
        s += 4;
    if(src->hasLocale)
        s += 4;
    if(src->hasAdditionalInfo)
        s += String_calcSizeBinary(&src->additionalInfo, NULL);
    if(src->hasInnerStatusCode)
        s += 4;
    if(src->hasInnerDiagnosticInfo)
        s += DiagnosticInfo_calcSizeBinary(src->innerDiagnosticInfo, NULL);
    return s;
}

static const UA_calcSizeBinarySignature calcSizeBinaryJumpTable[UA_BUILTIN_TYPES_COUNT + 1] = {
    (UA_calcSizeBinarySignature)calcSizeBinaryMemSize, // Boolean
    (UA_calcSizeBinarySignature)calcSizeBinaryMemSize, // Byte
    (UA_calcSizeBinarySignature)calcSizeBinaryMemSize,
    (UA_calcSizeBinarySignature)calcSizeBinaryMemSize, // Int16
    (UA_calcSizeBinarySignature)calcSizeBinaryMemSize,
    (UA_calcSizeBinarySignature)calcSizeBinaryMemSize, // Int32
    (UA_calcSizeBinarySignature)calcSizeBinaryMemSize,
    (UA_calcSizeBinarySignature)calcSizeBinaryMemSize, // Int64
    (UA_calcSizeBinarySignature)calcSizeBinaryMemSize,
    (UA_calcSizeBinarySignature)calcSizeBinaryMemSize, // Float
    (UA_calcSizeBinarySignature)calcSizeBinaryMemSize, // Double
    (UA_calcSizeBinarySignature)String_calcSizeBinary,
    (UA_calcSizeBinarySignature)calcSizeBinaryMemSize, // DateTime
    (UA_calcSizeBinarySignature)Guid_calcSizeBinary,
    (UA_calcSizeBinarySignature)String_calcSizeBinary, // ByteString
    (UA_calcSizeBinarySignature)String_calcSizeBinary, // XmlElement
    (UA_calcSizeBinarySignature)NodeId_calcSizeBinary,
    (UA_calcSizeBinarySignature)ExpandedNodeId_calcSizeBinary,
    (UA_calcSizeBinarySignature)calcSizeBinaryMemSize, // StatusCode
    (UA_calcSizeBinarySignature)UA_calcSizeBinary, // QualifiedName
    (UA_calcSizeBinarySignature)LocalizedText_calcSizeBinary,
    (UA_calcSizeBinarySignature)ExtensionObject_calcSizeBinary,
    (UA_calcSizeBinarySignature)DataValue_calcSizeBinary,
    (UA_calcSizeBinarySignature)Variant_calcSizeBinary,
    (UA_calcSizeBinarySignature)DiagnosticInfo_calcSizeBinary,
    (UA_calcSizeBinarySignature)UA_calcSizeBinary
};

size_t UA_calcSizeBinary(void *p, const UA_DataType *type) {
    size_t s = 0;
    uintptr_t ptr = (uintptr_t)p;
    UA_Byte membersSize = type->membersSize;
    const UA_DataType *typelists[2] = { UA_TYPES, &type[-type->typeIndex] };
    for(size_t i = 0; i < membersSize; i++) {
        const UA_DataTypeMember *member = &type->members[i];
        const UA_DataType *membertype = &typelists[!member->namespaceZero][member->memberTypeIndex];
        if(!member->isArray) {
            ptr += member->padding;
            size_t encode_index = membertype->builtin ? membertype->typeIndex : UA_BUILTIN_TYPES_COUNT;
            s += calcSizeBinaryJumpTable[encode_index]((const void*)ptr, membertype);
            ptr += membertype->memSize;
        } else {
            ptr += member->padding;
            const size_t length = *((const size_t*)ptr);
            ptr += sizeof(size_t);
            s += Array_calcSizeBinary(*(void *UA_RESTRICT const *)ptr, length, membertype);
            ptr += sizeof(void*);
        }
    }
    return s;
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/build/src_generated/ua_types_generated.c" ***********************************/

/* Generated from Opc.Ua.Types.bsd with script /home/travis/build/open62541/open62541/tools/generate_datatypes.py
 * on host testing-worker-linux-docker-8551a97d-3382-linux-4 by user travis at 2016-05-18 08:48:03 */
 

/* Boolean */
static UA_DataTypeMember Boolean_members[1] = {
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* SByte */
static UA_DataTypeMember SByte_members[1] = {
  { .memberTypeIndex = UA_TYPES_SBYTE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* Byte */
static UA_DataTypeMember Byte_members[1] = {
  { .memberTypeIndex = UA_TYPES_BYTE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* Int16 */
static UA_DataTypeMember Int16_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT16,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* UInt16 */
static UA_DataTypeMember UInt16_members[1] = {
  { .memberTypeIndex = UA_TYPES_UINT16,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* Int32 */
static UA_DataTypeMember Int32_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* UInt32 */
static UA_DataTypeMember UInt32_members[1] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* Int64 */
static UA_DataTypeMember Int64_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT64,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* UInt64 */
static UA_DataTypeMember UInt64_members[1] = {
  { .memberTypeIndex = UA_TYPES_UINT64,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* Float */
static UA_DataTypeMember Float_members[1] = {
  { .memberTypeIndex = UA_TYPES_FLOAT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* Double */
static UA_DataTypeMember Double_members[1] = {
  { .memberTypeIndex = UA_TYPES_DOUBLE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* String */
static UA_DataTypeMember String_members[1] = {
  { .memberTypeIndex = UA_TYPES_BYTE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = true
  },};

/* DateTime */
static UA_DataTypeMember DateTime_members[1] = {
  { .memberTypeIndex = UA_TYPES_DATETIME,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* Guid */
static UA_DataTypeMember Guid_members[1] = {
  { .memberTypeIndex = UA_TYPES_GUID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* ByteString */
static UA_DataTypeMember ByteString_members[1] = {
  { .memberTypeIndex = UA_TYPES_BYTE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = true
  },};

/* XmlElement */
static UA_DataTypeMember XmlElement_members[1] = {
  { .memberTypeIndex = UA_TYPES_BYTE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = true
  },};

/* NodeId */
static UA_DataTypeMember NodeId_members[1] = {
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* ExpandedNodeId */
static UA_DataTypeMember ExpandedNodeId_members[1] = {
  { .memberTypeIndex = UA_TYPES_EXPANDEDNODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* StatusCode */
static UA_DataTypeMember StatusCode_members[1] = {
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* QualifiedName */
static UA_DataTypeMember QualifiedName_members[2] = {
  { .memberTypeIndex = UA_TYPES_INT16,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "namespaceIndex",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "name",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QualifiedName, name) - offsetof(UA_QualifiedName, namespaceIndex) - sizeof(UA_Int16),
    .isArray = false
  },};

/* LocalizedText */
static UA_DataTypeMember LocalizedText_members[1] = {
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* ExtensionObject */
static UA_DataTypeMember ExtensionObject_members[1] = {
  { .memberTypeIndex = UA_TYPES_EXTENSIONOBJECT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* DataValue */
static UA_DataTypeMember DataValue_members[1] = {
  { .memberTypeIndex = UA_TYPES_DATAVALUE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* Variant */
static UA_DataTypeMember Variant_members[1] = {
  { .memberTypeIndex = UA_TYPES_VARIANT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* DiagnosticInfo */
static UA_DataTypeMember DiagnosticInfo_members[1] = {
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* SignedSoftwareCertificate */
static UA_DataTypeMember SignedSoftwareCertificate_members[2] = {
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "certificateData",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "signature",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_SignedSoftwareCertificate, signature) - offsetof(UA_SignedSoftwareCertificate, certificateData) - sizeof(UA_ByteString),
    .isArray = false
  },};

/* BrowsePathTarget */
static UA_DataTypeMember BrowsePathTarget_members[2] = {
  { .memberTypeIndex = UA_TYPES_EXPANDEDNODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "targetId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "remainingPathIndex",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowsePathTarget, remainingPathIndex) - offsetof(UA_BrowsePathTarget, targetId) - sizeof(UA_ExpandedNodeId),
    .isArray = false
  },};

/* ViewAttributes */
static UA_DataTypeMember ViewAttributes_members[7] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "specifiedAttributes",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "displayName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ViewAttributes, displayName) - offsetof(UA_ViewAttributes, specifiedAttributes) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "description",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ViewAttributes, description) - offsetof(UA_ViewAttributes, displayName) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "writeMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ViewAttributes, writeMask) - offsetof(UA_ViewAttributes, description) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userWriteMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ViewAttributes, userWriteMask) - offsetof(UA_ViewAttributes, writeMask) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "containsNoLoops",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ViewAttributes, containsNoLoops) - offsetof(UA_ViewAttributes, userWriteMask) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "eventNotifier",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ViewAttributes, eventNotifier) - offsetof(UA_ViewAttributes, containsNoLoops) - sizeof(UA_Boolean),
    .isArray = false
  },};

/* BrowseResultMask */
static UA_DataTypeMember BrowseResultMask_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* RequestHeader */
static UA_DataTypeMember RequestHeader_members[7] = {
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "authenticationToken",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DATETIME,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "timestamp",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_RequestHeader, timestamp) - offsetof(UA_RequestHeader, authenticationToken) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHandle",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_RequestHeader, requestHandle) - offsetof(UA_RequestHeader, timestamp) - sizeof(UA_DateTime),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "returnDiagnostics",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_RequestHeader, returnDiagnostics) - offsetof(UA_RequestHeader, requestHandle) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "auditEntryId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_RequestHeader, auditEntryId) - offsetof(UA_RequestHeader, returnDiagnostics) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "timeoutHint",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_RequestHeader, timeoutHint) - offsetof(UA_RequestHeader, auditEntryId) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXTENSIONOBJECT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "additionalHeader",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_RequestHeader, additionalHeader) - offsetof(UA_RequestHeader, timeoutHint) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* MonitoredItemModifyResult */
static UA_DataTypeMember MonitoredItemModifyResult_members[4] = {
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "statusCode",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DOUBLE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "revisedSamplingInterval",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoredItemModifyResult, revisedSamplingInterval) - offsetof(UA_MonitoredItemModifyResult, statusCode) - sizeof(UA_StatusCode),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "revisedQueueSize",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoredItemModifyResult, revisedQueueSize) - offsetof(UA_MonitoredItemModifyResult, revisedSamplingInterval) - sizeof(UA_Double),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXTENSIONOBJECT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "filterResult",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoredItemModifyResult, filterResult) - offsetof(UA_MonitoredItemModifyResult, revisedQueueSize) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* ViewDescription */
static UA_DataTypeMember ViewDescription_members[3] = {
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "viewId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DATETIME,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "timestamp",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ViewDescription, timestamp) - offsetof(UA_ViewDescription, viewId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "viewVersion",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ViewDescription, viewVersion) - offsetof(UA_ViewDescription, timestamp) - sizeof(UA_DateTime),
    .isArray = false
  },};

/* CloseSecureChannelRequest */
static UA_DataTypeMember CloseSecureChannelRequest_members[1] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* AddNodesResult */
static UA_DataTypeMember AddNodesResult_members[2] = {
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "statusCode",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "addedNodeId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddNodesResult, addedNodeId) - offsetof(UA_AddNodesResult, statusCode) - sizeof(UA_StatusCode),
    .isArray = false
  },};

/* VariableAttributes */
static UA_DataTypeMember VariableAttributes_members[13] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "specifiedAttributes",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "displayName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableAttributes, displayName) - offsetof(UA_VariableAttributes, specifiedAttributes) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "description",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableAttributes, description) - offsetof(UA_VariableAttributes, displayName) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "writeMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableAttributes, writeMask) - offsetof(UA_VariableAttributes, description) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userWriteMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableAttributes, userWriteMask) - offsetof(UA_VariableAttributes, writeMask) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_VARIANT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "value",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableAttributes, value) - offsetof(UA_VariableAttributes, userWriteMask) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "dataType",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableAttributes, dataType) - offsetof(UA_VariableAttributes, value) - sizeof(UA_Variant),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "valueRank",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableAttributes, valueRank) - offsetof(UA_VariableAttributes, dataType) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "arrayDimensions",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableAttributes, arrayDimensionsSize) - offsetof(UA_VariableAttributes, valueRank) - sizeof(UA_Int32),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_BYTE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "accessLevel",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableAttributes, accessLevel) - offsetof(UA_VariableAttributes, arrayDimensions) - sizeof(void*),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userAccessLevel",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableAttributes, userAccessLevel) - offsetof(UA_VariableAttributes, accessLevel) - sizeof(UA_Byte),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DOUBLE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "minimumSamplingInterval",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableAttributes, minimumSamplingInterval) - offsetof(UA_VariableAttributes, userAccessLevel) - sizeof(UA_Byte),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "historizing",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableAttributes, historizing) - offsetof(UA_VariableAttributes, minimumSamplingInterval) - sizeof(UA_Double),
    .isArray = false
  },};

/* NotificationMessage */
static UA_DataTypeMember NotificationMessage_members[3] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "sequenceNumber",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DATETIME,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "publishTime",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_NotificationMessage, publishTime) - offsetof(UA_NotificationMessage, sequenceNumber) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXTENSIONOBJECT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "notificationData",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_NotificationMessage, notificationDataSize) - offsetof(UA_NotificationMessage, publishTime) - sizeof(UA_DateTime),
    .isArray = true
  },};

/* NodeAttributesMask */
static UA_DataTypeMember NodeAttributesMask_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* MonitoringMode */
static UA_DataTypeMember MonitoringMode_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* CallMethodResult */
static UA_DataTypeMember CallMethodResult_members[4] = {
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "statusCode",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "inputArgumentResults",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CallMethodResult, inputArgumentResultsSize) - offsetof(UA_CallMethodResult, statusCode) - sizeof(UA_StatusCode),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "inputArgumentDiagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CallMethodResult, inputArgumentDiagnosticInfosSize) - offsetof(UA_CallMethodResult, inputArgumentResults) - sizeof(void*),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_VARIANT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "outputArguments",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CallMethodResult, outputArgumentsSize) - offsetof(UA_CallMethodResult, inputArgumentDiagnosticInfos) - sizeof(void*),
    .isArray = true
  },};

/* ParsingResult */
static UA_DataTypeMember ParsingResult_members[3] = {
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "statusCode",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "dataStatusCodes",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ParsingResult, dataStatusCodesSize) - offsetof(UA_ParsingResult, statusCode) - sizeof(UA_StatusCode),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "dataDiagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ParsingResult, dataDiagnosticInfosSize) - offsetof(UA_ParsingResult, dataStatusCodes) - sizeof(void*),
    .isArray = true
  },};

/* RelativePathElement */
static UA_DataTypeMember RelativePathElement_members[4] = {
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "referenceTypeId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "isInverse",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_RelativePathElement, isInverse) - offsetof(UA_RelativePathElement, referenceTypeId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "includeSubtypes",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_RelativePathElement, includeSubtypes) - offsetof(UA_RelativePathElement, isInverse) - sizeof(UA_Boolean),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_QUALIFIEDNAME,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "targetName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_RelativePathElement, targetName) - offsetof(UA_RelativePathElement, includeSubtypes) - sizeof(UA_Boolean),
    .isArray = false
  },};

/* BrowseDirection */
static UA_DataTypeMember BrowseDirection_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* CallMethodRequest */
static UA_DataTypeMember CallMethodRequest_members[3] = {
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "objectId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "methodId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CallMethodRequest, methodId) - offsetof(UA_CallMethodRequest, objectId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_VARIANT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "inputArguments",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CallMethodRequest, inputArgumentsSize) - offsetof(UA_CallMethodRequest, methodId) - sizeof(UA_NodeId),
    .isArray = true
  },};

/* ServerState */
static UA_DataTypeMember ServerState_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* UnregisterNodesRequest */
static UA_DataTypeMember UnregisterNodesRequest_members[2] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodesToUnregister",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_UnregisterNodesRequest, nodesToUnregisterSize) - offsetof(UA_UnregisterNodesRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = true
  },};

/* ContentFilterElementResult */
static UA_DataTypeMember ContentFilterElementResult_members[3] = {
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "statusCode",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "operandStatusCodes",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ContentFilterElementResult, operandStatusCodesSize) - offsetof(UA_ContentFilterElementResult, statusCode) - sizeof(UA_StatusCode),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "operandDiagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ContentFilterElementResult, operandDiagnosticInfosSize) - offsetof(UA_ContentFilterElementResult, operandStatusCodes) - sizeof(void*),
    .isArray = true
  },};

/* QueryDataSet */
static UA_DataTypeMember QueryDataSet_members[3] = {
  { .memberTypeIndex = UA_TYPES_EXPANDEDNODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodeId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXPANDEDNODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "typeDefinitionNode",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryDataSet, typeDefinitionNode) - offsetof(UA_QueryDataSet, nodeId) - sizeof(UA_ExpandedNodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_VARIANT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "values",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryDataSet, valuesSize) - offsetof(UA_QueryDataSet, typeDefinitionNode) - sizeof(UA_ExpandedNodeId),
    .isArray = true
  },};

/* SetPublishingModeRequest */
static UA_DataTypeMember SetPublishingModeRequest_members[3] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "publishingEnabled",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_SetPublishingModeRequest, publishingEnabled) - offsetof(UA_SetPublishingModeRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "subscriptionIds",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_SetPublishingModeRequest, subscriptionIdsSize) - offsetof(UA_SetPublishingModeRequest, publishingEnabled) - sizeof(UA_Boolean),
    .isArray = true
  },};

/* TimestampsToReturn */
static UA_DataTypeMember TimestampsToReturn_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* CallRequest */
static UA_DataTypeMember CallRequest_members[2] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_CALLMETHODREQUEST,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "methodsToCall",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CallRequest, methodsToCallSize) - offsetof(UA_CallRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = true
  },};

/* MethodAttributes */
static UA_DataTypeMember MethodAttributes_members[7] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "specifiedAttributes",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "displayName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MethodAttributes, displayName) - offsetof(UA_MethodAttributes, specifiedAttributes) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "description",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MethodAttributes, description) - offsetof(UA_MethodAttributes, displayName) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "writeMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MethodAttributes, writeMask) - offsetof(UA_MethodAttributes, description) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userWriteMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MethodAttributes, userWriteMask) - offsetof(UA_MethodAttributes, writeMask) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "executable",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MethodAttributes, executable) - offsetof(UA_MethodAttributes, userWriteMask) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userExecutable",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MethodAttributes, userExecutable) - offsetof(UA_MethodAttributes, executable) - sizeof(UA_Boolean),
    .isArray = false
  },};

/* DeleteReferencesItem */
static UA_DataTypeMember DeleteReferencesItem_members[5] = {
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "sourceNodeId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "referenceTypeId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteReferencesItem, referenceTypeId) - offsetof(UA_DeleteReferencesItem, sourceNodeId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "isForward",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteReferencesItem, isForward) - offsetof(UA_DeleteReferencesItem, referenceTypeId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXPANDEDNODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "targetNodeId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteReferencesItem, targetNodeId) - offsetof(UA_DeleteReferencesItem, isForward) - sizeof(UA_Boolean),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "deleteBidirectional",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteReferencesItem, deleteBidirectional) - offsetof(UA_DeleteReferencesItem, targetNodeId) - sizeof(UA_ExpandedNodeId),
    .isArray = false
  },};

/* WriteValue */
static UA_DataTypeMember WriteValue_members[4] = {
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodeId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "attributeId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_WriteValue, attributeId) - offsetof(UA_WriteValue, nodeId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "indexRange",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_WriteValue, indexRange) - offsetof(UA_WriteValue, attributeId) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DATAVALUE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "value",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_WriteValue, value) - offsetof(UA_WriteValue, indexRange) - sizeof(UA_String),
    .isArray = false
  },};

/* MonitoredItemCreateResult */
static UA_DataTypeMember MonitoredItemCreateResult_members[5] = {
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "statusCode",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "monitoredItemId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoredItemCreateResult, monitoredItemId) - offsetof(UA_MonitoredItemCreateResult, statusCode) - sizeof(UA_StatusCode),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DOUBLE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "revisedSamplingInterval",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoredItemCreateResult, revisedSamplingInterval) - offsetof(UA_MonitoredItemCreateResult, monitoredItemId) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "revisedQueueSize",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoredItemCreateResult, revisedQueueSize) - offsetof(UA_MonitoredItemCreateResult, revisedSamplingInterval) - sizeof(UA_Double),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXTENSIONOBJECT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "filterResult",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoredItemCreateResult, filterResult) - offsetof(UA_MonitoredItemCreateResult, revisedQueueSize) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* MessageSecurityMode */
static UA_DataTypeMember MessageSecurityMode_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* MonitoringParameters */
static UA_DataTypeMember MonitoringParameters_members[5] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "clientHandle",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DOUBLE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "samplingInterval",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoringParameters, samplingInterval) - offsetof(UA_MonitoringParameters, clientHandle) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXTENSIONOBJECT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "filter",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoringParameters, filter) - offsetof(UA_MonitoringParameters, samplingInterval) - sizeof(UA_Double),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "queueSize",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoringParameters, queueSize) - offsetof(UA_MonitoringParameters, filter) - sizeof(UA_ExtensionObject),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "discardOldest",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoringParameters, discardOldest) - offsetof(UA_MonitoringParameters, queueSize) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* SignatureData */
static UA_DataTypeMember SignatureData_members[2] = {
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "algorithm",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "signature",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_SignatureData, signature) - offsetof(UA_SignatureData, algorithm) - sizeof(UA_String),
    .isArray = false
  },};

/* ReferenceNode */
static UA_DataTypeMember ReferenceNode_members[3] = {
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "referenceTypeId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "isInverse",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceNode, isInverse) - offsetof(UA_ReferenceNode, referenceTypeId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXPANDEDNODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "targetId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceNode, targetId) - offsetof(UA_ReferenceNode, isInverse) - sizeof(UA_Boolean),
    .isArray = false
  },};

/* Argument */
static UA_DataTypeMember Argument_members[5] = {
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "name",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "dataType",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_Argument, dataType) - offsetof(UA_Argument, name) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "valueRank",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_Argument, valueRank) - offsetof(UA_Argument, dataType) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "arrayDimensions",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_Argument, arrayDimensionsSize) - offsetof(UA_Argument, valueRank) - sizeof(UA_Int32),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "description",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_Argument, description) - offsetof(UA_Argument, arrayDimensions) - sizeof(void*),
    .isArray = false
  },};

/* UserIdentityToken */
static UA_DataTypeMember UserIdentityToken_members[1] = {
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "policyId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* ObjectTypeAttributes */
static UA_DataTypeMember ObjectTypeAttributes_members[6] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "specifiedAttributes",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "displayName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ObjectTypeAttributes, displayName) - offsetof(UA_ObjectTypeAttributes, specifiedAttributes) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "description",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ObjectTypeAttributes, description) - offsetof(UA_ObjectTypeAttributes, displayName) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "writeMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ObjectTypeAttributes, writeMask) - offsetof(UA_ObjectTypeAttributes, description) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userWriteMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ObjectTypeAttributes, userWriteMask) - offsetof(UA_ObjectTypeAttributes, writeMask) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "isAbstract",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ObjectTypeAttributes, isAbstract) - offsetof(UA_ObjectTypeAttributes, userWriteMask) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* SecurityTokenRequestType */
static UA_DataTypeMember SecurityTokenRequestType_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* BuildInfo */
static UA_DataTypeMember BuildInfo_members[6] = {
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "productUri",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "manufacturerName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BuildInfo, manufacturerName) - offsetof(UA_BuildInfo, productUri) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "productName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BuildInfo, productName) - offsetof(UA_BuildInfo, manufacturerName) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "softwareVersion",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BuildInfo, softwareVersion) - offsetof(UA_BuildInfo, productName) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "buildNumber",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BuildInfo, buildNumber) - offsetof(UA_BuildInfo, softwareVersion) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DATETIME,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "buildDate",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BuildInfo, buildDate) - offsetof(UA_BuildInfo, buildNumber) - sizeof(UA_String),
    .isArray = false
  },};

/* NodeClass */
static UA_DataTypeMember NodeClass_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* ChannelSecurityToken */
static UA_DataTypeMember ChannelSecurityToken_members[4] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "channelId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "tokenId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ChannelSecurityToken, tokenId) - offsetof(UA_ChannelSecurityToken, channelId) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DATETIME,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "createdAt",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ChannelSecurityToken, createdAt) - offsetof(UA_ChannelSecurityToken, tokenId) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "revisedLifetime",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ChannelSecurityToken, revisedLifetime) - offsetof(UA_ChannelSecurityToken, createdAt) - sizeof(UA_DateTime),
    .isArray = false
  },};

/* MonitoredItemNotification */
static UA_DataTypeMember MonitoredItemNotification_members[2] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "clientHandle",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DATAVALUE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "value",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoredItemNotification, value) - offsetof(UA_MonitoredItemNotification, clientHandle) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* DeleteNodesItem */
static UA_DataTypeMember DeleteNodesItem_members[2] = {
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodeId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "deleteTargetReferences",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteNodesItem, deleteTargetReferences) - offsetof(UA_DeleteNodesItem, nodeId) - sizeof(UA_NodeId),
    .isArray = false
  },};

/* SubscriptionAcknowledgement */
static UA_DataTypeMember SubscriptionAcknowledgement_members[2] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "subscriptionId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "sequenceNumber",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_SubscriptionAcknowledgement, sequenceNumber) - offsetof(UA_SubscriptionAcknowledgement, subscriptionId) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* ReadValueId */
static UA_DataTypeMember ReadValueId_members[4] = {
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodeId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "attributeId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReadValueId, attributeId) - offsetof(UA_ReadValueId, nodeId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "indexRange",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReadValueId, indexRange) - offsetof(UA_ReadValueId, attributeId) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_QUALIFIEDNAME,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "dataEncoding",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReadValueId, dataEncoding) - offsetof(UA_ReadValueId, indexRange) - sizeof(UA_String),
    .isArray = false
  },};

/* AnonymousIdentityToken */
static UA_DataTypeMember AnonymousIdentityToken_members[1] = {
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "policyId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* DataTypeAttributes */
static UA_DataTypeMember DataTypeAttributes_members[6] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "specifiedAttributes",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "displayName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DataTypeAttributes, displayName) - offsetof(UA_DataTypeAttributes, specifiedAttributes) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "description",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DataTypeAttributes, description) - offsetof(UA_DataTypeAttributes, displayName) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "writeMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DataTypeAttributes, writeMask) - offsetof(UA_DataTypeAttributes, description) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userWriteMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DataTypeAttributes, userWriteMask) - offsetof(UA_DataTypeAttributes, writeMask) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "isAbstract",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DataTypeAttributes, isAbstract) - offsetof(UA_DataTypeAttributes, userWriteMask) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* ResponseHeader */
static UA_DataTypeMember ResponseHeader_members[6] = {
  { .memberTypeIndex = UA_TYPES_DATETIME,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "timestamp",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHandle",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ResponseHeader, requestHandle) - offsetof(UA_ResponseHeader, timestamp) - sizeof(UA_DateTime),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "serviceResult",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ResponseHeader, serviceResult) - offsetof(UA_ResponseHeader, requestHandle) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "serviceDiagnostics",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ResponseHeader, serviceDiagnostics) - offsetof(UA_ResponseHeader, serviceResult) - sizeof(UA_StatusCode),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "stringTable",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ResponseHeader, stringTableSize) - offsetof(UA_ResponseHeader, serviceDiagnostics) - sizeof(UA_DiagnosticInfo),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_EXTENSIONOBJECT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "additionalHeader",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ResponseHeader, additionalHeader) - offsetof(UA_ResponseHeader, stringTable) - sizeof(void*),
    .isArray = false
  },};

/* DeleteSubscriptionsRequest */
static UA_DataTypeMember DeleteSubscriptionsRequest_members[2] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "subscriptionIds",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteSubscriptionsRequest, subscriptionIdsSize) - offsetof(UA_DeleteSubscriptionsRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = true
  },};

/* DataChangeNotification */
static UA_DataTypeMember DataChangeNotification_members[2] = {
  { .memberTypeIndex = UA_TYPES_MONITOREDITEMNOTIFICATION,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "monitoredItems",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DataChangeNotification, diagnosticInfosSize) - offsetof(UA_DataChangeNotification, monitoredItems) - sizeof(void*),
    .isArray = true
  },};

/* DeleteMonitoredItemsResponse */
static UA_DataTypeMember DeleteMonitoredItemsResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteMonitoredItemsResponse, resultsSize) - offsetof(UA_DeleteMonitoredItemsResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteMonitoredItemsResponse, diagnosticInfosSize) - offsetof(UA_DeleteMonitoredItemsResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* RelativePath */
static UA_DataTypeMember RelativePath_members[1] = {
  { .memberTypeIndex = UA_TYPES_RELATIVEPATHELEMENT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "elements",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = true
  },};

/* RegisterNodesRequest */
static UA_DataTypeMember RegisterNodesRequest_members[2] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodesToRegister",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_RegisterNodesRequest, nodesToRegisterSize) - offsetof(UA_RegisterNodesRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = true
  },};

/* DeleteNodesRequest */
static UA_DataTypeMember DeleteNodesRequest_members[2] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DELETENODESITEM,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodesToDelete",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteNodesRequest, nodesToDeleteSize) - offsetof(UA_DeleteNodesRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = true
  },};

/* PublishResponse */
static UA_DataTypeMember PublishResponse_members[7] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "subscriptionId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_PublishResponse, subscriptionId) - offsetof(UA_PublishResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "availableSequenceNumbers",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_PublishResponse, availableSequenceNumbersSize) - offsetof(UA_PublishResponse, subscriptionId) - sizeof(UA_UInt32),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "moreNotifications",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_PublishResponse, moreNotifications) - offsetof(UA_PublishResponse, availableSequenceNumbers) - sizeof(void*),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NOTIFICATIONMESSAGE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "notificationMessage",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_PublishResponse, notificationMessage) - offsetof(UA_PublishResponse, moreNotifications) - sizeof(UA_Boolean),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_PublishResponse, resultsSize) - offsetof(UA_PublishResponse, notificationMessage) - sizeof(UA_NotificationMessage),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_PublishResponse, diagnosticInfosSize) - offsetof(UA_PublishResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* MonitoredItemModifyRequest */
static UA_DataTypeMember MonitoredItemModifyRequest_members[2] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "monitoredItemId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_MONITORINGPARAMETERS,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestedParameters",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoredItemModifyRequest, requestedParameters) - offsetof(UA_MonitoredItemModifyRequest, monitoredItemId) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* UserNameIdentityToken */
static UA_DataTypeMember UserNameIdentityToken_members[4] = {
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "policyId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_UserNameIdentityToken, userName) - offsetof(UA_UserNameIdentityToken, policyId) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "password",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_UserNameIdentityToken, password) - offsetof(UA_UserNameIdentityToken, userName) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "encryptionAlgorithm",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_UserNameIdentityToken, encryptionAlgorithm) - offsetof(UA_UserNameIdentityToken, password) - sizeof(UA_ByteString),
    .isArray = false
  },};

/* IdType */
static UA_DataTypeMember IdType_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* UserTokenType */
static UA_DataTypeMember UserTokenType_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* NodeAttributes */
static UA_DataTypeMember NodeAttributes_members[5] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "specifiedAttributes",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "displayName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_NodeAttributes, displayName) - offsetof(UA_NodeAttributes, specifiedAttributes) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "description",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_NodeAttributes, description) - offsetof(UA_NodeAttributes, displayName) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "writeMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_NodeAttributes, writeMask) - offsetof(UA_NodeAttributes, description) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userWriteMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_NodeAttributes, userWriteMask) - offsetof(UA_NodeAttributes, writeMask) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* ActivateSessionRequest */
static UA_DataTypeMember ActivateSessionRequest_members[6] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_SIGNATUREDATA,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "clientSignature",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ActivateSessionRequest, clientSignature) - offsetof(UA_ActivateSessionRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_SIGNEDSOFTWARECERTIFICATE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "clientSoftwareCertificates",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ActivateSessionRequest, clientSoftwareCertificatesSize) - offsetof(UA_ActivateSessionRequest, clientSignature) - sizeof(UA_SignatureData),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "localeIds",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ActivateSessionRequest, localeIdsSize) - offsetof(UA_ActivateSessionRequest, clientSoftwareCertificates) - sizeof(void*),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_EXTENSIONOBJECT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userIdentityToken",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ActivateSessionRequest, userIdentityToken) - offsetof(UA_ActivateSessionRequest, localeIds) - sizeof(void*),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_SIGNATUREDATA,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userTokenSignature",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ActivateSessionRequest, userTokenSignature) - offsetof(UA_ActivateSessionRequest, userIdentityToken) - sizeof(UA_ExtensionObject),
    .isArray = false
  },};

/* OpenSecureChannelResponse */
static UA_DataTypeMember OpenSecureChannelResponse_members[4] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "serverProtocolVersion",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_OpenSecureChannelResponse, serverProtocolVersion) - offsetof(UA_OpenSecureChannelResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_CHANNELSECURITYTOKEN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "securityToken",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_OpenSecureChannelResponse, securityToken) - offsetof(UA_OpenSecureChannelResponse, serverProtocolVersion) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "serverNonce",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_OpenSecureChannelResponse, serverNonce) - offsetof(UA_OpenSecureChannelResponse, securityToken) - sizeof(UA_ChannelSecurityToken),
    .isArray = false
  },};

/* ApplicationType */
static UA_DataTypeMember ApplicationType_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* QueryNextResponse */
static UA_DataTypeMember QueryNextResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_QUERYDATASET,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "queryDataSets",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryNextResponse, queryDataSetsSize) - offsetof(UA_QueryNextResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "revisedContinuationPoint",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryNextResponse, revisedContinuationPoint) - offsetof(UA_QueryNextResponse, queryDataSets) - sizeof(void*),
    .isArray = false
  },};

/* ActivateSessionResponse */
static UA_DataTypeMember ActivateSessionResponse_members[4] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "serverNonce",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ActivateSessionResponse, serverNonce) - offsetof(UA_ActivateSessionResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ActivateSessionResponse, resultsSize) - offsetof(UA_ActivateSessionResponse, serverNonce) - sizeof(UA_ByteString),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ActivateSessionResponse, diagnosticInfosSize) - offsetof(UA_ActivateSessionResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* FilterOperator */
static UA_DataTypeMember FilterOperator_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* QueryNextRequest */
static UA_DataTypeMember QueryNextRequest_members[3] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "releaseContinuationPoint",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryNextRequest, releaseContinuationPoint) - offsetof(UA_QueryNextRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "continuationPoint",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryNextRequest, continuationPoint) - offsetof(UA_QueryNextRequest, releaseContinuationPoint) - sizeof(UA_Boolean),
    .isArray = false
  },};

/* BrowseNextRequest */
static UA_DataTypeMember BrowseNextRequest_members[3] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "releaseContinuationPoints",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseNextRequest, releaseContinuationPoints) - offsetof(UA_BrowseNextRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "continuationPoints",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseNextRequest, continuationPointsSize) - offsetof(UA_BrowseNextRequest, releaseContinuationPoints) - sizeof(UA_Boolean),
    .isArray = true
  },};

/* CreateSubscriptionRequest */
static UA_DataTypeMember CreateSubscriptionRequest_members[7] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DOUBLE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestedPublishingInterval",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSubscriptionRequest, requestedPublishingInterval) - offsetof(UA_CreateSubscriptionRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestedLifetimeCount",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSubscriptionRequest, requestedLifetimeCount) - offsetof(UA_CreateSubscriptionRequest, requestedPublishingInterval) - sizeof(UA_Double),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestedMaxKeepAliveCount",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSubscriptionRequest, requestedMaxKeepAliveCount) - offsetof(UA_CreateSubscriptionRequest, requestedLifetimeCount) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "maxNotificationsPerPublish",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSubscriptionRequest, maxNotificationsPerPublish) - offsetof(UA_CreateSubscriptionRequest, requestedMaxKeepAliveCount) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "publishingEnabled",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSubscriptionRequest, publishingEnabled) - offsetof(UA_CreateSubscriptionRequest, maxNotificationsPerPublish) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "priority",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSubscriptionRequest, priority) - offsetof(UA_CreateSubscriptionRequest, publishingEnabled) - sizeof(UA_Boolean),
    .isArray = false
  },};

/* VariableTypeAttributes */
static UA_DataTypeMember VariableTypeAttributes_members[10] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "specifiedAttributes",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "displayName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableTypeAttributes, displayName) - offsetof(UA_VariableTypeAttributes, specifiedAttributes) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "description",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableTypeAttributes, description) - offsetof(UA_VariableTypeAttributes, displayName) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "writeMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableTypeAttributes, writeMask) - offsetof(UA_VariableTypeAttributes, description) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userWriteMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableTypeAttributes, userWriteMask) - offsetof(UA_VariableTypeAttributes, writeMask) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_VARIANT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "value",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableTypeAttributes, value) - offsetof(UA_VariableTypeAttributes, userWriteMask) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "dataType",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableTypeAttributes, dataType) - offsetof(UA_VariableTypeAttributes, value) - sizeof(UA_Variant),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "valueRank",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableTypeAttributes, valueRank) - offsetof(UA_VariableTypeAttributes, dataType) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "arrayDimensions",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableTypeAttributes, arrayDimensionsSize) - offsetof(UA_VariableTypeAttributes, valueRank) - sizeof(UA_Int32),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "isAbstract",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_VariableTypeAttributes, isAbstract) - offsetof(UA_VariableTypeAttributes, arrayDimensions) - sizeof(void*),
    .isArray = false
  },};

/* BrowsePathResult */
static UA_DataTypeMember BrowsePathResult_members[2] = {
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "statusCode",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BROWSEPATHTARGET,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "targets",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowsePathResult, targetsSize) - offsetof(UA_BrowsePathResult, statusCode) - sizeof(UA_StatusCode),
    .isArray = true
  },};

/* ModifySubscriptionResponse */
static UA_DataTypeMember ModifySubscriptionResponse_members[4] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DOUBLE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "revisedPublishingInterval",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ModifySubscriptionResponse, revisedPublishingInterval) - offsetof(UA_ModifySubscriptionResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "revisedLifetimeCount",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ModifySubscriptionResponse, revisedLifetimeCount) - offsetof(UA_ModifySubscriptionResponse, revisedPublishingInterval) - sizeof(UA_Double),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "revisedMaxKeepAliveCount",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ModifySubscriptionResponse, revisedMaxKeepAliveCount) - offsetof(UA_ModifySubscriptionResponse, revisedLifetimeCount) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* RegisterNodesResponse */
static UA_DataTypeMember RegisterNodesResponse_members[2] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "registeredNodeIds",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_RegisterNodesResponse, registeredNodeIdsSize) - offsetof(UA_RegisterNodesResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },};

/* CloseSessionRequest */
static UA_DataTypeMember CloseSessionRequest_members[2] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "deleteSubscriptions",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CloseSessionRequest, deleteSubscriptions) - offsetof(UA_CloseSessionRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },};

/* ModifySubscriptionRequest */
static UA_DataTypeMember ModifySubscriptionRequest_members[7] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "subscriptionId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ModifySubscriptionRequest, subscriptionId) - offsetof(UA_ModifySubscriptionRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DOUBLE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestedPublishingInterval",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ModifySubscriptionRequest, requestedPublishingInterval) - offsetof(UA_ModifySubscriptionRequest, subscriptionId) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestedLifetimeCount",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ModifySubscriptionRequest, requestedLifetimeCount) - offsetof(UA_ModifySubscriptionRequest, requestedPublishingInterval) - sizeof(UA_Double),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestedMaxKeepAliveCount",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ModifySubscriptionRequest, requestedMaxKeepAliveCount) - offsetof(UA_ModifySubscriptionRequest, requestedLifetimeCount) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "maxNotificationsPerPublish",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ModifySubscriptionRequest, maxNotificationsPerPublish) - offsetof(UA_ModifySubscriptionRequest, requestedMaxKeepAliveCount) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "priority",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ModifySubscriptionRequest, priority) - offsetof(UA_ModifySubscriptionRequest, maxNotificationsPerPublish) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* UserTokenPolicy */
static UA_DataTypeMember UserTokenPolicy_members[5] = {
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "policyId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_USERTOKENTYPE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "tokenType",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_UserTokenPolicy, tokenType) - offsetof(UA_UserTokenPolicy, policyId) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "issuedTokenType",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_UserTokenPolicy, issuedTokenType) - offsetof(UA_UserTokenPolicy, tokenType) - sizeof(UA_UserTokenType),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "issuerEndpointUrl",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_UserTokenPolicy, issuerEndpointUrl) - offsetof(UA_UserTokenPolicy, issuedTokenType) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "securityPolicyUri",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_UserTokenPolicy, securityPolicyUri) - offsetof(UA_UserTokenPolicy, issuerEndpointUrl) - sizeof(UA_String),
    .isArray = false
  },};

/* DeleteMonitoredItemsRequest */
static UA_DataTypeMember DeleteMonitoredItemsRequest_members[3] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "subscriptionId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteMonitoredItemsRequest, subscriptionId) - offsetof(UA_DeleteMonitoredItemsRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "monitoredItemIds",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteMonitoredItemsRequest, monitoredItemIdsSize) - offsetof(UA_DeleteMonitoredItemsRequest, subscriptionId) - sizeof(UA_UInt32),
    .isArray = true
  },};

/* ReferenceTypeAttributes */
static UA_DataTypeMember ReferenceTypeAttributes_members[8] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "specifiedAttributes",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "displayName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceTypeAttributes, displayName) - offsetof(UA_ReferenceTypeAttributes, specifiedAttributes) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "description",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceTypeAttributes, description) - offsetof(UA_ReferenceTypeAttributes, displayName) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "writeMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceTypeAttributes, writeMask) - offsetof(UA_ReferenceTypeAttributes, description) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userWriteMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceTypeAttributes, userWriteMask) - offsetof(UA_ReferenceTypeAttributes, writeMask) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "isAbstract",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceTypeAttributes, isAbstract) - offsetof(UA_ReferenceTypeAttributes, userWriteMask) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "symmetric",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceTypeAttributes, symmetric) - offsetof(UA_ReferenceTypeAttributes, isAbstract) - sizeof(UA_Boolean),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "inverseName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceTypeAttributes, inverseName) - offsetof(UA_ReferenceTypeAttributes, symmetric) - sizeof(UA_Boolean),
    .isArray = false
  },};

/* BrowsePath */
static UA_DataTypeMember BrowsePath_members[2] = {
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "startingNode",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_RELATIVEPATH,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "relativePath",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowsePath, relativePath) - offsetof(UA_BrowsePath, startingNode) - sizeof(UA_NodeId),
    .isArray = false
  },};

/* UnregisterNodesResponse */
static UA_DataTypeMember UnregisterNodesResponse_members[1] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* WriteRequest */
static UA_DataTypeMember WriteRequest_members[2] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_WRITEVALUE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodesToWrite",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_WriteRequest, nodesToWriteSize) - offsetof(UA_WriteRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = true
  },};

/* ObjectAttributes */
static UA_DataTypeMember ObjectAttributes_members[6] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "specifiedAttributes",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "displayName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ObjectAttributes, displayName) - offsetof(UA_ObjectAttributes, specifiedAttributes) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "description",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ObjectAttributes, description) - offsetof(UA_ObjectAttributes, displayName) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "writeMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ObjectAttributes, writeMask) - offsetof(UA_ObjectAttributes, description) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userWriteMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ObjectAttributes, userWriteMask) - offsetof(UA_ObjectAttributes, writeMask) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "eventNotifier",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ObjectAttributes, eventNotifier) - offsetof(UA_ObjectAttributes, userWriteMask) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* BrowseDescription */
static UA_DataTypeMember BrowseDescription_members[6] = {
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodeId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BROWSEDIRECTION,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "browseDirection",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseDescription, browseDirection) - offsetof(UA_BrowseDescription, nodeId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "referenceTypeId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseDescription, referenceTypeId) - offsetof(UA_BrowseDescription, browseDirection) - sizeof(UA_BrowseDirection),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "includeSubtypes",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseDescription, includeSubtypes) - offsetof(UA_BrowseDescription, referenceTypeId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodeClassMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseDescription, nodeClassMask) - offsetof(UA_BrowseDescription, includeSubtypes) - sizeof(UA_Boolean),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "resultMask",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseDescription, resultMask) - offsetof(UA_BrowseDescription, nodeClassMask) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* RepublishRequest */
static UA_DataTypeMember RepublishRequest_members[3] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "subscriptionId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_RepublishRequest, subscriptionId) - offsetof(UA_RepublishRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "retransmitSequenceNumber",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_RepublishRequest, retransmitSequenceNumber) - offsetof(UA_RepublishRequest, subscriptionId) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* GetEndpointsRequest */
static UA_DataTypeMember GetEndpointsRequest_members[4] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "endpointUrl",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_GetEndpointsRequest, endpointUrl) - offsetof(UA_GetEndpointsRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "localeIds",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_GetEndpointsRequest, localeIdsSize) - offsetof(UA_GetEndpointsRequest, endpointUrl) - sizeof(UA_String),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "profileUris",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_GetEndpointsRequest, profileUrisSize) - offsetof(UA_GetEndpointsRequest, localeIds) - sizeof(void*),
    .isArray = true
  },};

/* PublishRequest */
static UA_DataTypeMember PublishRequest_members[2] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_SUBSCRIPTIONACKNOWLEDGEMENT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "subscriptionAcknowledgements",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_PublishRequest, subscriptionAcknowledgementsSize) - offsetof(UA_PublishRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = true
  },};

/* AddNodesResponse */
static UA_DataTypeMember AddNodesResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_ADDNODESRESULT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddNodesResponse, resultsSize) - offsetof(UA_AddNodesResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddNodesResponse, diagnosticInfosSize) - offsetof(UA_AddNodesResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* CloseSecureChannelResponse */
static UA_DataTypeMember CloseSecureChannelResponse_members[1] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* ModifyMonitoredItemsRequest */
static UA_DataTypeMember ModifyMonitoredItemsRequest_members[4] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "subscriptionId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ModifyMonitoredItemsRequest, subscriptionId) - offsetof(UA_ModifyMonitoredItemsRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_TIMESTAMPSTORETURN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "timestampsToReturn",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ModifyMonitoredItemsRequest, timestampsToReturn) - offsetof(UA_ModifyMonitoredItemsRequest, subscriptionId) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_MONITOREDITEMMODIFYREQUEST,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "itemsToModify",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ModifyMonitoredItemsRequest, itemsToModifySize) - offsetof(UA_ModifyMonitoredItemsRequest, timestampsToReturn) - sizeof(UA_TimestampsToReturn),
    .isArray = true
  },};

/* FindServersRequest */
static UA_DataTypeMember FindServersRequest_members[4] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "endpointUrl",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_FindServersRequest, endpointUrl) - offsetof(UA_FindServersRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "localeIds",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_FindServersRequest, localeIdsSize) - offsetof(UA_FindServersRequest, endpointUrl) - sizeof(UA_String),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "serverUris",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_FindServersRequest, serverUrisSize) - offsetof(UA_FindServersRequest, localeIds) - sizeof(void*),
    .isArray = true
  },};

/* ReferenceDescription */
static UA_DataTypeMember ReferenceDescription_members[7] = {
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "referenceTypeId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "isForward",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceDescription, isForward) - offsetof(UA_ReferenceDescription, referenceTypeId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXPANDEDNODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodeId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceDescription, nodeId) - offsetof(UA_ReferenceDescription, isForward) - sizeof(UA_Boolean),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_QUALIFIEDNAME,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "browseName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceDescription, browseName) - offsetof(UA_ReferenceDescription, nodeId) - sizeof(UA_ExpandedNodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "displayName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceDescription, displayName) - offsetof(UA_ReferenceDescription, browseName) - sizeof(UA_QualifiedName),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODECLASS,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodeClass",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceDescription, nodeClass) - offsetof(UA_ReferenceDescription, displayName) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXPANDEDNODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "typeDefinition",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReferenceDescription, typeDefinition) - offsetof(UA_ReferenceDescription, nodeClass) - sizeof(UA_NodeClass),
    .isArray = false
  },};

/* SetPublishingModeResponse */
static UA_DataTypeMember SetPublishingModeResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_SetPublishingModeResponse, resultsSize) - offsetof(UA_SetPublishingModeResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_SetPublishingModeResponse, diagnosticInfosSize) - offsetof(UA_SetPublishingModeResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* ContentFilterResult */
static UA_DataTypeMember ContentFilterResult_members[2] = {
  { .memberTypeIndex = UA_TYPES_CONTENTFILTERELEMENTRESULT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "elementResults",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "elementDiagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ContentFilterResult, elementDiagnosticInfosSize) - offsetof(UA_ContentFilterResult, elementResults) - sizeof(void*),
    .isArray = true
  },};

/* AddReferencesItem */
static UA_DataTypeMember AddReferencesItem_members[6] = {
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "sourceNodeId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "referenceTypeId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddReferencesItem, referenceTypeId) - offsetof(UA_AddReferencesItem, sourceNodeId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "isForward",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddReferencesItem, isForward) - offsetof(UA_AddReferencesItem, referenceTypeId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "targetServerUri",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddReferencesItem, targetServerUri) - offsetof(UA_AddReferencesItem, isForward) - sizeof(UA_Boolean),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXPANDEDNODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "targetNodeId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddReferencesItem, targetNodeId) - offsetof(UA_AddReferencesItem, targetServerUri) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODECLASS,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "targetNodeClass",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddReferencesItem, targetNodeClass) - offsetof(UA_AddReferencesItem, targetNodeId) - sizeof(UA_ExpandedNodeId),
    .isArray = false
  },};

/* QueryDataDescription */
static UA_DataTypeMember QueryDataDescription_members[3] = {
  { .memberTypeIndex = UA_TYPES_RELATIVEPATH,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "relativePath",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "attributeId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryDataDescription, attributeId) - offsetof(UA_QueryDataDescription, relativePath) - sizeof(UA_RelativePath),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "indexRange",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryDataDescription, indexRange) - offsetof(UA_QueryDataDescription, attributeId) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* CreateSubscriptionResponse */
static UA_DataTypeMember CreateSubscriptionResponse_members[5] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "subscriptionId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSubscriptionResponse, subscriptionId) - offsetof(UA_CreateSubscriptionResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DOUBLE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "revisedPublishingInterval",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSubscriptionResponse, revisedPublishingInterval) - offsetof(UA_CreateSubscriptionResponse, subscriptionId) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "revisedLifetimeCount",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSubscriptionResponse, revisedLifetimeCount) - offsetof(UA_CreateSubscriptionResponse, revisedPublishingInterval) - sizeof(UA_Double),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "revisedMaxKeepAliveCount",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSubscriptionResponse, revisedMaxKeepAliveCount) - offsetof(UA_CreateSubscriptionResponse, revisedLifetimeCount) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* DeleteSubscriptionsResponse */
static UA_DataTypeMember DeleteSubscriptionsResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteSubscriptionsResponse, resultsSize) - offsetof(UA_DeleteSubscriptionsResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteSubscriptionsResponse, diagnosticInfosSize) - offsetof(UA_DeleteSubscriptionsResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* WriteResponse */
static UA_DataTypeMember WriteResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_WriteResponse, resultsSize) - offsetof(UA_WriteResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_WriteResponse, diagnosticInfosSize) - offsetof(UA_WriteResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* DeleteReferencesResponse */
static UA_DataTypeMember DeleteReferencesResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteReferencesResponse, resultsSize) - offsetof(UA_DeleteReferencesResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteReferencesResponse, diagnosticInfosSize) - offsetof(UA_DeleteReferencesResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* CreateMonitoredItemsResponse */
static UA_DataTypeMember CreateMonitoredItemsResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_MONITOREDITEMCREATERESULT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateMonitoredItemsResponse, resultsSize) - offsetof(UA_CreateMonitoredItemsResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateMonitoredItemsResponse, diagnosticInfosSize) - offsetof(UA_CreateMonitoredItemsResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* CallResponse */
static UA_DataTypeMember CallResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_CALLMETHODRESULT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CallResponse, resultsSize) - offsetof(UA_CallResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CallResponse, diagnosticInfosSize) - offsetof(UA_CallResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* DeleteNodesResponse */
static UA_DataTypeMember DeleteNodesResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteNodesResponse, resultsSize) - offsetof(UA_DeleteNodesResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteNodesResponse, diagnosticInfosSize) - offsetof(UA_DeleteNodesResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* RepublishResponse */
static UA_DataTypeMember RepublishResponse_members[2] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NOTIFICATIONMESSAGE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "notificationMessage",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_RepublishResponse, notificationMessage) - offsetof(UA_RepublishResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = false
  },};

/* MonitoredItemCreateRequest */
static UA_DataTypeMember MonitoredItemCreateRequest_members[3] = {
  { .memberTypeIndex = UA_TYPES_READVALUEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "itemToMonitor",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_MONITORINGMODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "monitoringMode",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoredItemCreateRequest, monitoringMode) - offsetof(UA_MonitoredItemCreateRequest, itemToMonitor) - sizeof(UA_ReadValueId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_MONITORINGPARAMETERS,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestedParameters",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_MonitoredItemCreateRequest, requestedParameters) - offsetof(UA_MonitoredItemCreateRequest, monitoringMode) - sizeof(UA_MonitoringMode),
    .isArray = false
  },};

/* DeleteReferencesRequest */
static UA_DataTypeMember DeleteReferencesRequest_members[2] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DELETEREFERENCESITEM,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "referencesToDelete",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_DeleteReferencesRequest, referencesToDeleteSize) - offsetof(UA_DeleteReferencesRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = true
  },};

/* ModifyMonitoredItemsResponse */
static UA_DataTypeMember ModifyMonitoredItemsResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_MONITOREDITEMMODIFYRESULT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ModifyMonitoredItemsResponse, resultsSize) - offsetof(UA_ModifyMonitoredItemsResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ModifyMonitoredItemsResponse, diagnosticInfosSize) - offsetof(UA_ModifyMonitoredItemsResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* ReadResponse */
static UA_DataTypeMember ReadResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DATAVALUE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReadResponse, resultsSize) - offsetof(UA_ReadResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReadResponse, diagnosticInfosSize) - offsetof(UA_ReadResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* AddReferencesRequest */
static UA_DataTypeMember AddReferencesRequest_members[2] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_ADDREFERENCESITEM,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "referencesToAdd",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddReferencesRequest, referencesToAddSize) - offsetof(UA_AddReferencesRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = true
  },};

/* ReadRequest */
static UA_DataTypeMember ReadRequest_members[4] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DOUBLE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "maxAge",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReadRequest, maxAge) - offsetof(UA_ReadRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_TIMESTAMPSTORETURN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "timestampsToReturn",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReadRequest, timestampsToReturn) - offsetof(UA_ReadRequest, maxAge) - sizeof(UA_Double),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_READVALUEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodesToRead",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ReadRequest, nodesToReadSize) - offsetof(UA_ReadRequest, timestampsToReturn) - sizeof(UA_TimestampsToReturn),
    .isArray = true
  },};

/* OpenSecureChannelRequest */
static UA_DataTypeMember OpenSecureChannelRequest_members[6] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "clientProtocolVersion",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_OpenSecureChannelRequest, clientProtocolVersion) - offsetof(UA_OpenSecureChannelRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_SECURITYTOKENREQUESTTYPE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestType",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_OpenSecureChannelRequest, requestType) - offsetof(UA_OpenSecureChannelRequest, clientProtocolVersion) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_MESSAGESECURITYMODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "securityMode",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_OpenSecureChannelRequest, securityMode) - offsetof(UA_OpenSecureChannelRequest, requestType) - sizeof(UA_SecurityTokenRequestType),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "clientNonce",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_OpenSecureChannelRequest, clientNonce) - offsetof(UA_OpenSecureChannelRequest, securityMode) - sizeof(UA_MessageSecurityMode),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestedLifetime",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_OpenSecureChannelRequest, requestedLifetime) - offsetof(UA_OpenSecureChannelRequest, clientNonce) - sizeof(UA_ByteString),
    .isArray = false
  },};

/* AddNodesItem */
static UA_DataTypeMember AddNodesItem_members[7] = {
  { .memberTypeIndex = UA_TYPES_EXPANDEDNODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "parentNodeId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "referenceTypeId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddNodesItem, referenceTypeId) - offsetof(UA_AddNodesItem, parentNodeId) - sizeof(UA_ExpandedNodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXPANDEDNODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestedNewNodeId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddNodesItem, requestedNewNodeId) - offsetof(UA_AddNodesItem, referenceTypeId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_QUALIFIEDNAME,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "browseName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddNodesItem, browseName) - offsetof(UA_AddNodesItem, requestedNewNodeId) - sizeof(UA_ExpandedNodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODECLASS,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodeClass",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddNodesItem, nodeClass) - offsetof(UA_AddNodesItem, browseName) - sizeof(UA_QualifiedName),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXTENSIONOBJECT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodeAttributes",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddNodesItem, nodeAttributes) - offsetof(UA_AddNodesItem, nodeClass) - sizeof(UA_NodeClass),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXPANDEDNODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "typeDefinition",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddNodesItem, typeDefinition) - offsetof(UA_AddNodesItem, nodeAttributes) - sizeof(UA_ExtensionObject),
    .isArray = false
  },};

/* ApplicationDescription */
static UA_DataTypeMember ApplicationDescription_members[7] = {
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "applicationUri",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "productUri",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ApplicationDescription, productUri) - offsetof(UA_ApplicationDescription, applicationUri) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "applicationName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ApplicationDescription, applicationName) - offsetof(UA_ApplicationDescription, productUri) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_APPLICATIONTYPE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "applicationType",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ApplicationDescription, applicationType) - offsetof(UA_ApplicationDescription, applicationName) - sizeof(UA_LocalizedText),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "gatewayServerUri",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ApplicationDescription, gatewayServerUri) - offsetof(UA_ApplicationDescription, applicationType) - sizeof(UA_ApplicationType),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "discoveryProfileUri",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ApplicationDescription, discoveryProfileUri) - offsetof(UA_ApplicationDescription, gatewayServerUri) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "discoveryUrls",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ApplicationDescription, discoveryUrlsSize) - offsetof(UA_ApplicationDescription, discoveryProfileUri) - sizeof(UA_String),
    .isArray = true
  },};

/* NodeTypeDescription */
static UA_DataTypeMember NodeTypeDescription_members[3] = {
  { .memberTypeIndex = UA_TYPES_EXPANDEDNODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "typeDefinitionNode",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "includeSubTypes",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_NodeTypeDescription, includeSubTypes) - offsetof(UA_NodeTypeDescription, typeDefinitionNode) - sizeof(UA_ExpandedNodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_QUERYDATADESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "dataToReturn",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_NodeTypeDescription, dataToReturnSize) - offsetof(UA_NodeTypeDescription, includeSubTypes) - sizeof(UA_Boolean),
    .isArray = true
  },};

/* FindServersResponse */
static UA_DataTypeMember FindServersResponse_members[2] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_APPLICATIONDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "servers",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_FindServersResponse, serversSize) - offsetof(UA_FindServersResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },};

/* ServerStatusDataType */
static UA_DataTypeMember ServerStatusDataType_members[6] = {
  { .memberTypeIndex = UA_TYPES_DATETIME,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "startTime",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DATETIME,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "currentTime",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ServerStatusDataType, currentTime) - offsetof(UA_ServerStatusDataType, startTime) - sizeof(UA_DateTime),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_SERVERSTATE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "state",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ServerStatusDataType, state) - offsetof(UA_ServerStatusDataType, currentTime) - sizeof(UA_DateTime),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BUILDINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "buildInfo",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ServerStatusDataType, buildInfo) - offsetof(UA_ServerStatusDataType, state) - sizeof(UA_ServerState),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "secondsTillShutdown",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ServerStatusDataType, secondsTillShutdown) - offsetof(UA_ServerStatusDataType, buildInfo) - sizeof(UA_BuildInfo),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "shutdownReason",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ServerStatusDataType, shutdownReason) - offsetof(UA_ServerStatusDataType, secondsTillShutdown) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* AddReferencesResponse */
static UA_DataTypeMember AddReferencesResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddReferencesResponse, resultsSize) - offsetof(UA_AddReferencesResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddReferencesResponse, diagnosticInfosSize) - offsetof(UA_AddReferencesResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* TranslateBrowsePathsToNodeIdsResponse */
static UA_DataTypeMember TranslateBrowsePathsToNodeIdsResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BROWSEPATHRESULT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_TranslateBrowsePathsToNodeIdsResponse, resultsSize) - offsetof(UA_TranslateBrowsePathsToNodeIdsResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_TranslateBrowsePathsToNodeIdsResponse, diagnosticInfosSize) - offsetof(UA_TranslateBrowsePathsToNodeIdsResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* ContentFilterElement */
static UA_DataTypeMember ContentFilterElement_members[2] = {
  { .memberTypeIndex = UA_TYPES_FILTEROPERATOR,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "filterOperator",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_EXTENSIONOBJECT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "filterOperands",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_ContentFilterElement, filterOperandsSize) - offsetof(UA_ContentFilterElement, filterOperator) - sizeof(UA_FilterOperator),
    .isArray = true
  },};

/* TranslateBrowsePathsToNodeIdsRequest */
static UA_DataTypeMember TranslateBrowsePathsToNodeIdsRequest_members[2] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BROWSEPATH,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "browsePaths",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_TranslateBrowsePathsToNodeIdsRequest, browsePathsSize) - offsetof(UA_TranslateBrowsePathsToNodeIdsRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = true
  },};

/* CloseSessionResponse */
static UA_DataTypeMember CloseSessionResponse_members[1] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* ServiceFault */
static UA_DataTypeMember ServiceFault_members[1] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* CreateMonitoredItemsRequest */
static UA_DataTypeMember CreateMonitoredItemsRequest_members[4] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "subscriptionId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateMonitoredItemsRequest, subscriptionId) - offsetof(UA_CreateMonitoredItemsRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_TIMESTAMPSTORETURN,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "timestampsToReturn",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateMonitoredItemsRequest, timestampsToReturn) - offsetof(UA_CreateMonitoredItemsRequest, subscriptionId) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_MONITOREDITEMCREATEREQUEST,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "itemsToCreate",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateMonitoredItemsRequest, itemsToCreateSize) - offsetof(UA_CreateMonitoredItemsRequest, timestampsToReturn) - sizeof(UA_TimestampsToReturn),
    .isArray = true
  },};

/* ContentFilter */
static UA_DataTypeMember ContentFilter_members[1] = {
  { .memberTypeIndex = UA_TYPES_CONTENTFILTERELEMENT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "elements",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = true
  },};

/* QueryFirstResponse */
static UA_DataTypeMember QueryFirstResponse_members[6] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_QUERYDATASET,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "queryDataSets",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryFirstResponse, queryDataSetsSize) - offsetof(UA_QueryFirstResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "continuationPoint",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryFirstResponse, continuationPoint) - offsetof(UA_QueryFirstResponse, queryDataSets) - sizeof(void*),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_PARSINGRESULT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "parsingResults",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryFirstResponse, parsingResultsSize) - offsetof(UA_QueryFirstResponse, continuationPoint) - sizeof(UA_ByteString),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryFirstResponse, diagnosticInfosSize) - offsetof(UA_QueryFirstResponse, parsingResults) - sizeof(void*),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_CONTENTFILTERRESULT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "filterResult",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryFirstResponse, filterResult) - offsetof(UA_QueryFirstResponse, diagnosticInfos) - sizeof(void*),
    .isArray = false
  },};

/* AddNodesRequest */
static UA_DataTypeMember AddNodesRequest_members[2] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_ADDNODESITEM,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodesToAdd",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AddNodesRequest, nodesToAddSize) - offsetof(UA_AddNodesRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = true
  },};

/* BrowseRequest */
static UA_DataTypeMember BrowseRequest_members[4] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_VIEWDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "view",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseRequest, view) - offsetof(UA_BrowseRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestedMaxReferencesPerNode",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseRequest, requestedMaxReferencesPerNode) - offsetof(UA_BrowseRequest, view) - sizeof(UA_ViewDescription),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BROWSEDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodesToBrowse",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseRequest, nodesToBrowseSize) - offsetof(UA_BrowseRequest, requestedMaxReferencesPerNode) - sizeof(UA_UInt32),
    .isArray = true
  },};

/* BrowseResult */
static UA_DataTypeMember BrowseResult_members[3] = {
  { .memberTypeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "statusCode",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "continuationPoint",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseResult, continuationPoint) - offsetof(UA_BrowseResult, statusCode) - sizeof(UA_StatusCode),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_REFERENCEDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "references",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseResult, referencesSize) - offsetof(UA_BrowseResult, continuationPoint) - sizeof(UA_ByteString),
    .isArray = true
  },};

/* CreateSessionRequest */
static UA_DataTypeMember CreateSessionRequest_members[9] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_APPLICATIONDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "clientDescription",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionRequest, clientDescription) - offsetof(UA_CreateSessionRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "serverUri",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionRequest, serverUri) - offsetof(UA_CreateSessionRequest, clientDescription) - sizeof(UA_ApplicationDescription),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "endpointUrl",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionRequest, endpointUrl) - offsetof(UA_CreateSessionRequest, serverUri) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "sessionName",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionRequest, sessionName) - offsetof(UA_CreateSessionRequest, endpointUrl) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "clientNonce",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionRequest, clientNonce) - offsetof(UA_CreateSessionRequest, sessionName) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "clientCertificate",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionRequest, clientCertificate) - offsetof(UA_CreateSessionRequest, clientNonce) - sizeof(UA_ByteString),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DOUBLE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestedSessionTimeout",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionRequest, requestedSessionTimeout) - offsetof(UA_CreateSessionRequest, clientCertificate) - sizeof(UA_ByteString),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "maxResponseMessageSize",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionRequest, maxResponseMessageSize) - offsetof(UA_CreateSessionRequest, requestedSessionTimeout) - sizeof(UA_Double),
    .isArray = false
  },};

/* EndpointDescription */
static UA_DataTypeMember EndpointDescription_members[8] = {
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "endpointUrl",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_APPLICATIONDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "server",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_EndpointDescription, server) - offsetof(UA_EndpointDescription, endpointUrl) - sizeof(UA_String),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "serverCertificate",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_EndpointDescription, serverCertificate) - offsetof(UA_EndpointDescription, server) - sizeof(UA_ApplicationDescription),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_MESSAGESECURITYMODE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "securityMode",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_EndpointDescription, securityMode) - offsetof(UA_EndpointDescription, serverCertificate) - sizeof(UA_ByteString),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "securityPolicyUri",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_EndpointDescription, securityPolicyUri) - offsetof(UA_EndpointDescription, securityMode) - sizeof(UA_MessageSecurityMode),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_USERTOKENPOLICY,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "userIdentityTokens",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_EndpointDescription, userIdentityTokensSize) - offsetof(UA_EndpointDescription, securityPolicyUri) - sizeof(UA_String),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "transportProfileUri",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_EndpointDescription, transportProfileUri) - offsetof(UA_EndpointDescription, userIdentityTokens) - sizeof(void*),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "securityLevel",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_EndpointDescription, securityLevel) - offsetof(UA_EndpointDescription, transportProfileUri) - sizeof(UA_String),
    .isArray = false
  },};

/* GetEndpointsResponse */
static UA_DataTypeMember GetEndpointsResponse_members[2] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_ENDPOINTDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "endpoints",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_GetEndpointsResponse, endpointsSize) - offsetof(UA_GetEndpointsResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },};

/* BrowseNextResponse */
static UA_DataTypeMember BrowseNextResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BROWSERESULT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseNextResponse, resultsSize) - offsetof(UA_BrowseNextResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseNextResponse, diagnosticInfosSize) - offsetof(UA_BrowseNextResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* BrowseResponse */
static UA_DataTypeMember BrowseResponse_members[3] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BROWSERESULT,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "results",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseResponse, resultsSize) - offsetof(UA_BrowseResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "diagnosticInfos",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_BrowseResponse, diagnosticInfosSize) - offsetof(UA_BrowseResponse, results) - sizeof(void*),
    .isArray = true
  },};

/* CreateSessionResponse */
static UA_DataTypeMember CreateSessionResponse_members[10] = {
  { .memberTypeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "responseHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "sessionId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionResponse, sessionId) - offsetof(UA_CreateSessionResponse, responseHeader) - sizeof(UA_ResponseHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "authenticationToken",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionResponse, authenticationToken) - offsetof(UA_CreateSessionResponse, sessionId) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_DOUBLE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "revisedSessionTimeout",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionResponse, revisedSessionTimeout) - offsetof(UA_CreateSessionResponse, authenticationToken) - sizeof(UA_NodeId),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "serverNonce",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionResponse, serverNonce) - offsetof(UA_CreateSessionResponse, revisedSessionTimeout) - sizeof(UA_Double),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "serverCertificate",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionResponse, serverCertificate) - offsetof(UA_CreateSessionResponse, serverNonce) - sizeof(UA_ByteString),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_ENDPOINTDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "serverEndpoints",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionResponse, serverEndpointsSize) - offsetof(UA_CreateSessionResponse, serverCertificate) - sizeof(UA_ByteString),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_SIGNEDSOFTWARECERTIFICATE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "serverSoftwareCertificates",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionResponse, serverSoftwareCertificatesSize) - offsetof(UA_CreateSessionResponse, serverEndpoints) - sizeof(void*),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_SIGNATUREDATA,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "serverSignature",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionResponse, serverSignature) - offsetof(UA_CreateSessionResponse, serverSoftwareCertificates) - sizeof(void*),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "maxRequestMessageSize",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_CreateSessionResponse, maxRequestMessageSize) - offsetof(UA_CreateSessionResponse, serverSignature) - sizeof(UA_SignatureData),
    .isArray = false
  },};

/* QueryFirstRequest */
static UA_DataTypeMember QueryFirstRequest_members[6] = {
  { .memberTypeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestHeader",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_VIEWDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "view",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryFirstRequest, view) - offsetof(UA_QueryFirstRequest, requestHeader) - sizeof(UA_RequestHeader),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_NODETYPEDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "nodeTypes",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryFirstRequest, nodeTypesSize) - offsetof(UA_QueryFirstRequest, view) - sizeof(UA_ViewDescription),
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_CONTENTFILTER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "filter",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryFirstRequest, filter) - offsetof(UA_QueryFirstRequest, nodeTypes) - sizeof(void*),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "maxDataSetsToReturn",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryFirstRequest, maxDataSetsToReturn) - offsetof(UA_QueryFirstRequest, filter) - sizeof(UA_ContentFilter),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "maxReferencesToReturn",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_QueryFirstRequest, maxReferencesToReturn) - offsetof(UA_QueryFirstRequest, maxDataSetsToReturn) - sizeof(UA_UInt32),
    .isArray = false
  },};
const UA_DataType UA_TYPES[UA_TYPES_COUNT] = {

/* Boolean */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 1},
  .typeIndex = UA_TYPES_BOOLEAN,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "Boolean",
#endif
  .memSize = sizeof(UA_Boolean),
  .builtin = true,
  .fixedSize = true,
  .overlayable = true,
  .membersSize = 1,
  .members = Boolean_members },

/* SByte */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 2},
  .typeIndex = UA_TYPES_SBYTE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "SByte",
#endif
  .memSize = sizeof(UA_SByte),
  .builtin = true,
  .fixedSize = true,
  .overlayable = true,
  .membersSize = 1,
  .members = SByte_members },

/* Byte */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 3},
  .typeIndex = UA_TYPES_BYTE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "Byte",
#endif
  .memSize = sizeof(UA_Byte),
  .builtin = true,
  .fixedSize = true,
  .overlayable = true,
  .membersSize = 1,
  .members = Byte_members },

/* Int16 */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 4},
  .typeIndex = UA_TYPES_INT16,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "Int16",
#endif
  .memSize = sizeof(UA_Int16),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = Int16_members },

/* UInt16 */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 5},
  .typeIndex = UA_TYPES_UINT16,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "UInt16",
#endif
  .memSize = sizeof(UA_UInt16),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = UInt16_members },

/* Int32 */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 6},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "Int32",
#endif
  .memSize = sizeof(UA_Int32),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = Int32_members },

/* UInt32 */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 7},
  .typeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "UInt32",
#endif
  .memSize = sizeof(UA_UInt32),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = UInt32_members },

/* Int64 */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 8},
  .typeIndex = UA_TYPES_INT64,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "Int64",
#endif
  .memSize = sizeof(UA_Int64),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = Int64_members },

/* UInt64 */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 9},
  .typeIndex = UA_TYPES_UINT64,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "UInt64",
#endif
  .memSize = sizeof(UA_UInt64),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = UInt64_members },

/* Float */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 10},
  .typeIndex = UA_TYPES_FLOAT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "Float",
#endif
  .memSize = sizeof(UA_Float),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_FLOAT,
  .membersSize = 1,
  .members = Float_members },

/* Double */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 11},
  .typeIndex = UA_TYPES_DOUBLE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "Double",
#endif
  .memSize = sizeof(UA_Double),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_FLOAT,
  .membersSize = 1,
  .members = Double_members },

/* String */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 12},
  .typeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "String",
#endif
  .memSize = sizeof(UA_String),
  .builtin = true,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = String_members },

/* DateTime */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 13},
  .typeIndex = UA_TYPES_DATETIME,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DateTime",
#endif
  .memSize = sizeof(UA_DateTime),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = DateTime_members },

/* Guid */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 14},
  .typeIndex = UA_TYPES_GUID,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "Guid",
#endif
  .memSize = sizeof(UA_Guid),
  .builtin = true,
  .fixedSize = true,
  .overlayable = (UA_BINARY_OVERLAYABLE_INTEGER && offsetof(UA_Guid, data2) == sizeof(UA_UInt32) && offsetof(UA_Guid, data3) == (sizeof(UA_UInt16) + sizeof(UA_UInt32)) && offsetof(UA_Guid, data4) == (2*sizeof(UA_UInt32))),
  .membersSize = 1,
  .members = Guid_members },

/* ByteString */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 15},
  .typeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ByteString",
#endif
  .memSize = sizeof(UA_ByteString),
  .builtin = true,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = ByteString_members },

/* XmlElement */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 16},
  .typeIndex = UA_TYPES_XMLELEMENT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "XmlElement",
#endif
  .memSize = sizeof(UA_XmlElement),
  .builtin = true,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = XmlElement_members },

/* NodeId */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 17},
  .typeIndex = UA_TYPES_NODEID,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "NodeId",
#endif
  .memSize = sizeof(UA_NodeId),
  .builtin = true,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = NodeId_members },

/* ExpandedNodeId */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 18},
  .typeIndex = UA_TYPES_EXPANDEDNODEID,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ExpandedNodeId",
#endif
  .memSize = sizeof(UA_ExpandedNodeId),
  .builtin = true,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = ExpandedNodeId_members },

/* StatusCode */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 19},
  .typeIndex = UA_TYPES_STATUSCODE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "StatusCode",
#endif
  .memSize = sizeof(UA_StatusCode),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = StatusCode_members },

/* QualifiedName */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 20},
  .typeIndex = UA_TYPES_QUALIFIEDNAME,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "QualifiedName",
#endif
  .memSize = sizeof(UA_QualifiedName),
  .builtin = true,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = QualifiedName_members },

/* LocalizedText */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 21},
  .typeIndex = UA_TYPES_LOCALIZEDTEXT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "LocalizedText",
#endif
  .memSize = sizeof(UA_LocalizedText),
  .builtin = true,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = LocalizedText_members },

/* ExtensionObject */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 22},
  .typeIndex = UA_TYPES_EXTENSIONOBJECT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ExtensionObject",
#endif
  .memSize = sizeof(UA_ExtensionObject),
  .builtin = true,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = ExtensionObject_members },

/* DataValue */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 23},
  .typeIndex = UA_TYPES_DATAVALUE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DataValue",
#endif
  .memSize = sizeof(UA_DataValue),
  .builtin = true,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = DataValue_members },

/* Variant */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 24},
  .typeIndex = UA_TYPES_VARIANT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "Variant",
#endif
  .memSize = sizeof(UA_Variant),
  .builtin = true,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = Variant_members },

/* DiagnosticInfo */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 25},
  .typeIndex = UA_TYPES_DIAGNOSTICINFO,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DiagnosticInfo",
#endif
  .memSize = sizeof(UA_DiagnosticInfo),
  .builtin = true,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = DiagnosticInfo_members },

/* SignedSoftwareCertificate */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 344},
  .typeIndex = UA_TYPES_SIGNEDSOFTWARECERTIFICATE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "SignedSoftwareCertificate",
#endif
  .memSize = sizeof(UA_SignedSoftwareCertificate),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = SignedSoftwareCertificate_members },

/* BrowsePathTarget */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 546},
  .typeIndex = UA_TYPES_BROWSEPATHTARGET,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "BrowsePathTarget",
#endif
  .memSize = sizeof(UA_BrowsePathTarget),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = BrowsePathTarget_members },

/* ViewAttributes */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 373},
  .typeIndex = UA_TYPES_VIEWATTRIBUTES,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ViewAttributes",
#endif
  .memSize = sizeof(UA_ViewAttributes),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 7,
  .members = ViewAttributes_members },

/* BrowseResultMask */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 6},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "BrowseResultMask",
#endif
  .memSize = sizeof(UA_BrowseResultMask),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = BrowseResultMask_members },

/* RequestHeader */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 389},
  .typeIndex = UA_TYPES_REQUESTHEADER,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "RequestHeader",
#endif
  .memSize = sizeof(UA_RequestHeader),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 7,
  .members = RequestHeader_members },

/* MonitoredItemModifyResult */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 758},
  .typeIndex = UA_TYPES_MONITOREDITEMMODIFYRESULT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "MonitoredItemModifyResult",
#endif
  .memSize = sizeof(UA_MonitoredItemModifyResult),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = MonitoredItemModifyResult_members },

/* ViewDescription */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 511},
  .typeIndex = UA_TYPES_VIEWDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ViewDescription",
#endif
  .memSize = sizeof(UA_ViewDescription),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = ViewDescription_members },

/* CloseSecureChannelRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 450},
  .typeIndex = UA_TYPES_CLOSESECURECHANNELREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "CloseSecureChannelRequest",
#endif
  .memSize = sizeof(UA_CloseSecureChannelRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = CloseSecureChannelRequest_members },

/* AddNodesResult */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 483},
  .typeIndex = UA_TYPES_ADDNODESRESULT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "AddNodesResult",
#endif
  .memSize = sizeof(UA_AddNodesResult),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = AddNodesResult_members },

/* VariableAttributes */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 355},
  .typeIndex = UA_TYPES_VARIABLEATTRIBUTES,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "VariableAttributes",
#endif
  .memSize = sizeof(UA_VariableAttributes),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 13,
  .members = VariableAttributes_members },

/* NotificationMessage */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 803},
  .typeIndex = UA_TYPES_NOTIFICATIONMESSAGE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "NotificationMessage",
#endif
  .memSize = sizeof(UA_NotificationMessage),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = NotificationMessage_members },

/* NodeAttributesMask */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 6},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "NodeAttributesMask",
#endif
  .memSize = sizeof(UA_NodeAttributesMask),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = NodeAttributesMask_members },

/* MonitoringMode */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 6},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "MonitoringMode",
#endif
  .memSize = sizeof(UA_MonitoringMode),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = MonitoringMode_members },

/* CallMethodResult */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 707},
  .typeIndex = UA_TYPES_CALLMETHODRESULT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "CallMethodResult",
#endif
  .memSize = sizeof(UA_CallMethodResult),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = CallMethodResult_members },

/* ParsingResult */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 610},
  .typeIndex = UA_TYPES_PARSINGRESULT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ParsingResult",
#endif
  .memSize = sizeof(UA_ParsingResult),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = ParsingResult_members },

/* RelativePathElement */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 537},
  .typeIndex = UA_TYPES_RELATIVEPATHELEMENT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "RelativePathElement",
#endif
  .memSize = sizeof(UA_RelativePathElement),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = RelativePathElement_members },

/* BrowseDirection */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 6},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "BrowseDirection",
#endif
  .memSize = sizeof(UA_BrowseDirection),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = BrowseDirection_members },

/* CallMethodRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 704},
  .typeIndex = UA_TYPES_CALLMETHODREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "CallMethodRequest",
#endif
  .memSize = sizeof(UA_CallMethodRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = CallMethodRequest_members },

/* ServerState */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 6},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ServerState",
#endif
  .memSize = sizeof(UA_ServerState),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = ServerState_members },

/* UnregisterNodesRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 564},
  .typeIndex = UA_TYPES_UNREGISTERNODESREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "UnregisterNodesRequest",
#endif
  .memSize = sizeof(UA_UnregisterNodesRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = UnregisterNodesRequest_members },

/* ContentFilterElementResult */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 604},
  .typeIndex = UA_TYPES_CONTENTFILTERELEMENTRESULT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ContentFilterElementResult",
#endif
  .memSize = sizeof(UA_ContentFilterElementResult),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = ContentFilterElementResult_members },

/* QueryDataSet */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 577},
  .typeIndex = UA_TYPES_QUERYDATASET,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "QueryDataSet",
#endif
  .memSize = sizeof(UA_QueryDataSet),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = QueryDataSet_members },

/* SetPublishingModeRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 797},
  .typeIndex = UA_TYPES_SETPUBLISHINGMODEREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "SetPublishingModeRequest",
#endif
  .memSize = sizeof(UA_SetPublishingModeRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = SetPublishingModeRequest_members },

/* TimestampsToReturn */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 6},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "TimestampsToReturn",
#endif
  .memSize = sizeof(UA_TimestampsToReturn),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = TimestampsToReturn_members },

/* CallRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 710},
  .typeIndex = UA_TYPES_CALLREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "CallRequest",
#endif
  .memSize = sizeof(UA_CallRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = CallRequest_members },

/* MethodAttributes */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 358},
  .typeIndex = UA_TYPES_METHODATTRIBUTES,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "MethodAttributes",
#endif
  .memSize = sizeof(UA_MethodAttributes),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 7,
  .members = MethodAttributes_members },

/* DeleteReferencesItem */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 385},
  .typeIndex = UA_TYPES_DELETEREFERENCESITEM,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DeleteReferencesItem",
#endif
  .memSize = sizeof(UA_DeleteReferencesItem),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 5,
  .members = DeleteReferencesItem_members },

/* WriteValue */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 668},
  .typeIndex = UA_TYPES_WRITEVALUE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "WriteValue",
#endif
  .memSize = sizeof(UA_WriteValue),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = WriteValue_members },

/* MonitoredItemCreateResult */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 746},
  .typeIndex = UA_TYPES_MONITOREDITEMCREATERESULT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "MonitoredItemCreateResult",
#endif
  .memSize = sizeof(UA_MonitoredItemCreateResult),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 5,
  .members = MonitoredItemCreateResult_members },

/* MessageSecurityMode */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 6},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "MessageSecurityMode",
#endif
  .memSize = sizeof(UA_MessageSecurityMode),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = MessageSecurityMode_members },

/* MonitoringParameters */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 740},
  .typeIndex = UA_TYPES_MONITORINGPARAMETERS,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "MonitoringParameters",
#endif
  .memSize = sizeof(UA_MonitoringParameters),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 5,
  .members = MonitoringParameters_members },

/* SignatureData */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 456},
  .typeIndex = UA_TYPES_SIGNATUREDATA,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "SignatureData",
#endif
  .memSize = sizeof(UA_SignatureData),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = SignatureData_members },

/* ReferenceNode */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 285},
  .typeIndex = UA_TYPES_REFERENCENODE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ReferenceNode",
#endif
  .memSize = sizeof(UA_ReferenceNode),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = ReferenceNode_members },

/* Argument */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 296},
  .typeIndex = UA_TYPES_ARGUMENT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "Argument",
#endif
  .memSize = sizeof(UA_Argument),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 5,
  .members = Argument_members },

/* UserIdentityToken */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 316},
  .typeIndex = UA_TYPES_USERIDENTITYTOKEN,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "UserIdentityToken",
#endif
  .memSize = sizeof(UA_UserIdentityToken),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = UserIdentityToken_members },

/* ObjectTypeAttributes */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 361},
  .typeIndex = UA_TYPES_OBJECTTYPEATTRIBUTES,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ObjectTypeAttributes",
#endif
  .memSize = sizeof(UA_ObjectTypeAttributes),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 6,
  .members = ObjectTypeAttributes_members },

/* SecurityTokenRequestType */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 6},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "SecurityTokenRequestType",
#endif
  .memSize = sizeof(UA_SecurityTokenRequestType),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = SecurityTokenRequestType_members },

/* BuildInfo */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 338},
  .typeIndex = UA_TYPES_BUILDINFO,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "BuildInfo",
#endif
  .memSize = sizeof(UA_BuildInfo),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 6,
  .members = BuildInfo_members },

/* NodeClass */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 6},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "NodeClass",
#endif
  .memSize = sizeof(UA_NodeClass),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = NodeClass_members },

/* ChannelSecurityToken */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 441},
  .typeIndex = UA_TYPES_CHANNELSECURITYTOKEN,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ChannelSecurityToken",
#endif
  .memSize = sizeof(UA_ChannelSecurityToken),
  .builtin = false,
  .fixedSize = true,
  .overlayable = true && UA_BINARY_OVERLAYABLE_INTEGER && UA_BINARY_OVERLAYABLE_INTEGER && offsetof(UA_ChannelSecurityToken, tokenId) == (offsetof(UA_ChannelSecurityToken, channelId) + sizeof(UA_UInt32)) && UA_BINARY_OVERLAYABLE_INTEGER && offsetof(UA_ChannelSecurityToken, createdAt) == (offsetof(UA_ChannelSecurityToken, tokenId) + sizeof(UA_UInt32)) && UA_BINARY_OVERLAYABLE_INTEGER && offsetof(UA_ChannelSecurityToken, revisedLifetime) == (offsetof(UA_ChannelSecurityToken, createdAt) + sizeof(UA_DateTime)),
  .membersSize = 4,
  .members = ChannelSecurityToken_members },

/* MonitoredItemNotification */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 806},
  .typeIndex = UA_TYPES_MONITOREDITEMNOTIFICATION,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "MonitoredItemNotification",
#endif
  .memSize = sizeof(UA_MonitoredItemNotification),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = MonitoredItemNotification_members },

/* DeleteNodesItem */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 382},
  .typeIndex = UA_TYPES_DELETENODESITEM,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DeleteNodesItem",
#endif
  .memSize = sizeof(UA_DeleteNodesItem),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = DeleteNodesItem_members },

/* SubscriptionAcknowledgement */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 821},
  .typeIndex = UA_TYPES_SUBSCRIPTIONACKNOWLEDGEMENT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "SubscriptionAcknowledgement",
#endif
  .memSize = sizeof(UA_SubscriptionAcknowledgement),
  .builtin = false,
  .fixedSize = true,
  .overlayable = true && UA_BINARY_OVERLAYABLE_INTEGER && UA_BINARY_OVERLAYABLE_INTEGER && offsetof(UA_SubscriptionAcknowledgement, sequenceNumber) == (offsetof(UA_SubscriptionAcknowledgement, subscriptionId) + sizeof(UA_UInt32)),
  .membersSize = 2,
  .members = SubscriptionAcknowledgement_members },

/* ReadValueId */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 626},
  .typeIndex = UA_TYPES_READVALUEID,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ReadValueId",
#endif
  .memSize = sizeof(UA_ReadValueId),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = ReadValueId_members },

/* AnonymousIdentityToken */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 319},
  .typeIndex = UA_TYPES_ANONYMOUSIDENTITYTOKEN,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "AnonymousIdentityToken",
#endif
  .memSize = sizeof(UA_AnonymousIdentityToken),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = AnonymousIdentityToken_members },

/* DataTypeAttributes */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 370},
  .typeIndex = UA_TYPES_DATATYPEATTRIBUTES,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DataTypeAttributes",
#endif
  .memSize = sizeof(UA_DataTypeAttributes),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 6,
  .members = DataTypeAttributes_members },

/* ResponseHeader */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 392},
  .typeIndex = UA_TYPES_RESPONSEHEADER,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ResponseHeader",
#endif
  .memSize = sizeof(UA_ResponseHeader),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 6,
  .members = ResponseHeader_members },

/* DeleteSubscriptionsRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 845},
  .typeIndex = UA_TYPES_DELETESUBSCRIPTIONSREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DeleteSubscriptionsRequest",
#endif
  .memSize = sizeof(UA_DeleteSubscriptionsRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = DeleteSubscriptionsRequest_members },

/* DataChangeNotification */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 809},
  .typeIndex = UA_TYPES_DATACHANGENOTIFICATION,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DataChangeNotification",
#endif
  .memSize = sizeof(UA_DataChangeNotification),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = DataChangeNotification_members },

/* DeleteMonitoredItemsResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 782},
  .typeIndex = UA_TYPES_DELETEMONITOREDITEMSRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DeleteMonitoredItemsResponse",
#endif
  .memSize = sizeof(UA_DeleteMonitoredItemsResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = DeleteMonitoredItemsResponse_members },

/* RelativePath */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 540},
  .typeIndex = UA_TYPES_RELATIVEPATH,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "RelativePath",
#endif
  .memSize = sizeof(UA_RelativePath),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = RelativePath_members },

/* RegisterNodesRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 558},
  .typeIndex = UA_TYPES_REGISTERNODESREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "RegisterNodesRequest",
#endif
  .memSize = sizeof(UA_RegisterNodesRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = RegisterNodesRequest_members },

/* DeleteNodesRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 498},
  .typeIndex = UA_TYPES_DELETENODESREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DeleteNodesRequest",
#endif
  .memSize = sizeof(UA_DeleteNodesRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = DeleteNodesRequest_members },

/* PublishResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 827},
  .typeIndex = UA_TYPES_PUBLISHRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "PublishResponse",
#endif
  .memSize = sizeof(UA_PublishResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 7,
  .members = PublishResponse_members },

/* MonitoredItemModifyRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 755},
  .typeIndex = UA_TYPES_MONITOREDITEMMODIFYREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "MonitoredItemModifyRequest",
#endif
  .memSize = sizeof(UA_MonitoredItemModifyRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = MonitoredItemModifyRequest_members },

/* UserNameIdentityToken */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 322},
  .typeIndex = UA_TYPES_USERNAMEIDENTITYTOKEN,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "UserNameIdentityToken",
#endif
  .memSize = sizeof(UA_UserNameIdentityToken),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = UserNameIdentityToken_members },

/* IdType */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 6},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "IdType",
#endif
  .memSize = sizeof(UA_IdType),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = IdType_members },

/* UserTokenType */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 6},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "UserTokenType",
#endif
  .memSize = sizeof(UA_UserTokenType),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = UserTokenType_members },

/* NodeAttributes */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 349},
  .typeIndex = UA_TYPES_NODEATTRIBUTES,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "NodeAttributes",
#endif
  .memSize = sizeof(UA_NodeAttributes),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 5,
  .members = NodeAttributes_members },

/* ActivateSessionRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 465},
  .typeIndex = UA_TYPES_ACTIVATESESSIONREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ActivateSessionRequest",
#endif
  .memSize = sizeof(UA_ActivateSessionRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 6,
  .members = ActivateSessionRequest_members },

/* OpenSecureChannelResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 447},
  .typeIndex = UA_TYPES_OPENSECURECHANNELRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "OpenSecureChannelResponse",
#endif
  .memSize = sizeof(UA_OpenSecureChannelResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = OpenSecureChannelResponse_members },

/* ApplicationType */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 6},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ApplicationType",
#endif
  .memSize = sizeof(UA_ApplicationType),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = ApplicationType_members },

/* QueryNextResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 622},
  .typeIndex = UA_TYPES_QUERYNEXTRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "QueryNextResponse",
#endif
  .memSize = sizeof(UA_QueryNextResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = QueryNextResponse_members },

/* ActivateSessionResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 468},
  .typeIndex = UA_TYPES_ACTIVATESESSIONRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ActivateSessionResponse",
#endif
  .memSize = sizeof(UA_ActivateSessionResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = ActivateSessionResponse_members },

/* FilterOperator */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 6},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "FilterOperator",
#endif
  .memSize = sizeof(UA_FilterOperator),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = FilterOperator_members },

/* QueryNextRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 619},
  .typeIndex = UA_TYPES_QUERYNEXTREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "QueryNextRequest",
#endif
  .memSize = sizeof(UA_QueryNextRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = QueryNextRequest_members },

/* BrowseNextRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 531},
  .typeIndex = UA_TYPES_BROWSENEXTREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "BrowseNextRequest",
#endif
  .memSize = sizeof(UA_BrowseNextRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = BrowseNextRequest_members },

/* CreateSubscriptionRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 785},
  .typeIndex = UA_TYPES_CREATESUBSCRIPTIONREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "CreateSubscriptionRequest",
#endif
  .memSize = sizeof(UA_CreateSubscriptionRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 7,
  .members = CreateSubscriptionRequest_members },

/* VariableTypeAttributes */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 364},
  .typeIndex = UA_TYPES_VARIABLETYPEATTRIBUTES,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "VariableTypeAttributes",
#endif
  .memSize = sizeof(UA_VariableTypeAttributes),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 10,
  .members = VariableTypeAttributes_members },

/* BrowsePathResult */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 549},
  .typeIndex = UA_TYPES_BROWSEPATHRESULT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "BrowsePathResult",
#endif
  .memSize = sizeof(UA_BrowsePathResult),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = BrowsePathResult_members },

/* ModifySubscriptionResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 794},
  .typeIndex = UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ModifySubscriptionResponse",
#endif
  .memSize = sizeof(UA_ModifySubscriptionResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = ModifySubscriptionResponse_members },

/* RegisterNodesResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 561},
  .typeIndex = UA_TYPES_REGISTERNODESRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "RegisterNodesResponse",
#endif
  .memSize = sizeof(UA_RegisterNodesResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = RegisterNodesResponse_members },

/* CloseSessionRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 471},
  .typeIndex = UA_TYPES_CLOSESESSIONREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "CloseSessionRequest",
#endif
  .memSize = sizeof(UA_CloseSessionRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = CloseSessionRequest_members },

/* ModifySubscriptionRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 791},
  .typeIndex = UA_TYPES_MODIFYSUBSCRIPTIONREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ModifySubscriptionRequest",
#endif
  .memSize = sizeof(UA_ModifySubscriptionRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 7,
  .members = ModifySubscriptionRequest_members },

/* UserTokenPolicy */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 304},
  .typeIndex = UA_TYPES_USERTOKENPOLICY,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "UserTokenPolicy",
#endif
  .memSize = sizeof(UA_UserTokenPolicy),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 5,
  .members = UserTokenPolicy_members },

/* DeleteMonitoredItemsRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 779},
  .typeIndex = UA_TYPES_DELETEMONITOREDITEMSREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DeleteMonitoredItemsRequest",
#endif
  .memSize = sizeof(UA_DeleteMonitoredItemsRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = DeleteMonitoredItemsRequest_members },

/* ReferenceTypeAttributes */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 367},
  .typeIndex = UA_TYPES_REFERENCETYPEATTRIBUTES,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ReferenceTypeAttributes",
#endif
  .memSize = sizeof(UA_ReferenceTypeAttributes),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 8,
  .members = ReferenceTypeAttributes_members },

/* BrowsePath */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 543},
  .typeIndex = UA_TYPES_BROWSEPATH,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "BrowsePath",
#endif
  .memSize = sizeof(UA_BrowsePath),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = BrowsePath_members },

/* UnregisterNodesResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 567},
  .typeIndex = UA_TYPES_UNREGISTERNODESRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "UnregisterNodesResponse",
#endif
  .memSize = sizeof(UA_UnregisterNodesResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = UnregisterNodesResponse_members },

/* WriteRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 671},
  .typeIndex = UA_TYPES_WRITEREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "WriteRequest",
#endif
  .memSize = sizeof(UA_WriteRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = WriteRequest_members },

/* ObjectAttributes */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 352},
  .typeIndex = UA_TYPES_OBJECTATTRIBUTES,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ObjectAttributes",
#endif
  .memSize = sizeof(UA_ObjectAttributes),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 6,
  .members = ObjectAttributes_members },

/* BrowseDescription */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 514},
  .typeIndex = UA_TYPES_BROWSEDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "BrowseDescription",
#endif
  .memSize = sizeof(UA_BrowseDescription),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 6,
  .members = BrowseDescription_members },

/* RepublishRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 830},
  .typeIndex = UA_TYPES_REPUBLISHREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "RepublishRequest",
#endif
  .memSize = sizeof(UA_RepublishRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = RepublishRequest_members },

/* GetEndpointsRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 426},
  .typeIndex = UA_TYPES_GETENDPOINTSREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "GetEndpointsRequest",
#endif
  .memSize = sizeof(UA_GetEndpointsRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = GetEndpointsRequest_members },

/* PublishRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 824},
  .typeIndex = UA_TYPES_PUBLISHREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "PublishRequest",
#endif
  .memSize = sizeof(UA_PublishRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = PublishRequest_members },

/* AddNodesResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 489},
  .typeIndex = UA_TYPES_ADDNODESRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "AddNodesResponse",
#endif
  .memSize = sizeof(UA_AddNodesResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = AddNodesResponse_members },

/* CloseSecureChannelResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 453},
  .typeIndex = UA_TYPES_CLOSESECURECHANNELRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "CloseSecureChannelResponse",
#endif
  .memSize = sizeof(UA_CloseSecureChannelResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = CloseSecureChannelResponse_members },

/* ModifyMonitoredItemsRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 761},
  .typeIndex = UA_TYPES_MODIFYMONITOREDITEMSREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ModifyMonitoredItemsRequest",
#endif
  .memSize = sizeof(UA_ModifyMonitoredItemsRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = ModifyMonitoredItemsRequest_members },

/* FindServersRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 420},
  .typeIndex = UA_TYPES_FINDSERVERSREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "FindServersRequest",
#endif
  .memSize = sizeof(UA_FindServersRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = FindServersRequest_members },

/* ReferenceDescription */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 518},
  .typeIndex = UA_TYPES_REFERENCEDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ReferenceDescription",
#endif
  .memSize = sizeof(UA_ReferenceDescription),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 7,
  .members = ReferenceDescription_members },

/* SetPublishingModeResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 800},
  .typeIndex = UA_TYPES_SETPUBLISHINGMODERESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "SetPublishingModeResponse",
#endif
  .memSize = sizeof(UA_SetPublishingModeResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = SetPublishingModeResponse_members },

/* ContentFilterResult */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 607},
  .typeIndex = UA_TYPES_CONTENTFILTERRESULT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ContentFilterResult",
#endif
  .memSize = sizeof(UA_ContentFilterResult),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = ContentFilterResult_members },

/* AddReferencesItem */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 379},
  .typeIndex = UA_TYPES_ADDREFERENCESITEM,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "AddReferencesItem",
#endif
  .memSize = sizeof(UA_AddReferencesItem),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 6,
  .members = AddReferencesItem_members },

/* QueryDataDescription */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 570},
  .typeIndex = UA_TYPES_QUERYDATADESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "QueryDataDescription",
#endif
  .memSize = sizeof(UA_QueryDataDescription),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = QueryDataDescription_members },

/* CreateSubscriptionResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 788},
  .typeIndex = UA_TYPES_CREATESUBSCRIPTIONRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "CreateSubscriptionResponse",
#endif
  .memSize = sizeof(UA_CreateSubscriptionResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 5,
  .members = CreateSubscriptionResponse_members },

/* DeleteSubscriptionsResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 848},
  .typeIndex = UA_TYPES_DELETESUBSCRIPTIONSRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DeleteSubscriptionsResponse",
#endif
  .memSize = sizeof(UA_DeleteSubscriptionsResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = DeleteSubscriptionsResponse_members },

/* WriteResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 674},
  .typeIndex = UA_TYPES_WRITERESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "WriteResponse",
#endif
  .memSize = sizeof(UA_WriteResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = WriteResponse_members },

/* DeleteReferencesResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 507},
  .typeIndex = UA_TYPES_DELETEREFERENCESRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DeleteReferencesResponse",
#endif
  .memSize = sizeof(UA_DeleteReferencesResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = DeleteReferencesResponse_members },

/* CreateMonitoredItemsResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 752},
  .typeIndex = UA_TYPES_CREATEMONITOREDITEMSRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "CreateMonitoredItemsResponse",
#endif
  .memSize = sizeof(UA_CreateMonitoredItemsResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = CreateMonitoredItemsResponse_members },

/* CallResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 713},
  .typeIndex = UA_TYPES_CALLRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "CallResponse",
#endif
  .memSize = sizeof(UA_CallResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = CallResponse_members },

/* DeleteNodesResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 501},
  .typeIndex = UA_TYPES_DELETENODESRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DeleteNodesResponse",
#endif
  .memSize = sizeof(UA_DeleteNodesResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = DeleteNodesResponse_members },

/* RepublishResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 833},
  .typeIndex = UA_TYPES_REPUBLISHRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "RepublishResponse",
#endif
  .memSize = sizeof(UA_RepublishResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = RepublishResponse_members },

/* MonitoredItemCreateRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 743},
  .typeIndex = UA_TYPES_MONITOREDITEMCREATEREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "MonitoredItemCreateRequest",
#endif
  .memSize = sizeof(UA_MonitoredItemCreateRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = MonitoredItemCreateRequest_members },

/* DeleteReferencesRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 504},
  .typeIndex = UA_TYPES_DELETEREFERENCESREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "DeleteReferencesRequest",
#endif
  .memSize = sizeof(UA_DeleteReferencesRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = DeleteReferencesRequest_members },

/* ModifyMonitoredItemsResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 764},
  .typeIndex = UA_TYPES_MODIFYMONITOREDITEMSRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ModifyMonitoredItemsResponse",
#endif
  .memSize = sizeof(UA_ModifyMonitoredItemsResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = ModifyMonitoredItemsResponse_members },

/* ReadResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 632},
  .typeIndex = UA_TYPES_READRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ReadResponse",
#endif
  .memSize = sizeof(UA_ReadResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = ReadResponse_members },

/* AddReferencesRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 492},
  .typeIndex = UA_TYPES_ADDREFERENCESREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "AddReferencesRequest",
#endif
  .memSize = sizeof(UA_AddReferencesRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = AddReferencesRequest_members },

/* ReadRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 629},
  .typeIndex = UA_TYPES_READREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ReadRequest",
#endif
  .memSize = sizeof(UA_ReadRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = ReadRequest_members },

/* OpenSecureChannelRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 444},
  .typeIndex = UA_TYPES_OPENSECURECHANNELREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "OpenSecureChannelRequest",
#endif
  .memSize = sizeof(UA_OpenSecureChannelRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 6,
  .members = OpenSecureChannelRequest_members },

/* AddNodesItem */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 376},
  .typeIndex = UA_TYPES_ADDNODESITEM,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "AddNodesItem",
#endif
  .memSize = sizeof(UA_AddNodesItem),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 7,
  .members = AddNodesItem_members },

/* ApplicationDescription */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 308},
  .typeIndex = UA_TYPES_APPLICATIONDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ApplicationDescription",
#endif
  .memSize = sizeof(UA_ApplicationDescription),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 7,
  .members = ApplicationDescription_members },

/* NodeTypeDescription */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 573},
  .typeIndex = UA_TYPES_NODETYPEDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "NodeTypeDescription",
#endif
  .memSize = sizeof(UA_NodeTypeDescription),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = NodeTypeDescription_members },

/* FindServersResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 423},
  .typeIndex = UA_TYPES_FINDSERVERSRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "FindServersResponse",
#endif
  .memSize = sizeof(UA_FindServersResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = FindServersResponse_members },

/* ServerStatusDataType */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 862},
  .typeIndex = UA_TYPES_SERVERSTATUSDATATYPE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ServerStatusDataType",
#endif
  .memSize = sizeof(UA_ServerStatusDataType),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 6,
  .members = ServerStatusDataType_members },

/* AddReferencesResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 495},
  .typeIndex = UA_TYPES_ADDREFERENCESRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "AddReferencesResponse",
#endif
  .memSize = sizeof(UA_AddReferencesResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = AddReferencesResponse_members },

/* TranslateBrowsePathsToNodeIdsResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 555},
  .typeIndex = UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "TranslateBrowsePathsToNodeIdsResponse",
#endif
  .memSize = sizeof(UA_TranslateBrowsePathsToNodeIdsResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = TranslateBrowsePathsToNodeIdsResponse_members },

/* ContentFilterElement */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 583},
  .typeIndex = UA_TYPES_CONTENTFILTERELEMENT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ContentFilterElement",
#endif
  .memSize = sizeof(UA_ContentFilterElement),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = ContentFilterElement_members },

/* TranslateBrowsePathsToNodeIdsRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 552},
  .typeIndex = UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "TranslateBrowsePathsToNodeIdsRequest",
#endif
  .memSize = sizeof(UA_TranslateBrowsePathsToNodeIdsRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = TranslateBrowsePathsToNodeIdsRequest_members },

/* CloseSessionResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 474},
  .typeIndex = UA_TYPES_CLOSESESSIONRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "CloseSessionResponse",
#endif
  .memSize = sizeof(UA_CloseSessionResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = CloseSessionResponse_members },

/* ServiceFault */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 395},
  .typeIndex = UA_TYPES_SERVICEFAULT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ServiceFault",
#endif
  .memSize = sizeof(UA_ServiceFault),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = ServiceFault_members },

/* CreateMonitoredItemsRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 749},
  .typeIndex = UA_TYPES_CREATEMONITOREDITEMSREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "CreateMonitoredItemsRequest",
#endif
  .memSize = sizeof(UA_CreateMonitoredItemsRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = CreateMonitoredItemsRequest_members },

/* ContentFilter */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 586},
  .typeIndex = UA_TYPES_CONTENTFILTER,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ContentFilter",
#endif
  .memSize = sizeof(UA_ContentFilter),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 1,
  .members = ContentFilter_members },

/* QueryFirstResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 616},
  .typeIndex = UA_TYPES_QUERYFIRSTRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "QueryFirstResponse",
#endif
  .memSize = sizeof(UA_QueryFirstResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 6,
  .members = QueryFirstResponse_members },

/* AddNodesRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 486},
  .typeIndex = UA_TYPES_ADDNODESREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "AddNodesRequest",
#endif
  .memSize = sizeof(UA_AddNodesRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = AddNodesRequest_members },

/* BrowseRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 525},
  .typeIndex = UA_TYPES_BROWSEREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "BrowseRequest",
#endif
  .memSize = sizeof(UA_BrowseRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 4,
  .members = BrowseRequest_members },

/* BrowseResult */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 522},
  .typeIndex = UA_TYPES_BROWSERESULT,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "BrowseResult",
#endif
  .memSize = sizeof(UA_BrowseResult),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = BrowseResult_members },

/* CreateSessionRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 459},
  .typeIndex = UA_TYPES_CREATESESSIONREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "CreateSessionRequest",
#endif
  .memSize = sizeof(UA_CreateSessionRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 9,
  .members = CreateSessionRequest_members },

/* EndpointDescription */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 312},
  .typeIndex = UA_TYPES_ENDPOINTDESCRIPTION,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "EndpointDescription",
#endif
  .memSize = sizeof(UA_EndpointDescription),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 8,
  .members = EndpointDescription_members },

/* GetEndpointsResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 429},
  .typeIndex = UA_TYPES_GETENDPOINTSRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "GetEndpointsResponse",
#endif
  .memSize = sizeof(UA_GetEndpointsResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = GetEndpointsResponse_members },

/* BrowseNextResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 534},
  .typeIndex = UA_TYPES_BROWSENEXTRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "BrowseNextResponse",
#endif
  .memSize = sizeof(UA_BrowseNextResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = BrowseNextResponse_members },

/* BrowseResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 528},
  .typeIndex = UA_TYPES_BROWSERESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "BrowseResponse",
#endif
  .memSize = sizeof(UA_BrowseResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = BrowseResponse_members },

/* CreateSessionResponse */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 462},
  .typeIndex = UA_TYPES_CREATESESSIONRESPONSE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "CreateSessionResponse",
#endif
  .memSize = sizeof(UA_CreateSessionResponse),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 10,
  .members = CreateSessionResponse_members },

/* QueryFirstRequest */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 613},
  .typeIndex = UA_TYPES_QUERYFIRSTREQUEST,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "QueryFirstRequest",
#endif
  .memSize = sizeof(UA_QueryFirstRequest),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 6,
  .members = QueryFirstRequest_members },
};


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/build/src_generated/ua_transport_generated.c" ***********************************/

/* Generated from Opc.Ua.Types.bsd, Custom.Opc.Ua.Transport.bsd with script /home/travis/build/open62541/open62541/tools/generate_datatypes.py
 * on host testing-worker-linux-docker-8551a97d-3382-linux-4 by user travis at 2016-05-18 08:48:03 */
 

/* SecureConversationMessageAbortBody */
static UA_DataTypeMember SecureConversationMessageAbortBody_members[2] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "error",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "reason",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_SecureConversationMessageAbortBody, reason) - offsetof(UA_SecureConversationMessageAbortBody, error) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* SecureConversationMessageFooter */
static UA_DataTypeMember SecureConversationMessageFooter_members[2] = {
  { .memberTypeIndex = UA_TYPES_BYTE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "padding",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = true
  },
  { .memberTypeIndex = UA_TYPES_BYTE,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "signature",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_SecureConversationMessageFooter, signature) - offsetof(UA_SecureConversationMessageFooter, padding) - sizeof(void*),
    .isArray = false
  },};

/* TcpHelloMessage */
static UA_DataTypeMember TcpHelloMessage_members[6] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "protocolVersion",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "receiveBufferSize",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_TcpHelloMessage, receiveBufferSize) - offsetof(UA_TcpHelloMessage, protocolVersion) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "sendBufferSize",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_TcpHelloMessage, sendBufferSize) - offsetof(UA_TcpHelloMessage, receiveBufferSize) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "maxMessageSize",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_TcpHelloMessage, maxMessageSize) - offsetof(UA_TcpHelloMessage, sendBufferSize) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "maxChunkCount",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_TcpHelloMessage, maxChunkCount) - offsetof(UA_TcpHelloMessage, maxMessageSize) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_STRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "endpointUrl",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_TcpHelloMessage, endpointUrl) - offsetof(UA_TcpHelloMessage, maxChunkCount) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* MessageType */
static UA_DataTypeMember MessageType_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* AsymmetricAlgorithmSecurityHeader */
static UA_DataTypeMember AsymmetricAlgorithmSecurityHeader_members[3] = {
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "securityPolicyUri",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "senderCertificate",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AsymmetricAlgorithmSecurityHeader, senderCertificate) - offsetof(UA_AsymmetricAlgorithmSecurityHeader, securityPolicyUri) - sizeof(UA_ByteString),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_BYTESTRING,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "receiverCertificateThumbprint",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_AsymmetricAlgorithmSecurityHeader, receiverCertificateThumbprint) - offsetof(UA_AsymmetricAlgorithmSecurityHeader, senderCertificate) - sizeof(UA_ByteString),
    .isArray = false
  },};

/* TcpAcknowledgeMessage */
static UA_DataTypeMember TcpAcknowledgeMessage_members[5] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "protocolVersion",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "receiveBufferSize",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_TcpAcknowledgeMessage, receiveBufferSize) - offsetof(UA_TcpAcknowledgeMessage, protocolVersion) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "sendBufferSize",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_TcpAcknowledgeMessage, sendBufferSize) - offsetof(UA_TcpAcknowledgeMessage, receiveBufferSize) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "maxMessageSize",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_TcpAcknowledgeMessage, maxMessageSize) - offsetof(UA_TcpAcknowledgeMessage, sendBufferSize) - sizeof(UA_UInt32),
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "maxChunkCount",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_TcpAcknowledgeMessage, maxChunkCount) - offsetof(UA_TcpAcknowledgeMessage, maxMessageSize) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* SequenceHeader */
static UA_DataTypeMember SequenceHeader_members[2] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "sequenceNumber",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "requestId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_SequenceHeader, requestId) - offsetof(UA_SequenceHeader, sequenceNumber) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* TcpMessageHeader */
static UA_DataTypeMember TcpMessageHeader_members[2] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "messageTypeAndChunkType",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "messageSize",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_TcpMessageHeader, messageSize) - offsetof(UA_TcpMessageHeader, messageTypeAndChunkType) - sizeof(UA_UInt32),
    .isArray = false
  },};

/* ChunkType */
static UA_DataTypeMember ChunkType_members[1] = {
  { .memberTypeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* SymmetricAlgorithmSecurityHeader */
static UA_DataTypeMember SymmetricAlgorithmSecurityHeader_members[1] = {
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "tokenId",
#endif
    .namespaceZero = true,
    .padding = 0,
    .isArray = false
  },};

/* SecureConversationMessageHeader */
static UA_DataTypeMember SecureConversationMessageHeader_members[2] = {
  { .memberTypeIndex = UA_TRANSPORT_TCPMESSAGEHEADER,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "messageHeader",
#endif
    .namespaceZero = false,
    .padding = 0,
    .isArray = false
  },
  { .memberTypeIndex = UA_TYPES_UINT32,
#ifdef UA_ENABLE_TYPENAMES
    .memberName = "secureChannelId",
#endif
    .namespaceZero = true,
    .padding = offsetof(UA_SecureConversationMessageHeader, secureChannelId) - offsetof(UA_SecureConversationMessageHeader, messageHeader) - sizeof(UA_TcpMessageHeader),
    .isArray = false
  },};
const UA_DataType UA_TRANSPORT[UA_TRANSPORT_COUNT] = {

/* SecureConversationMessageAbortBody */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 0},
  .typeIndex = UA_TRANSPORT_SECURECONVERSATIONMESSAGEABORTBODY,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "SecureConversationMessageAbortBody",
#endif
  .memSize = sizeof(UA_SecureConversationMessageAbortBody),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = SecureConversationMessageAbortBody_members },

/* SecureConversationMessageFooter */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 0},
  .typeIndex = UA_TRANSPORT_SECURECONVERSATIONMESSAGEFOOTER,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "SecureConversationMessageFooter",
#endif
  .memSize = sizeof(UA_SecureConversationMessageFooter),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 2,
  .members = SecureConversationMessageFooter_members },

/* TcpHelloMessage */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 0},
  .typeIndex = UA_TRANSPORT_TCPHELLOMESSAGE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "TcpHelloMessage",
#endif
  .memSize = sizeof(UA_TcpHelloMessage),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 6,
  .members = TcpHelloMessage_members },

/* MessageType */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 0},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "MessageType",
#endif
  .memSize = sizeof(UA_MessageType),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = MessageType_members },

/* AsymmetricAlgorithmSecurityHeader */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 0},
  .typeIndex = UA_TRANSPORT_ASYMMETRICALGORITHMSECURITYHEADER,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "AsymmetricAlgorithmSecurityHeader",
#endif
  .memSize = sizeof(UA_AsymmetricAlgorithmSecurityHeader),
  .builtin = false,
  .fixedSize = false,
  .overlayable = false,
  .membersSize = 3,
  .members = AsymmetricAlgorithmSecurityHeader_members },

/* TcpAcknowledgeMessage */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 0},
  .typeIndex = UA_TRANSPORT_TCPACKNOWLEDGEMESSAGE,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "TcpAcknowledgeMessage",
#endif
  .memSize = sizeof(UA_TcpAcknowledgeMessage),
  .builtin = false,
  .fixedSize = true,
  .overlayable = true && UA_BINARY_OVERLAYABLE_INTEGER && UA_BINARY_OVERLAYABLE_INTEGER && offsetof(UA_TcpAcknowledgeMessage, receiveBufferSize) == (offsetof(UA_TcpAcknowledgeMessage, protocolVersion) + sizeof(UA_UInt32)) && UA_BINARY_OVERLAYABLE_INTEGER && offsetof(UA_TcpAcknowledgeMessage, sendBufferSize) == (offsetof(UA_TcpAcknowledgeMessage, receiveBufferSize) + sizeof(UA_UInt32)) && UA_BINARY_OVERLAYABLE_INTEGER && offsetof(UA_TcpAcknowledgeMessage, maxMessageSize) == (offsetof(UA_TcpAcknowledgeMessage, sendBufferSize) + sizeof(UA_UInt32)) && UA_BINARY_OVERLAYABLE_INTEGER && offsetof(UA_TcpAcknowledgeMessage, maxChunkCount) == (offsetof(UA_TcpAcknowledgeMessage, maxMessageSize) + sizeof(UA_UInt32)),
  .membersSize = 5,
  .members = TcpAcknowledgeMessage_members },

/* SequenceHeader */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 0},
  .typeIndex = UA_TRANSPORT_SEQUENCEHEADER,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "SequenceHeader",
#endif
  .memSize = sizeof(UA_SequenceHeader),
  .builtin = false,
  .fixedSize = true,
  .overlayable = true && UA_BINARY_OVERLAYABLE_INTEGER && UA_BINARY_OVERLAYABLE_INTEGER && offsetof(UA_SequenceHeader, requestId) == (offsetof(UA_SequenceHeader, sequenceNumber) + sizeof(UA_UInt32)),
  .membersSize = 2,
  .members = SequenceHeader_members },

/* TcpMessageHeader */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 0},
  .typeIndex = UA_TRANSPORT_TCPMESSAGEHEADER,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "TcpMessageHeader",
#endif
  .memSize = sizeof(UA_TcpMessageHeader),
  .builtin = false,
  .fixedSize = true,
  .overlayable = true && UA_BINARY_OVERLAYABLE_INTEGER && UA_BINARY_OVERLAYABLE_INTEGER && offsetof(UA_TcpMessageHeader, messageSize) == (offsetof(UA_TcpMessageHeader, messageTypeAndChunkType) + sizeof(UA_UInt32)),
  .membersSize = 2,
  .members = TcpMessageHeader_members },

/* ChunkType */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 0},
  .typeIndex = UA_TYPES_INT32,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "ChunkType",
#endif
  .memSize = sizeof(UA_ChunkType),
  .builtin = true,
  .fixedSize = true,
  .overlayable = UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = ChunkType_members },

/* SymmetricAlgorithmSecurityHeader */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 0},
  .typeIndex = UA_TRANSPORT_SYMMETRICALGORITHMSECURITYHEADER,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "SymmetricAlgorithmSecurityHeader",
#endif
  .memSize = sizeof(UA_SymmetricAlgorithmSecurityHeader),
  .builtin = false,
  .fixedSize = true,
  .overlayable = true && UA_BINARY_OVERLAYABLE_INTEGER,
  .membersSize = 1,
  .members = SymmetricAlgorithmSecurityHeader_members },

/* SecureConversationMessageHeader */
{ .typeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 0},
  .typeIndex = UA_TRANSPORT_SECURECONVERSATIONMESSAGEHEADER,
#ifdef UA_ENABLE_TYPENAMES
  .typeName = "SecureConversationMessageHeader",
#endif
  .memSize = sizeof(UA_SecureConversationMessageHeader),
  .builtin = false,
  .fixedSize = true,
  .overlayable = true && true && UA_BINARY_OVERLAYABLE_INTEGER && UA_BINARY_OVERLAYABLE_INTEGER && offsetof(UA_TcpMessageHeader, messageSize) == (offsetof(UA_TcpMessageHeader, messageTypeAndChunkType) + sizeof(UA_UInt32)) && UA_BINARY_OVERLAYABLE_INTEGER && offsetof(UA_SecureConversationMessageHeader, secureChannelId) == (offsetof(UA_SecureConversationMessageHeader, messageHeader) + sizeof(UA_TcpMessageHeader)),
  .membersSize = 2,
  .members = SecureConversationMessageHeader_members },
};


/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/ua_connection.c" ***********************************/


// max message size is 64k
const UA_ConnectionConfig UA_ConnectionConfig_standard =
    {.protocolVersion = 0, .sendBufferSize = 65536, .recvBufferSize = 65536,
     .maxMessageSize = 1048576, .maxChunkCount = 16};

void UA_Connection_init(UA_Connection *connection) {
    connection->state = UA_CONNECTION_CLOSED;
    connection->localConf = UA_ConnectionConfig_standard;
    connection->remoteConf = UA_ConnectionConfig_standard;
    connection->channel = NULL;
    connection->sockfd = 0;
    connection->handle = NULL;
    UA_ByteString_init(&connection->incompleteMessage);
    connection->send = NULL;
    connection->close = NULL;
    connection->recv = NULL;
    connection->getSendBuffer = NULL;
    connection->releaseSendBuffer = NULL;
    connection->releaseRecvBuffer = NULL;
}

void UA_Connection_deleteMembers(UA_Connection *connection) {
    UA_ByteString_deleteMembers(&connection->incompleteMessage);
}

UA_StatusCode
UA_Connection_completeMessages(UA_Connection *connection, UA_ByteString * UA_RESTRICT message,
                              UA_Boolean * UA_RESTRICT realloced) {
    UA_ByteString *current = message;
    *realloced = false;
    if(connection->incompleteMessage.length > 0) {
        /* concat the existing incomplete message with the new message */
        UA_Byte *data = UA_realloc(connection->incompleteMessage.data,
                                   connection->incompleteMessage.length + message->length);
        if(!data) {
            /* not enough memory */
            UA_ByteString_deleteMembers(&connection->incompleteMessage);
            connection->releaseRecvBuffer(connection, message);
            return UA_STATUSCODE_BADOUTOFMEMORY;
        }
        memcpy(&data[connection->incompleteMessage.length], message->data, message->length);
        connection->incompleteMessage.data = data;
        connection->incompleteMessage.length += message->length;
        connection->releaseRecvBuffer(connection, message);
        current = &connection->incompleteMessage;
        *realloced = true;
    }

    /* the while loop sets pos to the first element after the last complete message. if a message
       contains garbage, the buffer length is set to contain only the "good" messages before. */
    size_t pos = 0;
    size_t delete_at = current->length-1; // garbled message after this point
    while(current->length - pos >= 16) {
        UA_UInt32 msgtype = (UA_UInt32)current->data[pos] +
            ((UA_UInt32)current->data[pos+1] << 8) +
            ((UA_UInt32)current->data[pos+2] << 16);
        if(msgtype != ('M' + ('S' << 8) + ('G' << 16)) &&
           msgtype != ('O' + ('P' << 8) + ('N' << 16)) &&
           msgtype != ('H' + ('E' << 8) + ('L' << 16)) &&
           msgtype != ('A' + ('C' << 8) + ('K' << 16)) &&
           msgtype != ('C' + ('L' << 8) + ('O' << 16))) {
            /* the message type is not recognized */
            delete_at = pos; // throw the remaining message away
            break;
        }
        UA_UInt32 length = 0;
        size_t length_pos = pos + 4;
        UA_StatusCode retval = UA_UInt32_decodeBinary(current, &length_pos, &length);
        if(retval != UA_STATUSCODE_GOOD || length < 16 || length > connection->localConf.recvBufferSize) {
            /* the message size is not allowed. throw the remaining bytestring away */
            delete_at = pos;
            break;
        }
        if(length + pos > current->length)
            break; /* the message is incomplete. keep the beginning */
        pos += length;
    }

    /* throw the message away */
    if(delete_at == 0) {
        if(!*realloced) {
            connection->releaseRecvBuffer(connection, message);
            *realloced = true;
        } else
            UA_ByteString_deleteMembers(current);
        return UA_STATUSCODE_GOOD;
    }

    /* no complete message at all */
    if(pos == 0) {
        if(!*realloced) {
            /* store the buffer in the connection */
            UA_ByteString_copy(current, &connection->incompleteMessage);
            connection->releaseRecvBuffer(connection, message);
            *realloced = true;
        } 
        return UA_STATUSCODE_GOOD;
    }

    /* there remains an incomplete message at the end */
    if(current->length != pos) {
        UA_Byte *data = UA_malloc(current->length - pos);
        if(!data) {
            UA_ByteString_deleteMembers(&connection->incompleteMessage);
            if(!*realloced) {
                connection->releaseRecvBuffer(connection, message);
                *realloced = true;
            }
            return UA_STATUSCODE_BADOUTOFMEMORY;
        }
        size_t newlength = current->length - pos;
        memcpy(data, &current->data[pos], newlength);
        current->length = pos;
        if(*realloced)
            *message = *current;
        connection->incompleteMessage.data = data;
        connection->incompleteMessage.length = newlength;
        return UA_STATUSCODE_GOOD;
    }

    if(current == &connection->incompleteMessage) {
        *message = *current;
        connection->incompleteMessage = UA_BYTESTRING_NULL;
    }
    return UA_STATUSCODE_GOOD;
}

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wunused-value"
#endif

void UA_Connection_detachSecureChannel(UA_Connection *connection) {
#ifdef UA_ENABLE_MULTITHREADING
    UA_SecureChannel *channel = connection->channel;
    if(channel)
        uatomic_cmpxchg(&channel->connection, connection, NULL);
    uatomic_set(&connection->channel, NULL);
#else
    if(connection->channel)
        connection->channel->connection = NULL;
    connection->channel = NULL;
#endif
}

void UA_Connection_attachSecureChannel(UA_Connection *connection, UA_SecureChannel *channel) {
#ifdef UA_ENABLE_MULTITHREADING
    if(uatomic_cmpxchg(&channel->connection, NULL, connection) == NULL)
        uatomic_set((void**)&connection->channel, (void*)channel);
#else
    if(channel->connection != NULL)
        return;
    channel->connection = connection;
    connection->channel = channel;
#endif
}

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/ua_securechannel.c" ***********************************/


#define UA_SECURE_MESSAGE_HEADER_LENGTH 24

void UA_SecureChannel_init(UA_SecureChannel *channel) {
    UA_MessageSecurityMode_init(&channel->securityMode);
    UA_ChannelSecurityToken_init(&channel->securityToken);
    UA_ChannelSecurityToken_init(&channel->nextSecurityToken);
    UA_AsymmetricAlgorithmSecurityHeader_init(&channel->clientAsymAlgSettings);
    UA_AsymmetricAlgorithmSecurityHeader_init(&channel->serverAsymAlgSettings);
    UA_ByteString_init(&channel->clientNonce);
    UA_ByteString_init(&channel->serverNonce);
    channel->sequenceNumber = 0;
    channel->connection = NULL;
    LIST_INIT(&channel->sessions);
    LIST_INIT(&channel->chunks);
}

void UA_SecureChannel_deleteMembersCleanup(UA_SecureChannel *channel) {
    UA_AsymmetricAlgorithmSecurityHeader_deleteMembers(&channel->serverAsymAlgSettings);
    UA_ByteString_deleteMembers(&channel->serverNonce);
    UA_AsymmetricAlgorithmSecurityHeader_deleteMembers(&channel->clientAsymAlgSettings);
    UA_ByteString_deleteMembers(&channel->clientNonce);
    UA_ChannelSecurityToken_deleteMembers(&channel->securityToken);
    UA_ChannelSecurityToken_deleteMembers(&channel->nextSecurityToken);
    UA_Connection *c = channel->connection;
    if(c) {
        UA_Connection_detachSecureChannel(c);
        if(c->close)
            c->close(c);
    }
    /* just remove the pointers and free the linked list (not the sessions) */
    struct SessionEntry *se, *temp;
    LIST_FOREACH_SAFE(se, &channel->sessions, pointers, temp) {
        if(se->session)
            se->session->channel = NULL;
        LIST_REMOVE(se, pointers);
        UA_free(se);
    }

    struct ChunkEntry *ch, *temp_ch;
    LIST_FOREACH_SAFE(ch, &channel->chunks, pointers, temp_ch) {
        UA_ByteString_deleteMembers(&ch->bytes);
        LIST_REMOVE(ch, pointers);
        UA_free(ch);
    }
}

//TODO implement real nonce generator - DUMMY function
UA_StatusCode UA_SecureChannel_generateNonce(UA_ByteString *nonce) {
    if(!(nonce->data = UA_malloc(1)))
        return UA_STATUSCODE_BADOUTOFMEMORY;
    nonce->length  = 1;
    nonce->data[0] = 'a';
    return UA_STATUSCODE_GOOD;
}

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wunused-value"
#endif

void UA_SecureChannel_attachSession(UA_SecureChannel *channel, UA_Session *session) {
    struct SessionEntry *se = UA_malloc(sizeof(struct SessionEntry));
    if(!se)
        return;
    se->session = session;
#ifdef UA_ENABLE_MULTITHREADING
    if(uatomic_cmpxchg(&session->channel, NULL, channel) != NULL) {
        UA_free(se);
        return;
    }
#else
    if(session->channel != NULL) {
        UA_free(se);
        return;
    }
    session->channel = channel;
#endif
    LIST_INSERT_HEAD(&channel->sessions, se, pointers);
}

#if (__GNUC__ >= 4 && __GNUC_MINOR__ >= 6)
#pragma GCC diagnostic pop
#endif

void UA_SecureChannel_detachSession(UA_SecureChannel *channel, UA_Session *session) {
    if(session)
        session->channel = NULL;
    struct SessionEntry *se, *temp;
    LIST_FOREACH_SAFE(se, &channel->sessions, pointers, temp) {
        if(se->session != session)
            continue;
        LIST_REMOVE(se, pointers);
        UA_free(se);
        break;
    }
}

UA_Session * UA_SecureChannel_getSession(UA_SecureChannel *channel, UA_NodeId *token) {
    struct SessionEntry *se;
    LIST_FOREACH(se, &channel->sessions, pointers) {
        if(UA_NodeId_equal(&se->session->authenticationToken, token))
            break;
    }
    if(!se)
        return NULL;
    return se->session;
}

void UA_SecureChannel_revolveTokens(UA_SecureChannel *channel) {
    if(channel->nextSecurityToken.tokenId == 0) //no security token issued
        return;

    //FIXME: not thread-safe
    memcpy(&channel->securityToken, &channel->nextSecurityToken, sizeof(UA_ChannelSecurityToken));
    UA_ChannelSecurityToken_init(&channel->nextSecurityToken);
}

static UA_StatusCode
UA_SecureChannel_sendChunk(UA_ChunkInfo *ci, UA_ByteString *dst, size_t offset) {
    UA_SecureChannel *channel = ci->channel;
    UA_Connection *connection = channel->connection;
    if(!connection)
       return UA_STATUSCODE_BADINTERNALERROR;

    /* adjust the buffer where the header was hidden */
    dst->data = &dst->data[-UA_SECURE_MESSAGE_HEADER_LENGTH];
    dst->length += UA_SECURE_MESSAGE_HEADER_LENGTH;
    offset += UA_SECURE_MESSAGE_HEADER_LENGTH;
    ci->messageSizeSoFar += offset;

    UA_Boolean chunkedMsg = (ci->chunksSoFar > 0 || ci->final == false);
    UA_Boolean abortMsg = ((++ci->chunksSoFar >= connection->remoteConf.maxChunkCount ||
                            ci->messageSizeSoFar > connection->remoteConf.maxMessageSize)) && chunkedMsg;

    /* Prepare the chunk headers */
    UA_SecureConversationMessageHeader respHeader;
    respHeader.secureChannelId = channel->securityToken.channelId;
    respHeader.messageHeader.messageTypeAndChunkType = ci->messageType;
    if(!abortMsg) {
        if(ci->final)
            respHeader.messageHeader.messageTypeAndChunkType += UA_CHUNKTYPE_FINAL;
        else
            respHeader.messageHeader.messageTypeAndChunkType += UA_CHUNKTYPE_INTERMEDIATE;
    } else {
        respHeader.messageHeader.messageTypeAndChunkType += UA_CHUNKTYPE_ABORT;
        ci->abort = true;
        UA_StatusCode retval = UA_STATUSCODE_BADTCPMESSAGETOOLARGE;
        UA_String errorMsg = UA_STRING("Encoded message too long");
        offset = UA_SECURE_MESSAGE_HEADER_LENGTH;
        UA_UInt32_encodeBinary(&retval,dst,&offset);
        UA_String_encodeBinary(&errorMsg,dst,&offset);
    }
    respHeader.messageHeader.messageSize = (UA_UInt32)offset;

    UA_SymmetricAlgorithmSecurityHeader symSecHeader;
    symSecHeader.tokenId = channel->securityToken.tokenId;

    UA_SequenceHeader seqHeader;
    seqHeader.requestId = ci->requestId;
#ifndef UA_ENABLE_MULTITHREADING
    seqHeader.sequenceNumber = ++channel->sequenceNumber;
#else
    seqHeader.sequenceNumber = uatomic_add_return(&channel->sequenceNumber, 1);
#endif

    /* Encode the header at the beginning of the buffer */
    size_t offset_header = 0;
    UA_SecureConversationMessageHeader_encodeBinary(&respHeader, dst, &offset_header);
    UA_SymmetricAlgorithmSecurityHeader_encodeBinary(&symSecHeader, dst, &offset_header);
    UA_SequenceHeader_encodeBinary(&seqHeader, dst, &offset_header);

    /* Send the chunk, the buffer is freed in the network layer */
    dst->length = offset; /* set the buffer length to the content length */
    connection->send(channel->connection, dst);

    /* Replace with the buffer for the next chunk */
    if(!ci->final && !ci->abort) {
        UA_StatusCode retval = connection->getSendBuffer(connection, connection->localConf.sendBufferSize, dst);
        if(retval != UA_STATUSCODE_GOOD)
            return retval;
        /* Hide the header of the buffer, so that the ensuing encoding does not overwrite anything */
        dst->data = &dst->data[UA_SECURE_MESSAGE_HEADER_LENGTH];
        dst->length = connection->localConf.sendBufferSize - UA_SECURE_MESSAGE_HEADER_LENGTH;
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
UA_SecureChannel_sendBinaryMessage(UA_SecureChannel *channel, UA_UInt32 requestId, const void *content,
                                   const UA_DataType *contentType) {
    UA_Connection *connection = channel->connection;
    if(!connection)
        return UA_STATUSCODE_BADINTERNALERROR;

    /* Allocate the message buffer */
    UA_ByteString message;
    UA_StatusCode retval = connection->getSendBuffer(connection, connection->localConf.sendBufferSize, &message);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;

    /* Hide the message beginning where the header will be encoded */
    message.data = &message.data[UA_SECURE_MESSAGE_HEADER_LENGTH];
    message.length -= UA_SECURE_MESSAGE_HEADER_LENGTH;

    /* Encode the message type */
    size_t messagePos = 0;
    UA_NodeId typeId = contentType->typeId; /* always numeric */
    typeId.identifier.numeric += UA_ENCODINGOFFSET_BINARY;
    UA_NodeId_encodeBinary(&typeId, &message, &messagePos);

    /* Encode with the chunking callback */
    UA_ChunkInfo ci;
    ci.channel = channel;
    ci.requestId = requestId;
    ci.chunksSoFar = 0;
    ci.messageSizeSoFar = 0;
    ci.final = false;
    ci.messageType = UA_MESSAGETYPE_MSG;
    ci.abort = false;
    if(typeId.identifier.numeric == 446 || typeId.identifier.numeric == 449)
        ci.messageType = UA_MESSAGETYPE_OPN;
    else if(typeId.identifier.numeric == 452 || typeId.identifier.numeric == 455)
        ci.messageType = UA_MESSAGETYPE_CLO;
    retval = UA_encodeBinary(content, contentType, (UA_exchangeEncodeBuffer)UA_SecureChannel_sendChunk,
                             &ci, &message, &messagePos);

    /* Abort message was sent, the buffer is already freed */
    if(ci.abort)
        return retval;

    /* Encoding failed, release the message */
    if(retval != UA_STATUSCODE_GOOD) {
        /* Unhide the beginning of the buffer (header) */
        message.data = &message.data[-UA_SECURE_MESSAGE_HEADER_LENGTH];
        connection->releaseSendBuffer(connection, &message);
        return retval;
    }

    /* Encoding finished, send the final chunk */
    ci.final = UA_TRUE;
    return UA_SecureChannel_sendChunk(&ci, &message, messagePos);
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/ua_session.c" ***********************************/

#ifdef UA_ENABLE_SUBSCRIPTIONS
#endif

UA_Session adminSession = {
    .clientDescription =  {.applicationUri = {0, NULL}, .productUri = {0, NULL},
                           .applicationName = {.locale = {0, NULL}, .text = {0, NULL}},
                           .applicationType = UA_APPLICATIONTYPE_CLIENT,
                           .gatewayServerUri = {0, NULL}, .discoveryProfileUri = {0, NULL},
                           .discoveryUrlsSize = 0, .discoveryUrls = NULL},
    .sessionName = {sizeof("Administrator Session")-1, (UA_Byte*)"Administrator Session"},
    .authenticationToken = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC,
                            .identifier.numeric = 1},
    .sessionId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC, .identifier.numeric = 1},
    .maxRequestMessageSize = UA_UINT32_MAX, .maxResponseMessageSize = UA_UINT32_MAX,
    .timeout = (UA_Double)UA_INT64_MAX, .validTill = UA_INT64_MAX, .channel = NULL,
    .continuationPoints = {NULL}};

void UA_Session_init(UA_Session *session) {
    UA_ApplicationDescription_init(&session->clientDescription);
    session->activated = false;
    UA_NodeId_init(&session->authenticationToken);
    UA_NodeId_init(&session->sessionId);
    UA_String_init(&session->sessionName);
    session->maxRequestMessageSize  = 0;
    session->maxResponseMessageSize = 0;
    session->timeout = 0;
    UA_DateTime_init(&session->validTill);
    session->channel = NULL;
    session->availableContinuationPoints = MAXCONTINUATIONPOINTS;
    LIST_INIT(&session->continuationPoints);
#ifdef UA_ENABLE_SUBSCRIPTIONS
    LIST_INIT(&session->serverSubscriptions);
    session->lastSubscriptionID = UA_UInt32_random();
    SIMPLEQ_INIT(&session->responseQueue);
#endif
}

void UA_Session_deleteMembersCleanup(UA_Session *session, UA_Server* server) {
    UA_ApplicationDescription_deleteMembers(&session->clientDescription);
    UA_NodeId_deleteMembers(&session->authenticationToken);
    UA_NodeId_deleteMembers(&session->sessionId);
    UA_String_deleteMembers(&session->sessionName);
    struct ContinuationPointEntry *cp, *temp;
    LIST_FOREACH_SAFE(cp, &session->continuationPoints, pointers, temp) {
        LIST_REMOVE(cp, pointers);
        UA_ByteString_deleteMembers(&cp->identifier);
        UA_BrowseDescription_deleteMembers(&cp->browseDescription);
        UA_free(cp);
    }
    if(session->channel)
        UA_SecureChannel_detachSession(session->channel, session);
#ifdef UA_ENABLE_SUBSCRIPTIONS
    UA_Subscription *currents, *temps;
    LIST_FOREACH_SAFE(currents, &session->serverSubscriptions, listEntry, temps) {
        LIST_REMOVE(currents, listEntry);
        UA_Subscription_deleteMembers(currents, server);
        UA_free(currents);
    }
    UA_PublishResponseEntry *entry;
    while((entry = SIMPLEQ_FIRST(&session->responseQueue))) {
        SIMPLEQ_REMOVE_HEAD(&session->responseQueue, listEntry);
        UA_PublishResponse_deleteMembers(&entry->response);
        UA_free(entry);
    }
#endif
}

void UA_Session_updateLifetime(UA_Session *session) {
    session->validTill = UA_DateTime_now() + (UA_DateTime)(session->timeout * UA_MSEC_TO_DATETIME);
}

#ifdef UA_ENABLE_SUBSCRIPTIONS

void UA_Session_addSubscription(UA_Session *session, UA_Subscription *newSubscription) {
    LIST_INSERT_HEAD(&session->serverSubscriptions, newSubscription, listEntry);
}

UA_StatusCode
UA_Session_deleteSubscription(UA_Server *server, UA_Session *session, UA_UInt32 subscriptionID) {
    UA_Subscription *sub = UA_Session_getSubscriptionByID(session, subscriptionID);    
    if(!sub)
        return UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
    LIST_REMOVE(sub, listEntry);
    UA_Subscription_deleteMembers(sub, server);
    UA_free(sub);
    return UA_STATUSCODE_GOOD;
} 

UA_Subscription *
UA_Session_getSubscriptionByID(UA_Session *session, UA_UInt32 subscriptionID) {
    UA_Subscription *sub;
    LIST_FOREACH(sub, &session->serverSubscriptions, listEntry) {
        if(sub->subscriptionID == subscriptionID)
            break;
    }
    return sub;
}

UA_UInt32 UA_Session_getUniqueSubscriptionID(UA_Session *session) {
    return ++(session->lastSubscriptionID);
}


#endif

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_server.c" ***********************************/


#ifdef UA_ENABLE_GENERATE_NAMESPACE0
#endif

#if defined(UA_ENABLE_MULTITHREADING) && !defined(NDEBUG)
UA_THREAD_LOCAL bool rcu_locked = false;
#endif

static const UA_NodeId nodeIdHasSubType = {
    .namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC,
    .identifier.numeric = UA_NS0ID_HASSUBTYPE};
static const UA_NodeId nodeIdHasTypeDefinition = {
    .namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC,
    .identifier.numeric = UA_NS0ID_HASTYPEDEFINITION};
static const UA_NodeId nodeIdHasComponent = {
    .namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC,
    .identifier.numeric = UA_NS0ID_HASCOMPONENT};
static const UA_NodeId nodeIdHasProperty = {
    .namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC,
    .identifier.numeric = UA_NS0ID_HASPROPERTY};
static const UA_NodeId nodeIdOrganizes = {
    .namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC,
    .identifier.numeric = UA_NS0ID_ORGANIZES};

static const UA_ExpandedNodeId expandedNodeIdBaseDataVariabletype = {
    .nodeId = {.namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC,
               .identifier.numeric = UA_NS0ID_BASEDATAVARIABLETYPE},
    .namespaceUri = {.length = 0, .data = NULL}, .serverIndex = 0};

#ifndef UA_ENABLE_GENERATE_NAMESPACE0
static const UA_NodeId nodeIdNonHierarchicalReferences = {
        .namespaceIndex = 0, .identifierType = UA_NODEIDTYPE_NUMERIC,
        .identifier.numeric = UA_NS0ID_NONHIERARCHICALREFERENCES};
#endif

/**********************/
/* Namespace Handling */
/**********************/

#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
static void UA_ExternalNamespace_init(UA_ExternalNamespace *ens) {
    ens->index = 0;
    UA_String_init(&ens->url);
}

static void UA_ExternalNamespace_deleteMembers(UA_ExternalNamespace *ens) {
    UA_String_deleteMembers(&ens->url);
    ens->externalNodeStore.destroy(ens->externalNodeStore.ensHandle);
}

static void UA_Server_deleteExternalNamespaces(UA_Server *server) {
	for(UA_UInt32 i = 0; i < server->externalNamespacesSize; i++){
		UA_ExternalNamespace_deleteMembers(&(server->externalNamespaces[i]));
	}
	if(server->externalNamespacesSize > 0){
		UA_free(server->externalNamespaces);
		server->externalNamespaces = NULL;
		server->externalNamespacesSize = 0;
	}
}

UA_StatusCode UA_EXPORT
UA_Server_addExternalNamespace(UA_Server *server,
                               const UA_String *url, UA_ExternalNodeStore *nodeStore,UA_UInt16 *assignedNamespaceIndex) {
	if (nodeStore == NULL)
		return UA_STATUSCODE_BADARGUMENTSMISSING;
	UA_UInt32 size = server->externalNamespacesSize;
	server->externalNamespaces =
		UA_realloc(server->externalNamespaces, sizeof(UA_ExternalNamespace) * (size + 1));
	server->externalNamespaces[size].externalNodeStore = *nodeStore;
	server->externalNamespaces[size].index = server->namespacesSize;
	*assignedNamespaceIndex  = server->namespacesSize;
	UA_String_copy(url, &server->externalNamespaces[size].url);
	server->externalNamespacesSize++;
	UA_Server_addNamespace(server, url);
	return UA_STATUSCODE_GOOD;
}
#endif /* UA_ENABLE_EXTERNAL_NAMESPACES*/

static UA_UInt16 addNamespaceInternal(UA_Server *server, UA_String *name) {
    //check if the namespace already exists in the server's namespace array
    for(UA_UInt16 i=0;i<server->namespacesSize;i++){
        if(UA_String_equal(name, &(server->namespaces[i])))
            return i;
    }
    //the namespace URI did not match - add a new namespace to the namsepace array
	server->namespaces = UA_realloc(server->namespaces,
		sizeof(UA_String) * (server->namespacesSize + 1));
	UA_String_copy(name, &(server->namespaces[server->namespacesSize]));
	server->namespacesSize++;
	return (UA_UInt16)(server->namespacesSize - 1);
}

UA_UInt16 UA_Server_addNamespace(UA_Server *server, const char* name) {
	UA_String nameString = UA_STRING_ALLOC(name);
	return addNamespaceInternal(server, &nameString);
}

UA_StatusCode
UA_Server_deleteNode(UA_Server *server, const UA_NodeId nodeId, UA_Boolean deleteReferences) {
    UA_RCU_LOCK();
    UA_StatusCode retval = Service_DeleteNodes_single(server, &adminSession, &nodeId, deleteReferences);
    UA_RCU_UNLOCK();
    return retval;
}

UA_StatusCode
UA_Server_deleteReference(UA_Server *server, const UA_NodeId sourceNodeId, const UA_NodeId referenceTypeId,
                          UA_Boolean isForward, const UA_ExpandedNodeId targetNodeId,
                          UA_Boolean deleteBidirectional) {
    UA_DeleteReferencesItem item;
    item.sourceNodeId = sourceNodeId;
    item.referenceTypeId = referenceTypeId;
    item.isForward = isForward;
    item.targetNodeId = targetNodeId;
    item.deleteBidirectional = deleteBidirectional;
    UA_RCU_LOCK();
    UA_StatusCode retval = Service_DeleteReferences_single(server, &adminSession, &item);
    UA_RCU_UNLOCK();
    return retval;
}

UA_StatusCode
UA_Server_forEachChildNodeCall(UA_Server *server, UA_NodeId parentNodeId,
                               UA_NodeIteratorCallback callback, void *handle) {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_RCU_LOCK();
    const UA_Node *parent = UA_NodeStore_get(server->nodestore, &parentNodeId);
    if(!parent) {
        UA_RCU_UNLOCK();
        return UA_STATUSCODE_BADNODEIDINVALID;
    }
    for(size_t i = 0; i < parent->referencesSize; i++) {
        UA_ReferenceNode *ref = &parent->references[i];
        retval |= callback(ref->targetId.nodeId, ref->isInverse,
                           ref->referenceTypeId, handle);
    }
    UA_RCU_UNLOCK();
    return retval;
}

UA_StatusCode
UA_Server_addReference(UA_Server *server, const UA_NodeId sourceId,
                       const UA_NodeId refTypeId, const UA_ExpandedNodeId targetId,
                       UA_Boolean isForward) {
    UA_AddReferencesItem item;
    UA_AddReferencesItem_init(&item);
    item.sourceNodeId = sourceId;
    item.referenceTypeId = refTypeId;
    item.isForward = isForward;
    item.targetNodeId = targetId;
    UA_RCU_LOCK();
    UA_StatusCode retval = Service_AddReferences_single(server, &adminSession, &item);
    UA_RCU_UNLOCK();
    return retval;
}

static UA_StatusCode
addReferenceInternal(UA_Server *server, const UA_NodeId sourceId, const UA_NodeId refTypeId,
                     const UA_ExpandedNodeId targetId, UA_Boolean isForward) {
    UA_AddReferencesItem item;
    UA_AddReferencesItem_init(&item);
    item.sourceNodeId = sourceId;
    item.referenceTypeId = refTypeId;
    item.isForward = isForward;
    item.targetNodeId = targetId;
    UA_RCU_LOCK();
    UA_StatusCode retval = Service_AddReferences_single(server, &adminSession, &item);
    UA_RCU_UNLOCK();
    return retval;
}

static UA_AddNodesResult
addNodeInternal(UA_Server *server, UA_Node *node, const UA_NodeId parentNodeId,
                const UA_NodeId referenceTypeId) {
    UA_AddNodesResult res;
    UA_AddNodesResult_init(&res);
    UA_RCU_LOCK();
    Service_AddNodes_existing(server, &adminSession, node, &parentNodeId,
                              &referenceTypeId, &res);
    UA_RCU_UNLOCK();
    return res;
}

UA_StatusCode
__UA_Server_addNode(UA_Server *server, const UA_NodeClass nodeClass,
                    const UA_NodeId requestedNewNodeId, const UA_NodeId parentNodeId,
                    const UA_NodeId referenceTypeId, const UA_QualifiedName browseName,
                    const UA_NodeId typeDefinition, const UA_NodeAttributes *attr,
                    const UA_DataType *attributeType, 
                    UA_InstantiationCallback *instantiationCallback, UA_NodeId *outNewNodeId) {
    UA_AddNodesItem item;
    UA_AddNodesItem_init(&item);
    item.parentNodeId.nodeId = parentNodeId;
    item.referenceTypeId = referenceTypeId;
    item.requestedNewNodeId.nodeId = requestedNewNodeId;
    item.browseName = browseName;
    item.nodeClass = nodeClass;
    item.typeDefinition.nodeId = typeDefinition;
    item.nodeAttributes = (UA_ExtensionObject){.encoding = UA_EXTENSIONOBJECT_DECODED_NODELETE,
                                               .content.decoded = {attributeType, (void*)(uintptr_t)attr}};
    UA_AddNodesResult result;
    UA_AddNodesResult_init(&result);
    UA_RCU_LOCK();
    Service_AddNodes_single(server, &adminSession, &item, &result, instantiationCallback);
    UA_RCU_UNLOCK();

    if(outNewNodeId && result.statusCode == UA_STATUSCODE_GOOD)
        *outNewNodeId = result.addedNodeId;
    else
        UA_AddNodesResult_deleteMembers(&result);
    return result.statusCode;
}

/**********/
/* Server */
/**********/

/* The server needs to be stopped before it can be deleted */
void UA_Server_delete(UA_Server *server) {
    // Delete the timed work
    UA_Server_deleteAllRepeatedJobs(server);

    // Delete all internal data
    UA_SecureChannelManager_deleteMembers(&server->secureChannelManager);
    UA_SessionManager_deleteMembers(&server->sessionManager);
    UA_RCU_LOCK();
    UA_NodeStore_delete(server->nodestore);
    UA_RCU_UNLOCK();
#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
    UA_Server_deleteExternalNamespaces(server);
#endif
    UA_Array_delete(server->namespaces, server->namespacesSize, &UA_TYPES[UA_TYPES_STRING]);
    UA_Array_delete(server->endpointDescriptions, server->endpointDescriptionsSize,
                    &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);

#ifdef UA_ENABLE_MULTITHREADING
    pthread_cond_destroy(&server->dispatchQueue_condition);
#endif
    UA_free(server);
}

/* Recurring cleanup. Removing unused and timed-out channels and sessions */
static void UA_Server_cleanup(UA_Server *server, void *_) {
    UA_DateTime now = UA_DateTime_now();
    UA_SessionManager_cleanupTimedOut(&server->sessionManager, now);
    UA_SecureChannelManager_cleanupTimedOut(&server->secureChannelManager, now);
}

static UA_StatusCode
readStatus(void *handle, const UA_NodeId nodeid, UA_Boolean sourceTimeStamp,
           const UA_NumericRange *range, UA_DataValue *value) {
    if(range) {
        value->hasStatus = true;
        value->status = UA_STATUSCODE_BADINDEXRANGEINVALID;
        return UA_STATUSCODE_GOOD;
    }

    UA_Server *server = (UA_Server*)handle;
    UA_ServerStatusDataType *status = UA_ServerStatusDataType_new();
    status->startTime = server->startTime;
    status->currentTime = UA_DateTime_now();
    status->state = UA_SERVERSTATE_RUNNING;
    status->secondsTillShutdown = 0;

    value->value.type = &UA_TYPES[UA_TYPES_SERVERSTATUSDATATYPE];
    value->value.arrayLength = 0;
    value->value.data = status;
    value->value.arrayDimensionsSize = 0;
    value->value.arrayDimensions = NULL;
    value->hasValue = true;
    if(sourceTimeStamp) {
        value->hasSourceTimestamp = true;
        value->sourceTimestamp = UA_DateTime_now();
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
readNamespaces(void *handle, const UA_NodeId nodeid, UA_Boolean sourceTimestamp,
               const UA_NumericRange *range, UA_DataValue *value) {
    if(range) {
        value->hasStatus = true;
        value->status = UA_STATUSCODE_BADINDEXRANGEINVALID;
        return UA_STATUSCODE_GOOD;
    }
    UA_Server *server = (UA_Server*)handle;
    UA_StatusCode retval;
    retval = UA_Variant_setArrayCopy(&value->value, server->namespaces,
                                     server->namespacesSize, &UA_TYPES[UA_TYPES_STRING]);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    value->hasValue = true;
    if(sourceTimestamp) {
        value->hasSourceTimestamp = true;
        value->sourceTimestamp = UA_DateTime_now();
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
readCurrentTime(void *handle, const UA_NodeId nodeid, UA_Boolean sourceTimeStamp,
                const UA_NumericRange *range, UA_DataValue *value) {
    if(range) {
        value->hasStatus = true;
        value->status = UA_STATUSCODE_BADINDEXRANGEINVALID;
        return UA_STATUSCODE_GOOD;
    }
    UA_DateTime currentTime = UA_DateTime_now();
    UA_StatusCode retval = UA_Variant_setScalarCopy(&value->value, &currentTime, &UA_TYPES[UA_TYPES_DATETIME]);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    value->hasValue = true;
    if(sourceTimeStamp) {
        value->hasSourceTimestamp = true;
        value->sourceTimestamp = currentTime;
    }
    return UA_STATUSCODE_GOOD;
}

static void copyNames(UA_Node *node, char *name) {
    node->browseName = UA_QUALIFIEDNAME_ALLOC(0, name);
    node->displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", name);
    node->description = UA_LOCALIZEDTEXT_ALLOC("en_US", name);
}

static void
addDataTypeNode(UA_Server *server, char* name, UA_UInt32 datatypeid, UA_UInt32 parent) {
    UA_DataTypeNode *datatype = UA_NodeStore_newDataTypeNode();
    copyNames((UA_Node*)datatype, name);
    datatype->nodeId.identifier.numeric = datatypeid;
    addNodeInternal(server, (UA_Node*)datatype, UA_NODEID_NUMERIC(0, parent), nodeIdOrganizes);
}

static void
addObjectTypeNode(UA_Server *server, char* name, UA_UInt32 objecttypeid,
                  UA_UInt32 parent, UA_UInt32 parentreference) {
    UA_ObjectTypeNode *objecttype = UA_NodeStore_newObjectTypeNode();
    copyNames((UA_Node*)objecttype, name);
    objecttype->nodeId.identifier.numeric = objecttypeid;
    addNodeInternal(server, (UA_Node*)objecttype, UA_NODEID_NUMERIC(0, parent),
                    UA_NODEID_NUMERIC(0, parentreference));
}

static UA_VariableTypeNode*
createVariableTypeNode(UA_Server *server, char* name, UA_UInt32 variabletypeid,
                       UA_UInt32 parent, UA_Boolean abstract) {
    UA_VariableTypeNode *variabletype = UA_NodeStore_newVariableTypeNode();
    copyNames((UA_Node*)variabletype, name);
    variabletype->nodeId.identifier.numeric = variabletypeid;
    variabletype->isAbstract = abstract;
    variabletype->value.variant.value.type = &UA_TYPES[UA_TYPES_VARIANT];
    return variabletype;
}

static void
addVariableTypeNode_organized(UA_Server *server, char* name, UA_UInt32 variabletypeid,
                              UA_UInt32 parent, UA_Boolean abstract) {
    UA_VariableTypeNode *variabletype = createVariableTypeNode(server, name, variabletypeid, parent, abstract);
    addNodeInternal(server, (UA_Node*)variabletype, UA_NODEID_NUMERIC(0, parent), nodeIdOrganizes);
}

static void
addVariableTypeNode_subtype(UA_Server *server, char* name, UA_UInt32 variabletypeid,
                            UA_UInt32 parent, UA_Boolean abstract) {
    UA_VariableTypeNode *variabletype =
        createVariableTypeNode(server, name, variabletypeid, parent, abstract);
    addNodeInternal(server, (UA_Node*)variabletype, UA_NODEID_NUMERIC(0, parent), nodeIdHasSubType);
}

UA_Server * UA_Server_new(const UA_ServerConfig config) {
    UA_Server *server = UA_calloc(1, sizeof(UA_Server));
    if(!server)
        return NULL;

    server->config = config;
    server->nodestore = UA_NodeStore_new();
    LIST_INIT(&server->repeatedJobs);

#ifdef UA_ENABLE_MULTITHREADING
    rcu_init();
    cds_wfcq_init(&server->dispatchQueue_head, &server->dispatchQueue_tail);
    cds_lfs_init(&server->mainLoopJobs);
#endif

    /* uncomment for non-reproducible server runs */
    //UA_random_seed(UA_DateTime_now());

    /* ns0 and ns1 */
    server->namespaces = UA_Array_new(2, &UA_TYPES[UA_TYPES_STRING]);
    server->namespaces[0] = UA_STRING_ALLOC("http://opcfoundation.org/UA/");
    UA_String_copy(&server->config.applicationDescription.applicationUri, &server->namespaces[1]);
    server->namespacesSize = 2;

    server->endpointDescriptions = UA_Array_new(server->config.networkLayersSize,
                                                &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
    server->endpointDescriptionsSize = server->config.networkLayersSize;
    for(size_t i = 0; i < server->config.networkLayersSize; i++) {
        UA_EndpointDescription *endpoint = &server->endpointDescriptions[i];
        endpoint->securityMode = UA_MESSAGESECURITYMODE_NONE;
        endpoint->securityPolicyUri =
            UA_STRING_ALLOC("http://opcfoundation.org/UA/SecurityPolicy#None");
        endpoint->transportProfileUri =
            UA_STRING_ALLOC("http://opcfoundation.org/UA-Profile/Transport/uatcp-uasc-uabinary");

        size_t policies = 0;
        if(server->config.enableAnonymousLogin)
            policies++;
        if(server->config.enableUsernamePasswordLogin)
            policies++;
        endpoint->userIdentityTokensSize = policies;
        endpoint->userIdentityTokens = UA_Array_new(policies, &UA_TYPES[UA_TYPES_USERTOKENPOLICY]);

        size_t currentIndex = 0;
        if(server->config.enableAnonymousLogin) {
            UA_UserTokenPolicy_init(&endpoint->userIdentityTokens[currentIndex]);
            endpoint->userIdentityTokens[currentIndex].tokenType = UA_USERTOKENTYPE_ANONYMOUS;
            endpoint->userIdentityTokens[currentIndex].policyId = UA_STRING_ALLOC(ANONYMOUS_POLICY);
            currentIndex++;
        }
        if(server->config.enableUsernamePasswordLogin) {
            UA_UserTokenPolicy_init(&endpoint->userIdentityTokens[currentIndex]);
            endpoint->userIdentityTokens[currentIndex].tokenType = UA_USERTOKENTYPE_USERNAME;
            endpoint->userIdentityTokens[currentIndex].policyId = UA_STRING_ALLOC(USERNAME_POLICY);
        }

        /* The standard says "the HostName specified in the Server Certificate is the
           same as the HostName contained in the endpointUrl provided in the
           EndpointDescription */
        UA_String_copy(&server->config.serverCertificate, &endpoint->serverCertificate);
        UA_ApplicationDescription_copy(&server->config.applicationDescription, &endpoint->server);
        
        /* copy the discovery url only once the networlayer has been started */
        // UA_String_copy(&server->config.networkLayers[i].discoveryUrl, &endpoint->endpointUrl);
    } 

#define MAXCHANNELCOUNT 100
#define STARTCHANNELID 1
#define TOKENLIFETIME 600000 //this is in milliseconds //600000 seems to be the minimal allowet time for UaExpert
#define STARTTOKENID 1
    UA_SecureChannelManager_init(&server->secureChannelManager, MAXCHANNELCOUNT,
                                 TOKENLIFETIME, STARTCHANNELID, STARTTOKENID, server);

#define MAXSESSIONCOUNT 1000
#define MAXSESSIONLIFETIME 3600000
#define STARTSESSIONID 1
    UA_SessionManager_init(&server->sessionManager, MAXSESSIONCOUNT, MAXSESSIONLIFETIME,
                           STARTSESSIONID, server);

    UA_Job cleanup = {.type = UA_JOBTYPE_METHODCALL,
                      .job.methodCall = {.method = UA_Server_cleanup, .data = NULL} };
    UA_Server_addRepeatedJob(server, cleanup, 10000, NULL);

    /**********************/
    /* Server Information */
    /**********************/

    server->startTime = UA_DateTime_now();

    /**************/
    /* References */
    /**************/
#ifndef UA_ENABLE_GENERATE_NAMESPACE0
    /* Bootstrap by manually inserting "references" and "hassubtype" */
    UA_ReferenceTypeNode *references = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)references, "References");
    references->nodeId.identifier.numeric = UA_NS0ID_REFERENCES;
    references->isAbstract = true;
    references->symmetric = true;
    references->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "References");
    /* The reference to root is later inserted */
    UA_RCU_LOCK();
    UA_NodeStore_insert(server->nodestore, (UA_Node*)references);
    UA_RCU_UNLOCK();

    UA_ReferenceTypeNode *hassubtype = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)hassubtype, "HasSubtype");
    hassubtype->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "HasSupertype");
    hassubtype->nodeId.identifier.numeric = UA_NS0ID_HASSUBTYPE;
    hassubtype->isAbstract = false;
    hassubtype->symmetric = false;
    /* The reference to root is later inserted */
    UA_RCU_LOCK();
    UA_NodeStore_insert(server->nodestore, (UA_Node*)hassubtype);
    UA_RCU_UNLOCK();

    /* Continue adding reference types with normal "addnode" */
    UA_ReferenceTypeNode *hierarchicalreferences = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)hierarchicalreferences, "Hierarchicalreferences");
    hierarchicalreferences->nodeId.identifier.numeric = UA_NS0ID_HIERARCHICALREFERENCES;
    hierarchicalreferences->isAbstract = true;
    hierarchicalreferences->symmetric  = false;
    addNodeInternal(server, (UA_Node*)hierarchicalreferences,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_REFERENCES), nodeIdHasSubType);

    UA_ReferenceTypeNode *nonhierarchicalreferences = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)nonhierarchicalreferences, "NonHierarchicalReferences");
    nonhierarchicalreferences->nodeId.identifier.numeric = UA_NS0ID_NONHIERARCHICALREFERENCES;
    nonhierarchicalreferences->isAbstract = true;
    nonhierarchicalreferences->symmetric  = false;
    addNodeInternal(server, (UA_Node*)nonhierarchicalreferences,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_REFERENCES), nodeIdHasSubType);

    UA_ReferenceTypeNode *haschild = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)haschild, "HasChild");
    haschild->nodeId.identifier.numeric = UA_NS0ID_HASCHILD;
    haschild->isAbstract = true;
    haschild->symmetric  = false;
    addNodeInternal(server, (UA_Node*)haschild,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), nodeIdHasSubType);

    UA_ReferenceTypeNode *organizes = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)organizes, "Organizes");
    organizes->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "OrganizedBy");
    organizes->nodeId.identifier.numeric = UA_NS0ID_ORGANIZES;
    organizes->isAbstract = false;
    organizes->symmetric  = false;
    addNodeInternal(server, (UA_Node*)organizes,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), nodeIdHasSubType);

    UA_ReferenceTypeNode *haseventsource = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)haseventsource, "HasEventSource");
    haseventsource->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "EventSourceOf");
    haseventsource->nodeId.identifier.numeric = UA_NS0ID_HASEVENTSOURCE;
    haseventsource->isAbstract = false;
    haseventsource->symmetric  = false;
    addNodeInternal(server, (UA_Node*)haseventsource,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_HIERARCHICALREFERENCES), nodeIdHasSubType);

    UA_ReferenceTypeNode *hasmodellingrule = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)hasmodellingrule, "HasModellingRule");
    hasmodellingrule->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "ModellingRuleOf");
    hasmodellingrule->nodeId.identifier.numeric = UA_NS0ID_HASMODELLINGRULE;
    hasmodellingrule->isAbstract = false;
    hasmodellingrule->symmetric  = false;
    addNodeInternal(server, (UA_Node*)hasmodellingrule, nodeIdNonHierarchicalReferences, nodeIdHasSubType);

    UA_ReferenceTypeNode *hasencoding = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)hasencoding, "HasEncoding");
    hasencoding->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "EncodingOf");
    hasencoding->nodeId.identifier.numeric = UA_NS0ID_HASENCODING;
    hasencoding->isAbstract = false;
    hasencoding->symmetric  = false;
    addNodeInternal(server, (UA_Node*)hasencoding, nodeIdNonHierarchicalReferences, nodeIdHasSubType);

    UA_ReferenceTypeNode *hasdescription = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)hasdescription, "HasDescription");
    hasdescription->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "DescriptionOf");
    hasdescription->nodeId.identifier.numeric = UA_NS0ID_HASDESCRIPTION;
    hasdescription->isAbstract = false;
    hasdescription->symmetric  = false;
    addNodeInternal(server, (UA_Node*)hasdescription, nodeIdNonHierarchicalReferences, nodeIdHasSubType);

    UA_ReferenceTypeNode *hastypedefinition = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)hastypedefinition, "HasTypeDefinition");
    hastypedefinition->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "TypeDefinitionOf");
    hastypedefinition->nodeId.identifier.numeric = UA_NS0ID_HASTYPEDEFINITION;
    hastypedefinition->isAbstract = false;
    hastypedefinition->symmetric  = false;
    addNodeInternal(server, (UA_Node*)hastypedefinition, nodeIdNonHierarchicalReferences, nodeIdHasSubType);

    UA_ReferenceTypeNode *generatesevent = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)generatesevent, "GeneratesEvent");
    generatesevent->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "GeneratedBy");
    generatesevent->nodeId.identifier.numeric = UA_NS0ID_GENERATESEVENT;
    generatesevent->isAbstract = false;
    generatesevent->symmetric  = false;
    addNodeInternal(server, (UA_Node*)generatesevent, nodeIdNonHierarchicalReferences,
                    nodeIdHasSubType);

    UA_ReferenceTypeNode *aggregates = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)aggregates, "Aggregates");
    // Todo: Is there an inverse name?
    aggregates->nodeId.identifier.numeric = UA_NS0ID_AGGREGATES;
    aggregates->isAbstract = true;
    aggregates->symmetric  = false;
    addNodeInternal(server, (UA_Node*)aggregates,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_HASCHILD), nodeIdHasSubType);

    // complete bootstrap of hassubtype
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCHILD), nodeIdHasSubType,
                         UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE), true);

    UA_ReferenceTypeNode *hasproperty = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)hasproperty, "HasProperty");
    hasproperty->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "PropertyOf");
    hasproperty->nodeId.identifier.numeric = UA_NS0ID_HASPROPERTY;
    hasproperty->isAbstract = false;
    hasproperty->symmetric  = false;
    addNodeInternal(server, (UA_Node*)hasproperty,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_AGGREGATES), nodeIdHasSubType);

    UA_ReferenceTypeNode *hascomponent = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)hascomponent, "HasComponent");
    hascomponent->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "ComponentOf");
    hascomponent->nodeId.identifier.numeric = UA_NS0ID_HASCOMPONENT;
    hascomponent->isAbstract = false;
    hascomponent->symmetric  = false;
    addNodeInternal(server, (UA_Node*)hascomponent,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_AGGREGATES), nodeIdHasSubType);

    UA_ReferenceTypeNode *hasnotifier = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)hasnotifier, "HasNotifier");
    hasnotifier->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "NotifierOf");
    hasnotifier->nodeId.identifier.numeric = UA_NS0ID_HASNOTIFIER;
    hasnotifier->isAbstract = false;
    hasnotifier->symmetric  = false;
    addNodeInternal(server, (UA_Node*)hasnotifier, UA_NODEID_NUMERIC(0, UA_NS0ID_HASEVENTSOURCE),
                    nodeIdHasSubType);

    UA_ReferenceTypeNode *hasorderedcomponent = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)hasorderedcomponent, "HasOrderedComponent");
    hasorderedcomponent->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "OrderedComponentOf");
    hasorderedcomponent->nodeId.identifier.numeric = UA_NS0ID_HASORDEREDCOMPONENT;
    hasorderedcomponent->isAbstract = false;
    hasorderedcomponent->symmetric  = false;
    addNodeInternal(server, (UA_Node*)hasorderedcomponent, UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
                    nodeIdHasSubType);

    UA_ReferenceTypeNode *hasmodelparent = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)hasmodelparent, "HasModelParent");
    hasmodelparent->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "ModelParentOf");
    hasmodelparent->nodeId.identifier.numeric = UA_NS0ID_HASMODELPARENT;
    hasmodelparent->isAbstract = false;
    hasmodelparent->symmetric  = false;
    addNodeInternal(server, (UA_Node*)hasmodelparent, nodeIdNonHierarchicalReferences, nodeIdHasSubType);

    UA_ReferenceTypeNode *fromstate = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)fromstate, "FromState");
    fromstate->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "ToTransition");
    fromstate->nodeId.identifier.numeric = UA_NS0ID_FROMSTATE;
    fromstate->isAbstract = false;
    fromstate->symmetric  = false;
    addNodeInternal(server, (UA_Node*)fromstate, nodeIdNonHierarchicalReferences, nodeIdHasSubType);

    UA_ReferenceTypeNode *tostate = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)tostate, "ToState");
    tostate->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "FromTransition");
    tostate->nodeId.identifier.numeric = UA_NS0ID_TOSTATE;
    tostate->isAbstract = false;
    tostate->symmetric  = false;
    addNodeInternal(server, (UA_Node*)tostate, nodeIdNonHierarchicalReferences, nodeIdHasSubType);

    UA_ReferenceTypeNode *hascause = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)hascause, "HasCause");
    hascause->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "MayBeCausedBy");
    hascause->nodeId.identifier.numeric = UA_NS0ID_HASCAUSE;
    hascause->isAbstract = false;
    hascause->symmetric  = false;
    addNodeInternal(server, (UA_Node*)hascause, nodeIdNonHierarchicalReferences, nodeIdHasSubType);
    
    UA_ReferenceTypeNode *haseffect = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)haseffect, "HasEffect");
    haseffect->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "MayBeEffectedBy");
    haseffect->nodeId.identifier.numeric = UA_NS0ID_HASEFFECT;
    haseffect->isAbstract = false;
    haseffect->symmetric  = false;
    addNodeInternal(server, (UA_Node*)haseffect, nodeIdNonHierarchicalReferences, nodeIdHasSubType);

    UA_ReferenceTypeNode *hashistoricalconfiguration = UA_NodeStore_newReferenceTypeNode();
    copyNames((UA_Node*)hashistoricalconfiguration, "HasHistoricalConfiguration");
    hashistoricalconfiguration->inverseName = UA_LOCALIZEDTEXT_ALLOC("en_US", "HistoricalConfigurationOf");
    hashistoricalconfiguration->nodeId.identifier.numeric = UA_NS0ID_HASHISTORICALCONFIGURATION;
    hashistoricalconfiguration->isAbstract = false;
    hashistoricalconfiguration->symmetric  = false;
    addNodeInternal(server, (UA_Node*)hashistoricalconfiguration,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_AGGREGATES), nodeIdHasSubType);

    /*****************/
    /* Basic Folders */
    /*****************/

    UA_ObjectNode *root = UA_NodeStore_newObjectNode();
    copyNames((UA_Node*)root, "Root");
    root->nodeId.identifier.numeric = UA_NS0ID_ROOTFOLDER;
    UA_RCU_LOCK();
    UA_NodeStore_insert(server->nodestore, (UA_Node*)root);
    UA_RCU_UNLOCK();

    UA_ObjectNode *objects = UA_NodeStore_newObjectNode();
    copyNames((UA_Node*)objects, "Objects");
    objects->nodeId.identifier.numeric = UA_NS0ID_OBJECTSFOLDER;
    addNodeInternal(server, (UA_Node*)objects, UA_NODEID_NUMERIC(0, UA_NS0ID_ROOTFOLDER),
                    nodeIdOrganizes);

    UA_ObjectNode *types = UA_NodeStore_newObjectNode();
    copyNames((UA_Node*)types, "Types");
    types->nodeId.identifier.numeric = UA_NS0ID_TYPESFOLDER;
    addNodeInternal(server, (UA_Node*)types, UA_NODEID_NUMERIC(0, UA_NS0ID_ROOTFOLDER),
                    nodeIdOrganizes);

    UA_ObjectNode *views = UA_NodeStore_newObjectNode();
    copyNames((UA_Node*)views, "Views");
    views->nodeId.identifier.numeric = UA_NS0ID_VIEWSFOLDER;
    addNodeInternal(server, (UA_Node*)views, UA_NODEID_NUMERIC(0, UA_NS0ID_ROOTFOLDER),
                    nodeIdOrganizes);

    UA_ObjectNode *referencetypes = UA_NodeStore_newObjectNode();
    copyNames((UA_Node*)referencetypes, "ReferenceTypes");
    referencetypes->nodeId.identifier.numeric = UA_NS0ID_REFERENCETYPESFOLDER;
    addNodeInternal(server, (UA_Node*)referencetypes, UA_NODEID_NUMERIC(0, UA_NS0ID_TYPESFOLDER),
                    nodeIdOrganizes);

    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_REFERENCETYPESFOLDER), nodeIdOrganizes,
                         UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_REFERENCES), true);

    /**********************/
    /* Basic Object Types */
    /**********************/

    UA_ObjectNode *objecttypes = UA_NodeStore_newObjectNode();
    copyNames((UA_Node*)objecttypes, "ObjectTypes");
    objecttypes->nodeId.identifier.numeric = UA_NS0ID_OBJECTTYPESFOLDER;
    addNodeInternal(server, (UA_Node*)objecttypes, UA_NODEID_NUMERIC(0, UA_NS0ID_TYPESFOLDER),
                    nodeIdOrganizes);

    addObjectTypeNode(server, "BaseObjectType", UA_NS0ID_BASEOBJECTTYPE, UA_NS0ID_OBJECTTYPESFOLDER,
                      UA_NS0ID_ORGANIZES);
    addObjectTypeNode(server, "FolderType", UA_NS0ID_FOLDERTYPE, UA_NS0ID_BASEOBJECTTYPE, UA_NS0ID_HASSUBTYPE);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTTYPESFOLDER), nodeIdHasTypeDefinition,
                         UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), true);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_ROOTFOLDER), nodeIdHasTypeDefinition,
                         UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), true);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER), nodeIdHasTypeDefinition,
                         UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), true);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_TYPESFOLDER), nodeIdHasTypeDefinition,
                         UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), true);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_VIEWSFOLDER), nodeIdHasTypeDefinition,
                         UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), true);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_REFERENCETYPESFOLDER),
                         nodeIdHasTypeDefinition, UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), true);
    addObjectTypeNode(server, "ServerType", UA_NS0ID_SERVERTYPE, UA_NS0ID_BASEOBJECTTYPE, UA_NS0ID_HASSUBTYPE);
    addObjectTypeNode(server, "ServerDiagnosticsType", UA_NS0ID_SERVERDIAGNOSTICSTYPE,
                      UA_NS0ID_BASEOBJECTTYPE, UA_NS0ID_HASSUBTYPE);
    addObjectTypeNode(server, "ServerCapatilitiesType", UA_NS0ID_SERVERCAPABILITIESTYPE,
                      UA_NS0ID_BASEOBJECTTYPE, UA_NS0ID_HASSUBTYPE);
    addObjectTypeNode(server, "ServerStatusType", UA_NS0ID_SERVERSTATUSTYPE, UA_NS0ID_BASEOBJECTTYPE,
                      UA_NS0ID_HASSUBTYPE);
    addObjectTypeNode(server, "BuildInfoType", UA_NS0ID_BUILDINFOTYPE, UA_NS0ID_BASEOBJECTTYPE,
                      UA_NS0ID_HASSUBTYPE);

    /**************/
    /* Data Types */
    /**************/

    UA_ObjectNode *datatypes = UA_NodeStore_newObjectNode();
    copyNames((UA_Node*)datatypes, "DataTypes");
    datatypes->nodeId.identifier.numeric = UA_NS0ID_DATATYPESFOLDER;
    addNodeInternal(server, (UA_Node*)datatypes, UA_NODEID_NUMERIC(0, UA_NS0ID_TYPESFOLDER), nodeIdOrganizes);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_DATATYPESFOLDER), nodeIdHasTypeDefinition,
                         UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), true);

    addDataTypeNode(server, "BaseDataType", UA_NS0ID_BASEDATATYPE, UA_NS0ID_DATATYPESFOLDER);
    addDataTypeNode(server, "Boolean", UA_NS0ID_BOOLEAN, UA_NS0ID_BASEDATATYPE);
    addDataTypeNode(server, "Number", UA_NS0ID_NUMBER, UA_NS0ID_BASEDATATYPE);
        addDataTypeNode(server, "Float", UA_NS0ID_FLOAT, UA_NS0ID_NUMBER);
        addDataTypeNode(server, "Double", UA_NS0ID_DOUBLE, UA_NS0ID_NUMBER);
        addDataTypeNode(server, "Integer", UA_NS0ID_INTEGER, UA_NS0ID_NUMBER);
            addDataTypeNode(server, "SByte", UA_NS0ID_SBYTE, UA_NS0ID_INTEGER);
            addDataTypeNode(server, "Int16", UA_NS0ID_INT16, UA_NS0ID_INTEGER);
            addDataTypeNode(server, "Int32", UA_NS0ID_INT32, UA_NS0ID_INTEGER);
            addDataTypeNode(server, "Int64", UA_NS0ID_INT64, UA_NS0ID_INTEGER);
            addDataTypeNode(server, "UInteger", UA_NS0ID_UINTEGER, UA_NS0ID_INTEGER);
                addDataTypeNode(server, "Byte", UA_NS0ID_BYTE, UA_NS0ID_UINTEGER);
                addDataTypeNode(server, "UInt16", UA_NS0ID_UINT16, UA_NS0ID_UINTEGER);
                addDataTypeNode(server, "UInt32", UA_NS0ID_UINT32, UA_NS0ID_UINTEGER);
                addDataTypeNode(server, "UInt64", UA_NS0ID_UINT64, UA_NS0ID_UINTEGER);
    addDataTypeNode(server, "String", UA_NS0ID_STRING, UA_NS0ID_BASEDATATYPE);
    addDataTypeNode(server, "DateTime", UA_NS0ID_DATETIME, UA_NS0ID_BASEDATATYPE);
    addDataTypeNode(server, "Guid", UA_NS0ID_GUID, UA_NS0ID_BASEDATATYPE);
    addDataTypeNode(server, "ByteString", UA_NS0ID_BYTESTRING, UA_NS0ID_BASEDATATYPE);
    addDataTypeNode(server, "XmlElement", UA_NS0ID_XMLELEMENT, UA_NS0ID_BASEDATATYPE);
    addDataTypeNode(server, "NodeId", UA_NS0ID_NODEID, UA_NS0ID_BASEDATATYPE);
    addDataTypeNode(server, "ExpandedNodeId", UA_NS0ID_EXPANDEDNODEID, UA_NS0ID_BASEDATATYPE);
    addDataTypeNode(server, "StatusCode", UA_NS0ID_STATUSCODE, UA_NS0ID_BASEDATATYPE);
    addDataTypeNode(server, "QualifiedName", UA_NS0ID_QUALIFIEDNAME, UA_NS0ID_BASEDATATYPE);
    addDataTypeNode(server, "LocalizedText", UA_NS0ID_LOCALIZEDTEXT, UA_NS0ID_BASEDATATYPE);
    addDataTypeNode(server, "Structure", UA_NS0ID_STRUCTURE, UA_NS0ID_BASEDATATYPE);
        addDataTypeNode(server, "ServerStatusDataType", UA_NS0ID_SERVERSTATUSDATATYPE, UA_NS0ID_STRUCTURE);
        addDataTypeNode(server, "BuildInfo", UA_NS0ID_BUILDINFO, UA_NS0ID_STRUCTURE);
    addDataTypeNode(server, "DataValue", UA_NS0ID_DATAVALUE, UA_NS0ID_BASEDATATYPE);
    addDataTypeNode(server, "DiagnosticInfo", UA_NS0ID_DIAGNOSTICINFO, UA_NS0ID_BASEDATATYPE);
    addDataTypeNode(server, "Enumeration", UA_NS0ID_ENUMERATION, UA_NS0ID_BASEDATATYPE);
        addDataTypeNode(server, "ServerState", UA_NS0ID_SERVERSTATE, UA_NS0ID_ENUMERATION);

    UA_ObjectNode *variabletypes = UA_NodeStore_newObjectNode();
    copyNames((UA_Node*)variabletypes, "VariableTypes");
    variabletypes->nodeId.identifier.numeric = UA_NS0ID_VARIABLETYPESFOLDER;
    addNodeInternal(server, (UA_Node*)variabletypes, UA_NODEID_NUMERIC(0, UA_NS0ID_TYPESFOLDER),
                    nodeIdOrganizes);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_VARIABLETYPESFOLDER),
                         nodeIdHasTypeDefinition, UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE), true);
    addVariableTypeNode_organized(server, "BaseVariableType", UA_NS0ID_BASEVARIABLETYPE,
                                  UA_NS0ID_VARIABLETYPESFOLDER, true);
    addVariableTypeNode_subtype(server, "BaseDataVariableType", UA_NS0ID_BASEDATAVARIABLETYPE,
                                UA_NS0ID_BASEVARIABLETYPE, false);
    addVariableTypeNode_subtype(server, "PropertyType", UA_NS0ID_PROPERTYTYPE,
                                UA_NS0ID_BASEVARIABLETYPE, false);
#endif

#ifdef UA_ENABLE_GENERATE_NAMESPACE0
    //load the generated namespace
    ua_namespaceinit_generated(server);
#endif

    /*********************/
    /* The Server Object */
    /*********************/

    UA_ObjectNode *servernode = UA_NodeStore_newObjectNode();
    copyNames((UA_Node*)servernode, "Server");
    servernode->nodeId.identifier.numeric = UA_NS0ID_SERVER;
    addNodeInternal(server, (UA_Node*)servernode, UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
                    nodeIdOrganizes);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), nodeIdHasTypeDefinition,
                         UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_SERVERTYPE), true);

    UA_VariableNode *namespaceArray = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)namespaceArray, "NamespaceArray");
    namespaceArray->nodeId.identifier.numeric = UA_NS0ID_SERVER_NAMESPACEARRAY;
    namespaceArray->valueSource = UA_VALUESOURCE_DATASOURCE;
    namespaceArray->value.dataSource = (UA_DataSource) {.handle = server, .read = readNamespaces,
                                                        .write = NULL};
    namespaceArray->valueRank = 1;
    namespaceArray->minimumSamplingInterval = 1.0;
    addNodeInternal(server, (UA_Node*)namespaceArray, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), nodeIdHasProperty);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_NAMESPACEARRAY),
                         nodeIdHasTypeDefinition, UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), true);

    UA_VariableNode *serverArray = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)serverArray, "ServerArray");
    serverArray->nodeId.identifier.numeric = UA_NS0ID_SERVER_SERVERARRAY;
    UA_Variant_setArrayCopy(&serverArray->value.variant.value,
                            &server->config.applicationDescription.applicationUri, 1,
                            &UA_TYPES[UA_TYPES_STRING]);
    serverArray->valueRank = 1;
    serverArray->minimumSamplingInterval = 1.0;
    addNodeInternal(server, (UA_Node*)serverArray, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), nodeIdHasProperty);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERARRAY), nodeIdHasTypeDefinition,
                         UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), true);

    UA_ObjectNode *servercapablities = UA_NodeStore_newObjectNode();
    copyNames((UA_Node*)servercapablities, "ServerCapabilities");
    servercapablities->nodeId.identifier.numeric = UA_NS0ID_SERVER_SERVERCAPABILITIES;
    addNodeInternal(server, (UA_Node*)servercapablities, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER),
                    nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERCAPABILITIES), nodeIdHasTypeDefinition,
                         UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_SERVERCAPABILITIESTYPE), true);

    UA_VariableNode *localeIdArray = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)localeIdArray, "LocaleIdArray");
    localeIdArray->nodeId.identifier.numeric = UA_NS0ID_SERVER_SERVERCAPABILITIES_LOCALEIDARRAY;
    localeIdArray->value.variant.value.data = UA_Array_new(1, &UA_TYPES[UA_TYPES_STRING]);
    localeIdArray->value.variant.value.arrayLength = 1;
    localeIdArray->value.variant.value.type = &UA_TYPES[UA_TYPES_STRING];
    *(UA_String *)localeIdArray->value.variant.value.data = UA_STRING_ALLOC("en");
    localeIdArray->valueRank = 1;
    localeIdArray->minimumSamplingInterval = 1.0;
    addNodeInternal(server, (UA_Node*)localeIdArray,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERCAPABILITIES), nodeIdHasProperty);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_LOCALEIDARRAY),
                         nodeIdHasTypeDefinition, UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), true);

    UA_VariableNode *maxBrowseContinuationPoints = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)maxBrowseContinuationPoints, "MaxBrowseContinuationPoints");
    maxBrowseContinuationPoints->nodeId.identifier.numeric =
        UA_NS0ID_SERVER_SERVERCAPABILITIES_MAXBROWSECONTINUATIONPOINTS;
    maxBrowseContinuationPoints->value.variant.value.data = UA_UInt16_new();
    *((UA_UInt16*)maxBrowseContinuationPoints->value.variant.value.data) = MAXCONTINUATIONPOINTS;
    maxBrowseContinuationPoints->value.variant.value.type = &UA_TYPES[UA_TYPES_UINT16];
    addNodeInternal(server, (UA_Node*)maxBrowseContinuationPoints,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERCAPABILITIES), nodeIdHasProperty);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_MAXBROWSECONTINUATIONPOINTS),
                         nodeIdHasTypeDefinition, UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), true);

    /** ServerProfileArray **/
#define MAX_PROFILEARRAY 16 //a *magic* limit to the number of supported profiles
#define ADDPROFILEARRAY(x) profileArray[profileArraySize++] = UA_STRING_ALLOC(x)
    UA_String profileArray[MAX_PROFILEARRAY];
    UA_UInt16 profileArraySize = 0;
    ADDPROFILEARRAY("http://opcfoundation.org/UA-Profile/Server/NanoEmbeddedDevice");

#ifdef UA_ENABLE_SERVICESET_NODEMANAGEMENT
    ADDPROFILEARRAY("http://opcfoundation.org/UA-Profile/Server/NodeManagement");
#endif
#ifdef UA_ENABLE_SERVICESET_METHOD
    ADDPROFILEARRAY("http://opcfoundation.org/UA-Profile/Server/Methods");
#endif
#ifdef UA_ENABLE_SUBSCRIPTIONS
    ADDPROFILEARRAY("http://opcfoundation.org/UA-Profile/Server/EmbeddedDataChangeSubscription");
#endif

    UA_VariableNode *serverProfileArray = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)serverProfileArray, "ServerProfileArray");
    serverProfileArray->nodeId.identifier.numeric = UA_NS0ID_SERVER_SERVERCAPABILITIES_SERVERPROFILEARRAY;
    serverProfileArray->value.variant.value.arrayLength = profileArraySize;
    serverProfileArray->value.variant.value.data = UA_Array_new(profileArraySize, &UA_TYPES[UA_TYPES_STRING]);
    serverProfileArray->value.variant.value.type = &UA_TYPES[UA_TYPES_STRING];
    for(UA_UInt16 i=0;i<profileArraySize;i++)
        ((UA_String *)serverProfileArray->value.variant.value.data)[i] = profileArray[i];
    serverProfileArray->valueRank = 1;
    serverProfileArray->minimumSamplingInterval = 1.0;
    addNodeInternal(server, (UA_Node*)serverProfileArray,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERCAPABILITIES), nodeIdHasProperty);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERCAPABILITIES_SERVERPROFILEARRAY),
                         nodeIdHasTypeDefinition, UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), true);

    UA_ObjectNode *serverdiagnostics = UA_NodeStore_newObjectNode();
    copyNames((UA_Node*)serverdiagnostics, "ServerDiagnostics");
    serverdiagnostics->nodeId.identifier.numeric = UA_NS0ID_SERVER_SERVERDIAGNOSTICS;
    addNodeInternal(server, (UA_Node*)serverdiagnostics,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERDIAGNOSTICS),
                         nodeIdHasTypeDefinition, UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_SERVERDIAGNOSTICSTYPE), true);

    UA_VariableNode *enabledFlag = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)enabledFlag, "EnabledFlag");
    enabledFlag->nodeId.identifier.numeric = UA_NS0ID_SERVER_SERVERDIAGNOSTICS_ENABLEDFLAG;
    enabledFlag->value.variant.value.data = UA_Boolean_new(); //initialized as false
    enabledFlag->value.variant.value.type = &UA_TYPES[UA_TYPES_BOOLEAN];
    enabledFlag->valueRank = 1;
    enabledFlag->minimumSamplingInterval = 1.0;
    addNodeInternal(server, (UA_Node*)enabledFlag,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERDIAGNOSTICS), nodeIdHasProperty);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERDIAGNOSTICS_ENABLEDFLAG),
                         nodeIdHasTypeDefinition, UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_PROPERTYTYPE), true);

    UA_VariableNode *serverstatus = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)serverstatus, "ServerStatus");
    serverstatus->nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS);
    serverstatus->valueSource = UA_VALUESOURCE_DATASOURCE;
    serverstatus->value.dataSource = (UA_DataSource) {.handle = server, .read = readStatus, .write = NULL};
    addNodeInternal(server, (UA_Node*)serverstatus, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS), nodeIdHasTypeDefinition,
                         UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_SERVERSTATUSTYPE), true);

    UA_VariableNode *starttime = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)starttime, "StartTime");
    starttime->nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_STARTTIME);
    starttime->value.variant.value.storageType = UA_VARIANT_DATA_NODELETE;
    starttime->value.variant.value.data = &server->startTime;
    starttime->value.variant.value.type = &UA_TYPES[UA_TYPES_DATETIME];
    addNodeInternal(server, (UA_Node*)starttime, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS),
                    nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_STARTTIME),
                         nodeIdHasTypeDefinition, expandedNodeIdBaseDataVariabletype, true);

    UA_VariableNode *currenttime = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)currenttime, "CurrentTime");
    currenttime->nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME);
    currenttime->valueSource = UA_VALUESOURCE_DATASOURCE;
    currenttime->value.dataSource = (UA_DataSource) {.handle = NULL, .read = readCurrentTime,
                                                     .write = NULL};
    addNodeInternal(server, (UA_Node*)currenttime,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS), nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_CURRENTTIME),
                         nodeIdHasTypeDefinition, expandedNodeIdBaseDataVariabletype, true);

    UA_VariableNode *state = UA_NodeStore_newVariableNode();
    UA_ServerState *stateEnum = UA_ServerState_new();
    *stateEnum = UA_SERVERSTATE_RUNNING;
    copyNames((UA_Node*)state, "State");
    state->nodeId.identifier.numeric = UA_NS0ID_SERVER_SERVERSTATUS_STATE;
    state->value.variant.value.type = &UA_TYPES[UA_TYPES_SERVERSTATE];
    state->value.variant.value.arrayLength = 0;
    state->value.variant.value.data = stateEnum; // points into the other object.
    addNodeInternal(server, (UA_Node*)state, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS),
                    nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_STATE),
                         nodeIdHasTypeDefinition, expandedNodeIdBaseDataVariabletype, true);

    UA_VariableNode *buildinfo = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)buildinfo, "BuildInfo");
    buildinfo->nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO);
    UA_Variant_setScalarCopy(&buildinfo->value.variant.value,
                             &server->config.buildInfo,
                             &UA_TYPES[UA_TYPES_BUILDINFO]);
    addNodeInternal(server, (UA_Node*)buildinfo,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS), nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO),
                         nodeIdHasTypeDefinition, UA_EXPANDEDNODEID_NUMERIC(0, UA_NS0ID_BUILDINFOTYPE), true);

    UA_VariableNode *producturi = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)producturi, "ProductUri");
    producturi->nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_PRODUCTURI);
    UA_Variant_setScalarCopy(&producturi->value.variant.value, &server->config.buildInfo.productUri,
                             &UA_TYPES[UA_TYPES_STRING]);
    producturi->value.variant.value.type = &UA_TYPES[UA_TYPES_STRING];
    addNodeInternal(server, (UA_Node*)producturi,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO), nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_PRODUCTURI),
                         nodeIdHasTypeDefinition, expandedNodeIdBaseDataVariabletype, true);

    UA_VariableNode *manufacturername = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)manufacturername, "ManufacturerName");
    manufacturername->nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_MANUFACTURERNAME);
    UA_Variant_setScalarCopy(&manufacturername->value.variant.value,
                             &server->config.buildInfo.manufacturerName,
                             &UA_TYPES[UA_TYPES_STRING]);
    addNodeInternal(server, (UA_Node*)manufacturername,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO), nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_MANUFACTURERNAME),
                         nodeIdHasTypeDefinition, expandedNodeIdBaseDataVariabletype, true);

    UA_VariableNode *productname = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)productname, "ProductName");
    productname->nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_PRODUCTNAME);
    UA_Variant_setScalarCopy(&productname->value.variant.value, &server->config.buildInfo.productName,
                             &UA_TYPES[UA_TYPES_STRING]);
    addNodeInternal(server, (UA_Node*)productname,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO), nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_PRODUCTNAME),
                         nodeIdHasTypeDefinition, expandedNodeIdBaseDataVariabletype, true);

    UA_VariableNode *softwareversion = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)softwareversion, "SoftwareVersion");
    softwareversion->nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_SOFTWAREVERSION);
    UA_Variant_setScalarCopy(&softwareversion->value.variant.value, &server->config.buildInfo.softwareVersion,
                             &UA_TYPES[UA_TYPES_STRING]);
    addNodeInternal(server, (UA_Node*)softwareversion,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO), nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_SOFTWAREVERSION),
                         nodeIdHasTypeDefinition, expandedNodeIdBaseDataVariabletype, true);

    UA_VariableNode *buildnumber = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)buildnumber, "BuildNumber");
    buildnumber->nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_BUILDNUMBER);
    UA_Variant_setScalarCopy(&buildnumber->value.variant.value, &server->config.buildInfo.buildNumber,
                             &UA_TYPES[UA_TYPES_STRING]);
    addNodeInternal(server, (UA_Node*)buildnumber,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO), nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_BUILDNUMBER),
                         nodeIdHasTypeDefinition, expandedNodeIdBaseDataVariabletype, true);

    UA_VariableNode *builddate = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)builddate, "BuildDate");
    builddate->nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_BUILDDATE);
    UA_Variant_setScalarCopy(&builddate->value.variant.value, &server->config.buildInfo.buildDate,
                             &UA_TYPES[UA_TYPES_DATETIME]);
    addNodeInternal(server, (UA_Node*)builddate,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO), nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_BUILDINFO_BUILDNUMBER),
                         nodeIdHasTypeDefinition, expandedNodeIdBaseDataVariabletype, true);

    UA_VariableNode *secondstillshutdown = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)secondstillshutdown, "SecondsTillShutdown");
    secondstillshutdown->nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_SECONDSTILLSHUTDOWN);
    secondstillshutdown->value.variant.value.data = UA_UInt32_new();
    secondstillshutdown->value.variant.value.type = &UA_TYPES[UA_TYPES_UINT32];
    addNodeInternal(server, (UA_Node*)secondstillshutdown,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS), nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_SECONDSTILLSHUTDOWN),
                         nodeIdHasTypeDefinition, expandedNodeIdBaseDataVariabletype, true);

    UA_VariableNode *shutdownreason = UA_NodeStore_newVariableNode();
    copyNames((UA_Node*)shutdownreason, "ShutdownReason");
    shutdownreason->nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_SHUTDOWNREASON);
    shutdownreason->value.variant.value.data = UA_LocalizedText_new();
    shutdownreason->value.variant.value.type = &UA_TYPES[UA_TYPES_LOCALIZEDTEXT];
    addNodeInternal(server, (UA_Node*)shutdownreason,
                    UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS), nodeIdHasComponent);
    addReferenceInternal(server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_SERVERSTATUS_SHUTDOWNREASON),
                         nodeIdHasTypeDefinition, expandedNodeIdBaseDataVariabletype, true);
    return server;
}

UA_StatusCode
__UA_Server_write(UA_Server *server, const UA_NodeId *nodeId,
                  const UA_AttributeId attributeId, const UA_DataType *attr_type,
                  const void *value) {
    UA_WriteValue wvalue;
    UA_WriteValue_init(&wvalue);
    wvalue.nodeId = *nodeId;
    wvalue.attributeId = attributeId;
    if(attributeId != UA_ATTRIBUTEID_VALUE)
        /* hacked cast. the target WriteValue is used as const anyway */
        UA_Variant_setScalar(&wvalue.value.value, (void*)(uintptr_t)value, attr_type);
    else {
        if(attr_type != &UA_TYPES[UA_TYPES_VARIANT])
            return UA_STATUSCODE_BADTYPEMISMATCH;
        wvalue.value.value = *(const UA_Variant*)value;
    }
    wvalue.value.hasValue = true;
    UA_RCU_LOCK();
    UA_StatusCode retval = Service_Write_single(server, &adminSession, &wvalue);
    UA_RCU_UNLOCK();
    return retval;
}

static UA_StatusCode
setValueCallback(UA_Server *server, UA_Session *session, UA_VariableNode *node, UA_ValueCallback *callback) {
    if(node->nodeClass != UA_NODECLASS_VARIABLE)
        return UA_STATUSCODE_BADNODECLASSINVALID;
    node->value.variant.callback = *callback;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode UA_EXPORT
UA_Server_setVariableNode_valueCallback(UA_Server *server, const UA_NodeId nodeId,
                                        const UA_ValueCallback callback) {
    UA_RCU_LOCK();
    UA_StatusCode retval = UA_Server_editNode(server, &adminSession, &nodeId,
                                              (UA_EditNodeCallback)setValueCallback, &callback);
    UA_RCU_UNLOCK();
    return retval;
}

static UA_StatusCode
setDataSource(UA_Server *server, UA_Session *session,
              UA_VariableNode* node, UA_DataSource *dataSource) {
    if(node->nodeClass != UA_NODECLASS_VARIABLE)
        return UA_STATUSCODE_BADNODECLASSINVALID;
    if(node->valueSource == UA_VALUESOURCE_VARIANT)
        UA_Variant_deleteMembers(&node->value.variant.value);
    node->value.dataSource = *dataSource;
    node->valueSource = UA_VALUESOURCE_DATASOURCE;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
UA_Server_setVariableNode_dataSource(UA_Server *server, const UA_NodeId nodeId,
                                     const UA_DataSource dataSource) {
    UA_RCU_LOCK();
    UA_StatusCode retval = UA_Server_editNode(server, &adminSession, &nodeId,
                                              (UA_EditNodeCallback)setDataSource, &dataSource);
    UA_RCU_UNLOCK();
    return retval;
}

static UA_StatusCode
setObjectTypeLifecycleManagement(UA_Server *server, UA_Session *session, UA_ObjectTypeNode* node,
                                 UA_ObjectLifecycleManagement *olm) {
    if(node->nodeClass != UA_NODECLASS_OBJECTTYPE)
        return UA_STATUSCODE_BADNODECLASSINVALID;
    node->lifecycleManagement = *olm;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode UA_EXPORT
UA_Server_setObjectTypeNode_lifecycleManagement(UA_Server *server, UA_NodeId nodeId,
                                                UA_ObjectLifecycleManagement olm) {
    UA_RCU_LOCK();
    UA_StatusCode retval = UA_Server_editNode(server, &adminSession, &nodeId,
                                              (UA_EditNodeCallback)setObjectTypeLifecycleManagement, &olm);
    UA_RCU_UNLOCK();
    return retval;
}

#ifdef UA_ENABLE_METHODCALLS

struct addMethodCallback {
    UA_MethodCallback callback;
    void *handle;
};

static UA_StatusCode
editMethodCallback(UA_Server *server, UA_Session* session, UA_Node* node, const void* handle) {
    if(node->nodeClass != UA_NODECLASS_METHOD)
        return UA_STATUSCODE_BADNODECLASSINVALID;
    const struct addMethodCallback *newCallback = handle;
    UA_MethodNode *mnode = (UA_MethodNode*) node;
    mnode->attachedMethod = newCallback->callback;
    mnode->methodHandle   = newCallback->handle;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode UA_EXPORT
UA_Server_setMethodNode_callback(UA_Server *server, const UA_NodeId methodNodeId,
                                 UA_MethodCallback method, void *handle) {
    struct addMethodCallback cb = { method, handle };
    UA_RCU_LOCK();
    UA_StatusCode retval = UA_Server_editNode(server, &adminSession, &methodNodeId, editMethodCallback, &cb);
    UA_RCU_UNLOCK();
    return retval;
}

#endif

UA_StatusCode
__UA_Server_read(UA_Server *server, const UA_NodeId *nodeId, const UA_AttributeId attributeId, void *v) {
    UA_ReadValueId item;
    UA_ReadValueId_init(&item);
    item.nodeId = *nodeId;
    item.attributeId = attributeId;
    UA_DataValue dv;
    UA_DataValue_init(&dv);
    UA_RCU_LOCK();
    Service_Read_single(server, &adminSession, UA_TIMESTAMPSTORETURN_NEITHER,
                        &item, &dv);
    UA_RCU_UNLOCK();
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    if(dv.hasStatus)
        retval = dv.hasStatus;
    else if(!dv.hasValue)
        retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_DataValue_deleteMembers(&dv);
        return retval;
    }
    if(attributeId == UA_ATTRIBUTEID_VALUE ||
       attributeId == UA_ATTRIBUTEID_ARRAYDIMENSIONS)
        memcpy(v, &dv.value, sizeof(UA_Variant));
    else {
        memcpy(v, dv.value.data, dv.value.type->memSize);
        dv.value.data = NULL;
        dv.value.arrayLength = 0;
        UA_Variant_deleteMembers(&dv.value);
    }
    return UA_STATUSCODE_GOOD;
}

UA_BrowseResult
UA_Server_browse(UA_Server *server, UA_UInt32 maxrefs, const UA_BrowseDescription *descr) {
    UA_BrowseResult result;
    UA_BrowseResult_init(&result);
    UA_RCU_LOCK();
    Service_Browse_single(server, &adminSession, NULL, descr, maxrefs, &result);
    UA_RCU_UNLOCK();
    return result;
}

UA_BrowseResult
UA_Server_browseNext(UA_Server *server, UA_Boolean releaseContinuationPoint,
                     const UA_ByteString *continuationPoint) {
    UA_BrowseResult result;
    UA_BrowseResult_init(&result);
    UA_RCU_LOCK();
    UA_Server_browseNext_single(server, &adminSession, releaseContinuationPoint,
                                continuationPoint, &result);
    UA_RCU_UNLOCK();
    return result;
}

#ifdef UA_ENABLE_METHODCALLS
UA_CallMethodResult UA_Server_call(UA_Server *server, const UA_CallMethodRequest *request) {
    UA_CallMethodResult result;
    UA_CallMethodResult_init(&result);
    UA_RCU_LOCK();
    Service_Call_single(server, &adminSession, request, &result);
    UA_RCU_UNLOCK();
    return result;
}
#endif

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_server_binary.c" ***********************************/


/** Max size of messages that are allocated on the stack */
#define MAX_STACK_MESSAGE 65536

static void processHEL(UA_Connection *connection, const UA_ByteString *msg, size_t *pos) {
    UA_TcpHelloMessage helloMessage;
    if(UA_TcpHelloMessage_decodeBinary(msg, pos, &helloMessage) != UA_STATUSCODE_GOOD) {
        connection->close(connection);
        return;
    }

    connection->remoteConf.maxChunkCount = helloMessage.maxChunkCount;
    connection->remoteConf.maxMessageSize = helloMessage.maxMessageSize;
    connection->remoteConf.protocolVersion = helloMessage.protocolVersion;
    connection->remoteConf.recvBufferSize = helloMessage.receiveBufferSize;
    if(connection->localConf.sendBufferSize > helloMessage.receiveBufferSize)
        connection->localConf.sendBufferSize = helloMessage.receiveBufferSize;
    if(connection->localConf.recvBufferSize > helloMessage.sendBufferSize)
        connection->localConf.recvBufferSize = helloMessage.sendBufferSize;
    connection->remoteConf.sendBufferSize = helloMessage.sendBufferSize;
    connection->state = UA_CONNECTION_ESTABLISHED;
    UA_TcpHelloMessage_deleteMembers(&helloMessage);

    // build acknowledge response
    UA_TcpAcknowledgeMessage ackMessage;
    ackMessage.protocolVersion = connection->localConf.protocolVersion;
    ackMessage.receiveBufferSize = connection->localConf.recvBufferSize;
    ackMessage.sendBufferSize = connection->localConf.sendBufferSize;
    ackMessage.maxMessageSize = connection->localConf.maxMessageSize;
    ackMessage.maxChunkCount = connection->localConf.maxChunkCount;

    UA_TcpMessageHeader ackHeader;
    ackHeader.messageTypeAndChunkType = UA_MESSAGETYPE_ACK + UA_CHUNKTYPE_FINAL;
    ackHeader.messageSize = 8 + 20; /* ackHeader + ackMessage */

    UA_ByteString ack_msg;
    UA_ByteString_init(&ack_msg);
    if(connection->getSendBuffer(connection, connection->localConf.sendBufferSize,
                                 &ack_msg) != UA_STATUSCODE_GOOD)
        return;

    size_t tmpPos = 0;
    UA_TcpMessageHeader_encodeBinary(&ackHeader, &ack_msg, &tmpPos);
    UA_TcpAcknowledgeMessage_encodeBinary(&ackMessage, &ack_msg, &tmpPos);
    ack_msg.length = ackHeader.messageSize;
    connection->send(connection, &ack_msg);
}

static void processOPN(UA_Connection *connection, UA_Server *server, const UA_ByteString *msg, size_t *pos) {
    if(connection->state != UA_CONNECTION_ESTABLISHED) {
        connection->close(connection);
        return;
    }

    UA_UInt32 secureChannelId;
    UA_StatusCode retval = UA_UInt32_decodeBinary(msg, pos, &secureChannelId);

    //we can check secureChannelId also here -> if we are asked to isse a token it is 0, otherwise we have to renew
    //issue
    if(connection->channel == NULL && secureChannelId != 0){
        retval |= UA_STATUSCODE_BADREQUESTTYPEINVALID;
    }
    //renew
    if(connection->channel != NULL && secureChannelId != connection->channel->securityToken.channelId){
        retval |= UA_STATUSCODE_BADREQUESTTYPEINVALID;
    }

    UA_AsymmetricAlgorithmSecurityHeader asymHeader;
    retval |= UA_AsymmetricAlgorithmSecurityHeader_decodeBinary(msg, pos, &asymHeader);

    UA_SequenceHeader seqHeader;
    retval |= UA_SequenceHeader_decodeBinary(msg, pos, &seqHeader);

    UA_NodeId requestType;
    retval |= UA_NodeId_decodeBinary(msg, pos, &requestType);

    UA_OpenSecureChannelRequest r;
    retval |= UA_OpenSecureChannelRequest_decodeBinary(msg, pos, &r);

    if(retval != UA_STATUSCODE_GOOD || requestType.identifier.numeric != 446) {
        UA_AsymmetricAlgorithmSecurityHeader_deleteMembers(&asymHeader);
        UA_SequenceHeader_deleteMembers(&seqHeader);
        UA_NodeId_deleteMembers(&requestType);
        UA_OpenSecureChannelRequest_deleteMembers(&r);
        connection->close(connection);
        return;
    }


    UA_OpenSecureChannelResponse p;
    UA_OpenSecureChannelResponse_init(&p);
    Service_OpenSecureChannel(server, connection, &r, &p);
    UA_OpenSecureChannelRequest_deleteMembers(&r);

    UA_SecureChannel *channel = connection->channel;
    if(!channel) {
        connection->close(connection);
        UA_OpenSecureChannelResponse_deleteMembers(&p);
        UA_AsymmetricAlgorithmSecurityHeader_deleteMembers(&asymHeader);
        return;
    }

    /* send the response with an asymmetric security header */
#ifndef UA_ENABLE_MULTITHREADING
    seqHeader.sequenceNumber = ++channel->sequenceNumber;
#else
    seqHeader.sequenceNumber = uatomic_add_return(&channel->sequenceNumber, 1);
#endif

    UA_SecureConversationMessageHeader respHeader;
    respHeader.messageHeader.messageTypeAndChunkType = UA_MESSAGETYPE_OPN + UA_CHUNKTYPE_FINAL;
    respHeader.messageHeader.messageSize = 0;
    respHeader.secureChannelId = p.securityToken.channelId;

    UA_NodeId responseType = UA_NODEID_NUMERIC(0, UA_NS0ID_OPENSECURECHANNELRESPONSE +
                                               UA_ENCODINGOFFSET_BINARY);

    UA_ByteString resp_msg;
    UA_ByteString_init(&resp_msg);
    retval = connection->getSendBuffer(connection, connection->localConf.sendBufferSize, &resp_msg);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_OpenSecureChannelResponse_deleteMembers(&p);
        UA_AsymmetricAlgorithmSecurityHeader_deleteMembers(&asymHeader);
        return;
    }
        
    size_t tmpPos = 12; /* skip the secureconversationmessageheader for now */
    retval |= UA_AsymmetricAlgorithmSecurityHeader_encodeBinary(&asymHeader, &resp_msg, &tmpPos); // just mirror back
    retval |= UA_SequenceHeader_encodeBinary(&seqHeader, &resp_msg, &tmpPos); // just mirror back
    retval |= UA_NodeId_encodeBinary(&responseType, &resp_msg, &tmpPos);
    retval |= UA_OpenSecureChannelResponse_encodeBinary(&p, &resp_msg, &tmpPos);

    if(retval != UA_STATUSCODE_GOOD) {
        connection->releaseSendBuffer(connection, &resp_msg);
        connection->close(connection);
    } else {
        respHeader.messageHeader.messageSize = (UA_UInt32)tmpPos;
        tmpPos = 0;
        UA_SecureConversationMessageHeader_encodeBinary(&respHeader, &resp_msg, &tmpPos);
        resp_msg.length = respHeader.messageHeader.messageSize;
        connection->send(connection, &resp_msg);
    }
    UA_OpenSecureChannelResponse_deleteMembers(&p);
    UA_AsymmetricAlgorithmSecurityHeader_deleteMembers(&asymHeader);
}

static void init_response_header(const UA_RequestHeader *p, UA_ResponseHeader *r) {
    r->requestHandle = p->requestHandle;
    r->timestamp = UA_DateTime_now();
}

static void
getServicePointers(UA_UInt32 requestTypeId, const UA_DataType **requestType,
                   const UA_DataType **responseType, UA_Service *service) {
    switch(requestTypeId - UA_ENCODINGOFFSET_BINARY) {
    case UA_NS0ID_GETENDPOINTSREQUEST:
        *service = (UA_Service)Service_GetEndpoints;
        *requestType = &UA_TYPES[UA_TYPES_GETENDPOINTSREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_GETENDPOINTSRESPONSE];
        break;
    case UA_NS0ID_FINDSERVERSREQUEST:
        *service = (UA_Service)Service_FindServers;
        *requestType = &UA_TYPES[UA_TYPES_FINDSERVERSREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_FINDSERVERSRESPONSE];
        break;
    case UA_NS0ID_CREATESESSIONREQUEST:
        *service = (UA_Service)Service_CreateSession;
        *requestType = &UA_TYPES[UA_TYPES_CREATESESSIONREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_CREATESESSIONRESPONSE];
        break;
    case UA_NS0ID_ACTIVATESESSIONREQUEST:
        *service = (UA_Service)Service_ActivateSession;
        *requestType = &UA_TYPES[UA_TYPES_ACTIVATESESSIONREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_ACTIVATESESSIONRESPONSE];
        break;
    case UA_NS0ID_CLOSESESSIONREQUEST:
        *service = (UA_Service)Service_CloseSession;
        *requestType = &UA_TYPES[UA_TYPES_CLOSESESSIONREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_CLOSESESSIONRESPONSE];
        break;
    case UA_NS0ID_READREQUEST:
        *service = (UA_Service)Service_Read;
        *requestType = &UA_TYPES[UA_TYPES_READREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_READRESPONSE];
        break;
    case UA_NS0ID_WRITEREQUEST:
        *service = (UA_Service)Service_Write;
        *requestType = &UA_TYPES[UA_TYPES_WRITEREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_WRITERESPONSE];
        break;
    case UA_NS0ID_BROWSEREQUEST:
        *service = (UA_Service)Service_Browse;
        *requestType = &UA_TYPES[UA_TYPES_BROWSEREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_BROWSERESPONSE];
        break;
    case UA_NS0ID_BROWSENEXTREQUEST:
        *service = (UA_Service)Service_BrowseNext;
        *requestType = &UA_TYPES[UA_TYPES_BROWSENEXTREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_BROWSENEXTRESPONSE];
        break;
    case UA_NS0ID_REGISTERNODESREQUEST:
        *service = (UA_Service)Service_RegisterNodes;
        *requestType = &UA_TYPES[UA_TYPES_REGISTERNODESREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_REGISTERNODESRESPONSE];
        break;
    case UA_NS0ID_UNREGISTERNODESREQUEST:
        *service = (UA_Service)Service_UnregisterNodes;
        *requestType = &UA_TYPES[UA_TYPES_UNREGISTERNODESREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_UNREGISTERNODESRESPONSE];
        break;
    case UA_NS0ID_TRANSLATEBROWSEPATHSTONODEIDSREQUEST:
        *service = (UA_Service)Service_TranslateBrowsePathsToNodeIds;
        *requestType = &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_TRANSLATEBROWSEPATHSTONODEIDSRESPONSE];
        break;

#ifdef UA_ENABLE_SUBSCRIPTIONS
    case UA_NS0ID_CREATESUBSCRIPTIONREQUEST:
        *service = (UA_Service)Service_CreateSubscription;
        *requestType = &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_CREATESUBSCRIPTIONRESPONSE];
        break;
    case UA_NS0ID_PUBLISHREQUEST:
        *requestType = &UA_TYPES[UA_TYPES_PUBLISHREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_PUBLISHRESPONSE];
        break;
    case UA_NS0ID_REPUBLISHREQUEST:
        *service = (UA_Service)Service_Republish;
        *requestType = &UA_TYPES[UA_TYPES_REPUBLISHREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_REPUBLISHRESPONSE];
        break;
    case UA_NS0ID_MODIFYSUBSCRIPTIONREQUEST:
        *service = (UA_Service)Service_ModifySubscription;
        *requestType = &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_MODIFYSUBSCRIPTIONRESPONSE];
        break;
	case UA_NS0ID_SETPUBLISHINGMODEREQUEST:
		*service = (UA_Service)Service_SetPublishingMode;
		*requestType = &UA_TYPES[UA_TYPES_SETPUBLISHINGMODEREQUEST];
		*responseType = &UA_TYPES[UA_TYPES_SETPUBLISHINGMODERESPONSE];
		break;
    case UA_NS0ID_DELETESUBSCRIPTIONSREQUEST:
        *service = (UA_Service)Service_DeleteSubscriptions;
        *requestType = &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_DELETESUBSCRIPTIONSRESPONSE];
        break;
    case UA_NS0ID_CREATEMONITOREDITEMSREQUEST:
        *service = (UA_Service)Service_CreateMonitoredItems;
        *requestType = &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_CREATEMONITOREDITEMSRESPONSE];
        break;
    case UA_NS0ID_DELETEMONITOREDITEMSREQUEST:
        *service = (UA_Service)Service_DeleteMonitoredItems;
        *requestType = &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_DELETEMONITOREDITEMSRESPONSE];
        break;
    case UA_NS0ID_MODIFYMONITOREDITEMSREQUEST:
        *service = (UA_Service)Service_ModifyMonitoredItems;
        *requestType = &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_MODIFYMONITOREDITEMSRESPONSE];
        break;
#endif

#ifdef UA_ENABLE_METHODCALLS
    case UA_NS0ID_CALLREQUEST:
        *service = (UA_Service)Service_Call;
        *requestType = &UA_TYPES[UA_TYPES_CALLREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_CALLRESPONSE];
	break;
#endif

#ifdef UA_ENABLE_NODEMANAGEMENT
    case UA_NS0ID_ADDNODESREQUEST:
        *service = (UA_Service)Service_AddNodes;
        *requestType = &UA_TYPES[UA_TYPES_ADDNODESREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_ADDNODESRESPONSE];
        break;
    case UA_NS0ID_ADDREFERENCESREQUEST:
        *service = (UA_Service)Service_AddReferences;
        *requestType = &UA_TYPES[UA_TYPES_ADDREFERENCESREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_ADDREFERENCESRESPONSE];
        break;
    case UA_NS0ID_DELETENODESREQUEST:
        *service = (UA_Service)Service_DeleteNodes;
        *requestType = &UA_TYPES[UA_TYPES_DELETENODESREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_DELETENODESRESPONSE];
        break;
    case UA_NS0ID_DELETEREFERENCESREQUEST:
        *service = (UA_Service)Service_DeleteReferences;
        *requestType = &UA_TYPES[UA_TYPES_DELETEREFERENCESREQUEST];
        *responseType = &UA_TYPES[UA_TYPES_DELETEREFERENCESRESPONSE];
        break;
#endif

    default:
        break;
    }
}

static void
sendError(UA_SecureChannel *channel, const UA_ByteString *msg, size_t pos,
          UA_UInt32 requestId, UA_StatusCode error) {
    UA_RequestHeader p;
    if(UA_RequestHeader_decodeBinary(msg, &pos, &p) != UA_STATUSCODE_GOOD)
        return;
    UA_ResponseHeader r;
    UA_ResponseHeader_init(&r);
    init_response_header(&p, &r);
    r.serviceResult = error;
    UA_SecureChannel_sendBinaryMessage(channel, requestId, &r,
                                       &UA_TYPES[UA_TYPES_SERVICEFAULT]);
    UA_RequestHeader_deleteMembers(&p);
    UA_ResponseHeader_deleteMembers(&r);
}

static void
appendChunkedMessage(struct ChunkEntry *ch, const UA_ByteString *msg, size_t *pos) {
    if (ch->invalid_message) {
        return;
    }

    UA_UInt32 len;
    *pos -= 20;
    UA_UInt32_decodeBinary(msg, pos, &len);
    if (len > msg->length) {
        UA_ByteString_deleteMembers(&ch->bytes);
        ch->invalid_message = true;
        return;
    }
    len -= 24;
    *pos += 16; // 4 bytes consumed by decode above

    UA_Byte* new_bytes = UA_realloc(ch->bytes.data, ch->bytes.length + len);
    if (! new_bytes) {
        UA_ByteString_deleteMembers(&ch->bytes);
        ch->invalid_message = true;
        return;
    }
    ch->bytes.data = new_bytes;

    memcpy(&ch->bytes.data[ch->bytes.length], &msg->data[*pos], len);
    ch->bytes.length += len;
    *pos += len;
}

static struct ChunkEntry*
chunkEntryFromRequestId(UA_SecureChannel *channel, UA_UInt32 requestId) {
    struct ChunkEntry *ch;
    LIST_FOREACH(ch, &channel->chunks, pointers) {
        if (ch->requestId == requestId) {
            return ch;
        }
    }
    return NULL;
}

static void
processMSG(UA_Connection *connection, UA_Server *server, const UA_ByteString *msg, size_t *pos) {
    /* If we cannot decode these, don't respond */
    UA_UInt32 secureChannelId = 0;
    UA_UInt32 tokenId = 0;
    UA_SequenceHeader sequenceHeader;
    UA_NodeId requestTypeId;
    UA_StatusCode retval = UA_UInt32_decodeBinary(msg, pos, &secureChannelId);
    retval |= UA_UInt32_decodeBinary(msg, pos, &tokenId);
    retval |= UA_SequenceHeader_decodeBinary(msg, pos, &sequenceHeader);
    if(retval != UA_STATUSCODE_GOOD)
        return;

    UA_SecureChannel *channel = connection->channel;
    UA_SecureChannel anonymousChannel;
    if(!channel) {
        UA_SecureChannel_init(&anonymousChannel);
        anonymousChannel.connection = connection;
        channel = &anonymousChannel;
    }

    /* Test if the secure channel is ok */
    if(secureChannelId != channel->securityToken.channelId)
        return;
    if(tokenId != channel->securityToken.tokenId) {
        if(tokenId != channel->nextSecurityToken.tokenId) {
            /* close the securechannel but keep the connection open */
            UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_SECURECHANNEL,
                        "Request with a wrong security token. Closing the SecureChannel %i.",
                        channel->securityToken.channelId);
            Service_CloseSecureChannel(server, channel->securityToken.channelId);
            return;
        }
        UA_SecureChannel_revolveTokens(channel);
    }

    size_t final_chunked_pos = 0;
    UA_ByteString bytes;
    struct ChunkEntry *ch;
    switch (msg->data[*pos - 24 + 3]) {
    case 'C':
        UA_LOG_TRACE(server->config.logger, UA_LOGCATEGORY_SECURECHANNEL, "Chunk message");
        ch = chunkEntryFromRequestId(channel, sequenceHeader.requestId);
        if (! ch) {
            ch = UA_calloc(1, sizeof(struct ChunkEntry));
            ch->invalid_message = false;
            ch->requestId = sequenceHeader.requestId;
            UA_ByteString_init(&ch->bytes);
            LIST_INSERT_HEAD(&channel->chunks, ch, pointers);
        }

        appendChunkedMessage(ch, msg, pos);
        return;
    case 'F':
        ch = chunkEntryFromRequestId(channel, sequenceHeader.requestId);
        if (ch) {
            UA_LOG_TRACE(server->config.logger, UA_LOGCATEGORY_SECURECHANNEL, "Final chunk message");
            appendChunkedMessage(ch, msg, pos);

            bytes = ch->bytes;
            LIST_REMOVE(ch, pointers);
            UA_free(ch);

            final_chunked_pos = *pos;
            *pos = 0;

            // if the chunks have failed decoding
            // message is invalid => return early
            if (bytes.length == 0) {
                *pos = final_chunked_pos;
                return;
            }
        } else {
            bytes = *msg;
        }
        break;
    case 'A':
        ch = chunkEntryFromRequestId(channel, sequenceHeader.requestId);
        if (ch) {
            UA_ByteString_deleteMembers(&ch->bytes);
            LIST_REMOVE(ch, pointers);
            UA_free(ch);
        } else {
            UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_SECURECHANNEL,
                        "Received MSGA on an unknown request");
        }

        return;
    default:
        UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_SECURECHANNEL,
            "Received unknown message chunk: %c", msg->data[*pos - 24 + 3]);
        return;
    }

    retval |= UA_NodeId_decodeBinary(&bytes, pos, &requestTypeId);
    if(retval != UA_STATUSCODE_GOOD)
        return;

    /* Test if the service type nodeid has the right format */
    if(requestTypeId.identifierType != UA_NODEIDTYPE_NUMERIC ||
       requestTypeId.namespaceIndex != 0) {
        UA_NodeId_deleteMembers(&requestTypeId);
        sendError(channel, &bytes, *pos, sequenceHeader.requestId, UA_STATUSCODE_BADSERVICEUNSUPPORTED);
        return;
    }

    /* Get the service pointers */
    UA_Service service = NULL;
    const UA_DataType *requestType = NULL;
    const UA_DataType *responseType = NULL;
    getServicePointers(requestTypeId.identifier.numeric, &requestType, &responseType, &service);
    if(!requestType) {
        /* The service is not supported */
        if(requestTypeId.identifier.numeric==787)
            UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_SERVER,
                        "Client requested a subscription, but those are not enabled "
                        "in the build. The message will be skipped");
        else
            UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_SERVER,
                        "Unknown request: NodeId(ns=%d, i=%d)",
                        requestTypeId.namespaceIndex, requestTypeId.identifier.numeric);
        sendError(channel, &bytes, *pos, sequenceHeader.requestId, UA_STATUSCODE_BADSERVICEUNSUPPORTED);
        return;
    }

    /* Most services can only be called with a valid securechannel */
#ifndef UA_ENABLE_NONSTANDARD_STATELESS
    if(channel == &anonymousChannel &&
       requestType->typeIndex > UA_TYPES_OPENSECURECHANNELREQUEST) {
        sendError(channel, &bytes, *pos, sequenceHeader.requestId, UA_STATUSCODE_BADSECURECHANNELIDINVALID);
        return;
    }
#endif

    /* Decode the request */
    void *request = UA_alloca(requestType->memSize);
    size_t oldpos = *pos;
    retval = UA_decodeBinary(&bytes, pos, request, requestType);
    if(retval != UA_STATUSCODE_GOOD) {
        sendError(channel, &bytes, oldpos, sequenceHeader.requestId, retval);
        return;
    }

    /* Find the matching session */
    UA_Session *session =
        UA_SecureChannel_getSession(channel, &((UA_RequestHeader*)request)->authenticationToken);
    UA_Session anonymousSession;
    if(!session) {
        /* session id 0 -> anonymous session */
        UA_Session_init(&anonymousSession);
        anonymousSession.sessionId = UA_NODEID_NUMERIC(0,0);
        anonymousSession.channel = channel;
        anonymousSession.activated = true;
        session = &anonymousSession;
    }

    /* Test if the session is valid */
    if(!session->activated &&
       requestType->typeIndex != UA_TYPES_CREATESESSIONREQUEST &&
       requestType->typeIndex != UA_TYPES_ACTIVATESESSIONREQUEST &&
       requestType->typeIndex != UA_TYPES_FINDSERVERSREQUEST &&
       requestType->typeIndex != UA_TYPES_GETENDPOINTSREQUEST &&
       requestType->typeIndex != UA_TYPES_OPENSECURECHANNELREQUEST) {
        UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_SERVER,
                    "Client tries to call a service with a non-activated session");
        sendError(channel, &bytes, *pos, sequenceHeader.requestId, UA_STATUSCODE_BADSESSIONNOTACTIVATED);
        return;
    }

#ifndef UA_ENABLE_NONSTANDARD_STATELESS
    if(session == &anonymousSession &&
       requestType->typeIndex != UA_TYPES_CREATESESSIONREQUEST &&
       requestType->typeIndex != UA_TYPES_ACTIVATESESSIONREQUEST &&
       requestType->typeIndex != UA_TYPES_FINDSERVERSREQUEST &&
       requestType->typeIndex != UA_TYPES_GETENDPOINTSREQUEST &&
       requestType->typeIndex != UA_TYPES_OPENSECURECHANNELREQUEST) {
#ifdef UA_ENABLE_TYPENAMES
        UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_SERVER,
                    "Client sent a %s without a session", requestType->typeName);
#else
        UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_SERVER,
                    "Client tries to call a service without a session");
#endif
        sendError(channel, &bytes, *pos, sequenceHeader.requestId, UA_STATUSCODE_BADSESSIONIDINVALID);
        return;
    }
#endif

    UA_Session_updateLifetime(session);

#ifdef UA_ENABLE_SUBSCRIPTIONS
    /* The publish request is answered asynchronously */
    if(requestTypeId.identifier.numeric - UA_ENCODINGOFFSET_BINARY == UA_NS0ID_PUBLISHREQUEST) {
        Service_Publish(server, session, request, sequenceHeader.requestId);
        UA_deleteMembers(request, requestType);
        return;
    }
#endif
        
    /* Call the service */
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
                 "Processing a service with type id %u on Session %u",
                 requestType->typeId.identifier.numeric, session->authenticationToken.identifier.numeric);
    UA_assert(service);
    UA_assert(requestType);
    UA_assert(responseType);
    void *response = UA_alloca(responseType->memSize);
    UA_init(response, responseType);
    init_response_header(request, response);
    service(server, session, request, response);

    /* Send the response */
    retval = UA_SecureChannel_sendBinaryMessage(channel, sequenceHeader.requestId,
                                                response, responseType);
    if(retval != UA_STATUSCODE_GOOD) {
        /* e.g. UA_STATUSCODE_BADENCODINGLIMITSEXCEEDED */
        sendError(channel, &bytes, oldpos, sequenceHeader.requestId, retval);
    }

    /* Clean up */
    if (final_chunked_pos) {
        *pos = final_chunked_pos;
        UA_ByteString_deleteMembers(&bytes);
    }

    UA_deleteMembers(request, requestType);
    UA_deleteMembers(response, responseType);
    return;
}

static void
processCLO(UA_Connection *connection, UA_Server *server, const UA_ByteString *msg, size_t *pos) {
    UA_UInt32 secureChannelId;
    UA_StatusCode retval = UA_UInt32_decodeBinary(msg, pos, &secureChannelId);
    if(retval != UA_STATUSCODE_GOOD || !connection->channel ||
       connection->channel->securityToken.channelId != secureChannelId)
        return;
    Service_CloseSecureChannel(server, secureChannelId);
}

/**
 * process binary message received from Connection
 * dose not modify UA_ByteString you have to free it youself.
 * use of connection->getSendBuffer() and connection->send() to answer Message
 */
void UA_Server_processBinaryMessage(UA_Server *server, UA_Connection *connection, const UA_ByteString *msg) {
    size_t pos = 0;
    UA_TcpMessageHeader tcpMessageHeader;
    do {
        if(UA_TcpMessageHeader_decodeBinary(msg, &pos, &tcpMessageHeader)) {
            UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_NETWORK,
                        "Decoding of message header failed on Connection %i", connection->sockfd);
            connection->close(connection);
            break;
        }

        if(tcpMessageHeader.messageSize < 16) {
            UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_NETWORK,
                        "The message is suspiciously small on Connection %i", connection->sockfd);
            connection->close(connection);
            break;
        }

        size_t targetpos = pos - 8 + tcpMessageHeader.messageSize;
        switch(tcpMessageHeader.messageTypeAndChunkType & 0x00ffffff) {
        case UA_MESSAGETYPE_HEL:
            UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_NETWORK,
                         "Process a HEL on Connection %i", connection->sockfd);
            processHEL(connection, msg, &pos);
            break;
        case UA_MESSAGETYPE_OPN:
            processOPN(connection, server, msg, &pos);
            break;
        case UA_MESSAGETYPE_MSG:
#ifndef UA_ENABLE_NONSTANDARD_STATELESS
            if(connection->state != UA_CONNECTION_ESTABLISHED) {
                UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_NETWORK,
                             "Received a MSG where the connection is not established on Connection %i",
                             connection->sockfd);
                connection->close(connection);
                return;
            }
#endif
            UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_NETWORK,
                         "Process a MSG on Connection %i", connection->sockfd);
            processMSG(connection, server, msg, &pos);
            break;
        case UA_MESSAGETYPE_CLO:
            processCLO(connection, server, msg, &pos);
            connection->close(connection);
            return;
        default:
            UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_NETWORK,
                        "Unknown request type on Connection %i", connection->sockfd);
        }

        UA_TcpMessageHeader_deleteMembers(&tcpMessageHeader);
        if(pos != targetpos) {
            UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_NETWORK,
                        "Message on Connection %i was not entirely processed. "
                        "Arrived at position %i, skip after the announced length to position %i",
                        connection->sockfd, pos, targetpos);
            pos = targetpos;
        }
    } while(msg->length > pos);
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_nodes.c" ***********************************/


void UA_Node_deleteMembersAnyNodeClass(UA_Node *node) {
    /* delete standard content */
    UA_NodeId_deleteMembers(&node->nodeId);
    UA_QualifiedName_deleteMembers(&node->browseName);
    UA_LocalizedText_deleteMembers(&node->displayName);
    UA_LocalizedText_deleteMembers(&node->description);
    UA_Array_delete(node->references, node->referencesSize, &UA_TYPES[UA_TYPES_REFERENCENODE]);
    node->references = NULL;
    node->referencesSize = 0;

    /* delete unique content of the nodeclass */
    switch(node->nodeClass) {
    case UA_NODECLASS_OBJECT:
        break;
    case UA_NODECLASS_METHOD:
        break;
    case UA_NODECLASS_OBJECTTYPE:
        break;
    case UA_NODECLASS_VARIABLE:
    case UA_NODECLASS_VARIABLETYPE: {
        UA_VariableNode *p = (UA_VariableNode*)node;
        if(p->valueSource == UA_VALUESOURCE_VARIANT)
            UA_Variant_deleteMembers(&p->value.variant.value);
        break;
    }
    case UA_NODECLASS_REFERENCETYPE: {
        UA_ReferenceTypeNode *p = (UA_ReferenceTypeNode*)node;
        UA_LocalizedText_deleteMembers(&p->inverseName);
        break;
    }
    case UA_NODECLASS_DATATYPE:
        break;
    case UA_NODECLASS_VIEW:
        break;
    default:
        break;
    }
}

static UA_StatusCode
UA_ObjectNode_copy(const UA_ObjectNode *src, UA_ObjectNode *dst) {
    dst->eventNotifier = src->eventNotifier;
    dst->instanceHandle = src->instanceHandle;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
UA_VariableNode_copy(const UA_VariableNode *src, UA_VariableNode *dst) {
    dst->valueRank = src->valueRank;
    dst->valueSource = src->valueSource;
    if(src->valueSource == UA_VALUESOURCE_VARIANT) {
        UA_StatusCode retval = UA_Variant_copy(&src->value.variant.value, &dst->value.variant.value);
        if(retval != UA_STATUSCODE_GOOD)
            return retval;
        dst->value.variant.callback = src->value.variant.callback;
    } else
        dst->value.dataSource = src->value.dataSource;
    dst->accessLevel = src->accessLevel;
    dst->userAccessLevel = src->accessLevel;
    dst->minimumSamplingInterval = src->minimumSamplingInterval;
    dst->historizing = src->historizing;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
UA_MethodNode_copy(const UA_MethodNode *src, UA_MethodNode *dst) {
    dst->executable = src->executable;
    dst->userExecutable = src->userExecutable;
    dst->methodHandle  = src->methodHandle;
    dst->attachedMethod = src->attachedMethod;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
UA_ObjectTypeNode_copy(const UA_ObjectTypeNode *src, UA_ObjectTypeNode *dst) {
    dst->isAbstract = src->isAbstract;
    dst->lifecycleManagement = src->lifecycleManagement;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
UA_VariableTypeNode_copy(const UA_VariableTypeNode *src, UA_VariableTypeNode *dst) {
    dst->valueRank = src->valueRank;
    dst->valueSource = src->valueSource;
    if(src->valueSource == UA_VALUESOURCE_VARIANT){
        UA_StatusCode retval = UA_Variant_copy(&src->value.variant.value, &dst->value.variant.value);
        if(retval != UA_STATUSCODE_GOOD)
            return retval;
        dst->value.variant.callback = src->value.variant.callback;
    } else
        dst->value.dataSource = src->value.dataSource;
    dst->isAbstract = src->isAbstract;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
UA_ReferenceTypeNode_copy(const UA_ReferenceTypeNode *src, UA_ReferenceTypeNode *dst) {
    UA_StatusCode retval = UA_LocalizedText_copy(&src->inverseName, &dst->inverseName);
    dst->isAbstract = src->isAbstract;
    dst->symmetric = src->symmetric;
    return retval;
}

static UA_StatusCode
UA_DataTypeNode_copy(const UA_DataTypeNode *src, UA_DataTypeNode *dst) {
    dst->isAbstract = src->isAbstract;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
UA_ViewNode_copy(const UA_ViewNode *src, UA_ViewNode *dst) {
    dst->containsNoLoops = src->containsNoLoops;
    dst->eventNotifier = src->eventNotifier;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode UA_Node_copyAnyNodeClass(const UA_Node *src, UA_Node *dst) {
    if(src->nodeClass != dst->nodeClass)
        return UA_STATUSCODE_BADINTERNALERROR;
    
    /* copy standard content */
	UA_StatusCode retval = UA_NodeId_copy(&src->nodeId, &dst->nodeId);
	dst->nodeClass = src->nodeClass;
	retval |= UA_QualifiedName_copy(&src->browseName, &dst->browseName);
	retval |= UA_LocalizedText_copy(&src->displayName, &dst->displayName);
	retval |= UA_LocalizedText_copy(&src->description, &dst->description);
	dst->writeMask = src->writeMask;
	dst->userWriteMask = src->userWriteMask;
	if(retval != UA_STATUSCODE_GOOD) {
    	UA_Node_deleteMembersAnyNodeClass(dst);
        return retval;
    }
	retval |= UA_Array_copy(src->references, src->referencesSize, (void**)&dst->references,
                            &UA_TYPES[UA_TYPES_REFERENCENODE]);
	if(retval != UA_STATUSCODE_GOOD) {
    	UA_Node_deleteMembersAnyNodeClass(dst);
        return retval;
    }
    dst->referencesSize = src->referencesSize;

    /* copy unique content of the nodeclass */
    switch(src->nodeClass) {
    case UA_NODECLASS_OBJECT:
        retval = UA_ObjectNode_copy((const UA_ObjectNode*)src, (UA_ObjectNode*)dst);
        break;
    case UA_NODECLASS_VARIABLE:
        retval = UA_VariableNode_copy((const UA_VariableNode*)src, (UA_VariableNode*)dst);
        break;
    case UA_NODECLASS_METHOD:
        retval = UA_MethodNode_copy((const UA_MethodNode*)src, (UA_MethodNode*)dst);
        break;
    case UA_NODECLASS_OBJECTTYPE:
        retval = UA_ObjectTypeNode_copy((const UA_ObjectTypeNode*)src, (UA_ObjectTypeNode*)dst);
        break;
    case UA_NODECLASS_VARIABLETYPE:
        retval = UA_VariableTypeNode_copy((const UA_VariableTypeNode*)src, (UA_VariableTypeNode*)dst);
        break;
    case UA_NODECLASS_REFERENCETYPE:
        retval = UA_ReferenceTypeNode_copy((const UA_ReferenceTypeNode*)src, (UA_ReferenceTypeNode*)dst);
        break;
    case UA_NODECLASS_DATATYPE:
        retval = UA_DataTypeNode_copy((const UA_DataTypeNode*)src, (UA_DataTypeNode*)dst);
        break;
    case UA_NODECLASS_VIEW:
        retval = UA_ViewNode_copy((const UA_ViewNode*)src, (UA_ViewNode*)dst);
        break;
    default:
        break;
    }
	if(retval != UA_STATUSCODE_GOOD)
    	UA_Node_deleteMembersAnyNodeClass(dst);
    return retval;
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_server_worker.c" ***********************************/


/**
 * There are four types of job execution:
 *
 * 1. Normal jobs (dispatched to worker threads if multithreading is activated)
 *
 * 2. Repeated jobs with a repetition interval (dispatched to worker threads)
 *
 * 3. Mainloop jobs are executed (once) from the mainloop and not in the worker threads. The server
 * contains a stack structure where all threads can add mainloop jobs for the next mainloop
 * iteration. This is used e.g. to trigger adding and removing repeated jobs without blocking the
 * mainloop.
 *
 * 4. Delayed jobs are executed once in a worker thread. But only when all normal jobs that were
 * dispatched earlier have been executed. This is achieved by a counter in the worker threads. We
 * compute from the counter if all previous jobs have finished. The delay can be very long, since we
 * try to not interfere too much with normal execution. A use case is to eventually free obsolete
 * structures that _could_ still be accessed from concurrent threads.
 *
 * - Remove the entry from the list
 * - mark it as "dead" with an atomic operation
 * - add a delayed job that frees the memory when all concurrent operations have completed
 * 
 * This approach to concurrently accessible memory is known as epoch based reclamation [1]. According to
 * [2], it performs competitively well on many-core systems. Our version of EBR does however not require
 * a global epoch. Instead, every worker thread has its own epoch counter that we observe for changes.
 * 
 * [1] Fraser, K. 2003. Practical lock freedom. Ph.D. thesis. Computer Laboratory, University of Cambridge.
 * [2] Hart, T. E., McKenney, P. E., Brown, A. D., & Walpole, J. (2007). Performance of memory reclamation
 *     for lockless synchronization. Journal of Parallel and Distributed Computing, 67(12), 1270-1285.
 * 
 * 
 */

#define MAXTIMEOUT 500 // max timeout in millisec until the next main loop iteration
#define BATCHSIZE 20 // max number of jobs that are dispatched at once to workers

static void processJobs(UA_Server *server, UA_Job *jobs, size_t jobsSize) {
    UA_ASSERT_RCU_UNLOCKED();
    UA_RCU_LOCK();
    for(size_t i = 0; i < jobsSize; i++) {
        UA_Job *job = &jobs[i];
        switch(job->type) {
        case UA_JOBTYPE_NOTHING:
            break;
        case UA_JOBTYPE_DETACHCONNECTION:
            UA_Connection_detachSecureChannel(job->job.closeConnection);
            break;
        case UA_JOBTYPE_BINARYMESSAGE_NETWORKLAYER:
            UA_Server_processBinaryMessage(server, job->job.binaryMessage.connection,
                                           &job->job.binaryMessage.message);
            UA_Connection *connection = job->job.binaryMessage.connection;
            connection->releaseRecvBuffer(connection, &job->job.binaryMessage.message);
            break;
        case UA_JOBTYPE_BINARYMESSAGE_ALLOCATED:
            UA_Server_processBinaryMessage(server, job->job.binaryMessage.connection,
                                           &job->job.binaryMessage.message);
            UA_ByteString_deleteMembers(&job->job.binaryMessage.message);
            break;
        case UA_JOBTYPE_METHODCALL:
        case UA_JOBTYPE_METHODCALL_DELAYED:
            job->job.methodCall.method(server, job->job.methodCall.data);
            break;
        default:
            UA_LOG_WARNING(server->config.logger, UA_LOGCATEGORY_SERVER,
                           "Trying to execute a job of unknown type");
            break;
        }
    }
    UA_RCU_UNLOCK();
}

/*******************************/
/* Worker Threads and Dispatch */
/*******************************/

#ifdef UA_ENABLE_MULTITHREADING

struct MainLoopJob {
    struct cds_lfs_node node;
    UA_Job job;
};

/** Entry in the dispatch queue */
struct DispatchJobsList {
    struct cds_wfcq_node node; // node for the queue
    size_t jobsSize;
    UA_Job *jobs;
};

static void * workerLoop(UA_Worker *worker) {
    UA_Server *server = worker->server;
    UA_UInt32 *counter = &worker->counter;
    volatile UA_Boolean *running = &worker->running;
    
    /* Initialize the (thread local) random seed with the ram address of worker */
    UA_random_seed((uintptr_t)worker);
   	rcu_register_thread();

    pthread_mutex_t mutex; // required for the condition variable
    pthread_mutex_init(&mutex,0);
    pthread_mutex_lock(&mutex);

    while(*running) {
        struct DispatchJobsList *wln = (struct DispatchJobsList*)
            cds_wfcq_dequeue_blocking(&server->dispatchQueue_head, &server->dispatchQueue_tail);
        if(!wln) {
            uatomic_inc(counter);
            /* sleep until a work arrives (and wakes up all worker threads) */
            pthread_cond_wait(&server->dispatchQueue_condition, &mutex);
            continue;
        }
        processJobs(server, wln->jobs, wln->jobsSize);
        UA_free(wln->jobs);
        UA_free(wln);
        uatomic_inc(counter);
    }

    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    UA_ASSERT_RCU_UNLOCKED();
    rcu_barrier(); // wait for all scheduled call_rcu work to complete
   	rcu_unregister_thread();
    return NULL;
}

/** Dispatch jobs to workers. Slices the job array up if it contains more than
    BATCHSIZE items. The jobs array is freed in the worker threads. */
static void dispatchJobs(UA_Server *server, UA_Job *jobs, size_t jobsSize) {
    size_t startIndex = jobsSize; // start at the end
    while(jobsSize > 0) {
        size_t size = BATCHSIZE;
        if(size > jobsSize)
            size = jobsSize;
        startIndex = startIndex - size;
        struct DispatchJobsList *wln = UA_malloc(sizeof(struct DispatchJobsList));
        if(startIndex > 0) {
            wln->jobs = UA_malloc(size * sizeof(UA_Job));
            memcpy(wln->jobs, &jobs[startIndex], size * sizeof(UA_Job));
            wln->jobsSize = size;
        } else {
            /* forward the original array */
            wln->jobsSize = size;
            wln->jobs = jobs;
        }
        cds_wfcq_node_init(&wln->node);
        cds_wfcq_enqueue(&server->dispatchQueue_head, &server->dispatchQueue_tail, &wln->node);
        jobsSize -= size;
    }
}

static void
emptyDispatchQueue(UA_Server *server) {
    while(!cds_wfcq_empty(&server->dispatchQueue_head, &server->dispatchQueue_tail)) {
        struct DispatchJobsList *wln = (struct DispatchJobsList*)
            cds_wfcq_dequeue_blocking(&server->dispatchQueue_head, &server->dispatchQueue_tail);
        processJobs(server, wln->jobs, wln->jobsSize);
        UA_free(wln->jobs);
        UA_free(wln);
    }
}

#endif

/*****************/
/* Repeated Jobs */
/*****************/

struct IdentifiedJob {
    UA_Job job;
    UA_Guid id;
};

/**
 * The RepeatedJobs structure contains an array of jobs that are either executed with the same
 * repetition interval. The linked list is sorted, so we can stop traversing when the first element
 * has nextTime > now.
 */
struct RepeatedJobs {
    LIST_ENTRY(RepeatedJobs) pointers; ///> Links to the next list of repeated jobs (with a different) interval
    UA_DateTime nextTime; ///> The next time when the jobs are to be executed
    UA_UInt32 interval; ///> Interval in 100ns resolution
    size_t jobsSize; ///> Number of jobs contained
    struct IdentifiedJob jobs[]; ///> The jobs. This is not a pointer, instead the struct is variable sized.
};

/* throwaway struct for the mainloop callback */
struct AddRepeatedJob {
    struct IdentifiedJob job;
    UA_UInt32 interval;
};

/* internal. call only from the main loop. */
static UA_StatusCode addRepeatedJob(UA_Server *server, struct AddRepeatedJob * UA_RESTRICT arw) {
    struct RepeatedJobs *matchingTw = NULL; // add the item here
    struct RepeatedJobs *lastTw = NULL; // if there is no repeated job, add a new one this entry
    struct RepeatedJobs *tempTw;
    UA_StatusCode retval = UA_STATUSCODE_GOOD;

    /* search for matching entry */
    UA_DateTime firstTime = UA_DateTime_nowMonotonic();
    tempTw = LIST_FIRST(&server->repeatedJobs);
    while(tempTw) {
        if(arw->interval == tempTw->interval) {
            matchingTw = tempTw;
            break;
        }
        if(tempTw->nextTime > firstTime)
            break;
        lastTw = tempTw;
        tempTw = LIST_NEXT(lastTw, pointers);
    }

    if(matchingTw) {
        /* append to matching entry */
        matchingTw = UA_realloc(matchingTw, sizeof(struct RepeatedJobs) +
                                (sizeof(struct IdentifiedJob) * (matchingTw->jobsSize + 1)));
        if(!matchingTw) {
            retval = UA_STATUSCODE_BADOUTOFMEMORY;
            goto cleanup;
        }
        /* link the reallocated tw into the list */
        LIST_REPLACE(matchingTw, matchingTw, pointers);
    } else {
        /* create a new entry */
        matchingTw = UA_malloc(sizeof(struct RepeatedJobs) + sizeof(struct IdentifiedJob));
        if(!matchingTw) {
            retval = UA_STATUSCODE_BADOUTOFMEMORY;
            goto cleanup;
        }
        matchingTw->jobsSize = 0;
        matchingTw->nextTime = firstTime;
        matchingTw->interval = arw->interval;
        if(lastTw)
            LIST_INSERT_AFTER(lastTw, matchingTw, pointers);
        else
            LIST_INSERT_HEAD(&server->repeatedJobs, matchingTw, pointers);
    }
    matchingTw->jobs[matchingTw->jobsSize] = arw->job;
    matchingTw->jobsSize++;

 cleanup:
#ifdef UA_ENABLE_MULTITHREADING
    UA_free(arw);
#endif
    return retval;
}

UA_StatusCode UA_Server_addRepeatedJob(UA_Server *server, UA_Job job, UA_UInt32 interval, UA_Guid *jobId) {
    /* the interval needs to be at least 5ms */
    if(interval < 5)
        return UA_STATUSCODE_BADINTERNALERROR;
    interval *= (UA_UInt32)UA_MSEC_TO_DATETIME; // from ms to 100ns resolution

#ifdef UA_ENABLE_MULTITHREADING
    struct AddRepeatedJob *arw = UA_malloc(sizeof(struct AddRepeatedJob));
    if(!arw)
        return UA_STATUSCODE_BADOUTOFMEMORY;

    arw->interval = interval;
    arw->job.job = job;
    if(jobId) {
        arw->job.id = UA_Guid_random();
        *jobId = arw->job.id;
    } else
        UA_Guid_init(&arw->job.id);

    struct MainLoopJob *mlw = UA_malloc(sizeof(struct MainLoopJob));
    if(!mlw) {
        UA_free(arw);
        return UA_STATUSCODE_BADOUTOFMEMORY;
    }
    mlw->job = (UA_Job) {
        .type = UA_JOBTYPE_METHODCALL,
        .job.methodCall = {.data = arw, .method = (void (*)(UA_Server*, void*))addRepeatedJob}};
    cds_lfs_push(&server->mainLoopJobs, &mlw->node);
#else
    struct AddRepeatedJob arw;
    arw.interval = interval;
    arw.job.job = job;
    if(jobId) {
        arw.job.id = UA_Guid_random();
        *jobId = arw.job.id;
    } else
        UA_Guid_init(&arw.job.id);
    addRepeatedJob(server, &arw);
#endif
    return UA_STATUSCODE_GOOD;
}

/* Returns the next datetime when a repeated job is scheduled */
static UA_DateTime processRepeatedJobs(UA_Server *server, UA_DateTime current) {
    struct RepeatedJobs *tw, *tmp_tw;
    /* Iterate over the list of elements (sorted according to the next execution timestamp) */
    LIST_FOREACH_SAFE(tw, &server->repeatedJobs, pointers, tmp_tw) {
        if(tw->nextTime > current)
            break;

#ifdef UA_ENABLE_MULTITHREADING
        // copy the entry and insert at the new location
        UA_Job *jobsCopy = UA_malloc(sizeof(UA_Job) * tw->jobsSize);
        if(!jobsCopy) {
            UA_LOG_ERROR(server->config.logger, UA_LOGCATEGORY_SERVER,
                         "Not enough memory to dispatch delayed jobs");
            break;
        }
        for(size_t i=0;i<tw->jobsSize;i++)
            jobsCopy[i] = tw->jobs[i].job;
        dispatchJobs(server, jobsCopy, tw->jobsSize); // frees the job pointer
#else
		size_t size = tw->jobsSize;
        for(size_t i = 0; i < size; i++)
            processJobs(server, &tw->jobs[i].job, 1); // does not free the job ptr
#endif

		/* Elements are removed only here. Check if empty. */
		if(tw->jobsSize == 0) {
			LIST_REMOVE(tw, pointers);
			UA_free(tw);
            UA_assert(LIST_FIRST(&server->repeatedJobs) != tw); /* Assert for static code checkers */
			continue;
		}

        /* Set the time for the next execution */
        tw->nextTime += tw->interval;
        if(tw->nextTime < current)
            tw->nextTime = current;

        /* Reinsert to keep the list sorted */
        struct RepeatedJobs *prevTw = LIST_FIRST(&server->repeatedJobs);
        while(true) {
            struct RepeatedJobs *n = LIST_NEXT(prevTw, pointers);
            if(!n || n->nextTime > tw->nextTime)
                break;
            prevTw = n;
        }
        if(prevTw != tw) {
            LIST_REMOVE(tw, pointers);
            LIST_INSERT_AFTER(prevTw, tw, pointers);
        }
    }

    // check if the next repeated job is sooner than the usual timeout
    // calc in 32 bit must be ok
    struct RepeatedJobs *first = LIST_FIRST(&server->repeatedJobs);
    UA_DateTime next = current + (MAXTIMEOUT * UA_MSEC_TO_DATETIME);
    if(first && first->nextTime < next)
        next = first->nextTime;
    return next;
}

/* Call this function only from the main loop! */
static void removeRepeatedJob(UA_Server *server, UA_Guid *jobId) {
    struct RepeatedJobs *tw;
    LIST_FOREACH(tw, &server->repeatedJobs, pointers) {
        for(size_t i = 0; i < tw->jobsSize; i++) {
            if(!UA_Guid_equal(jobId, &tw->jobs[i].id))
                continue;
			tw->jobsSize--; /* if size == 0, tw is freed during the next processing */
            if(tw->jobsSize > 0)
                tw->jobs[i] = tw->jobs[tw->jobsSize]; // move the last entry to overwrite
            goto finish;
        }
    }
 finish:
#ifdef UA_ENABLE_MULTITHREADING
    UA_free(jobId);
#endif
    return;
}

UA_StatusCode UA_Server_removeRepeatedJob(UA_Server *server, UA_Guid jobId) {
#ifdef UA_ENABLE_MULTITHREADING
    UA_Guid *idptr = UA_malloc(sizeof(UA_Guid));
    if(!idptr)
        return UA_STATUSCODE_BADOUTOFMEMORY;
    *idptr = jobId;
    // dispatch to the mainloopjobs stack
    struct MainLoopJob *mlw = UA_malloc(sizeof(struct MainLoopJob));
    mlw->job = (UA_Job) {
        .type = UA_JOBTYPE_METHODCALL,
        .job.methodCall = {.data = idptr, .method = (void (*)(UA_Server*, void*))removeRepeatedJob}};
    cds_lfs_push(&server->mainLoopJobs, &mlw->node);
#else
    removeRepeatedJob(server, &jobId);
#endif
    return UA_STATUSCODE_GOOD;
}

void UA_Server_deleteAllRepeatedJobs(UA_Server *server) {
    struct RepeatedJobs *current, *temp;
    LIST_FOREACH_SAFE(current, &server->repeatedJobs, pointers, temp) {
        LIST_REMOVE(current, pointers);
        UA_free(current);
    }
}

/****************/
/* Delayed Jobs */
/****************/

#ifdef UA_ENABLE_MULTITHREADING

#define DELAYEDJOBSSIZE 100 // Collect delayed jobs until we have DELAYEDWORKSIZE items

struct DelayedJobs {
    struct DelayedJobs *next;
    UA_UInt32 *workerCounters; // initially NULL until the counter are set
    UA_UInt32 jobsCount; // the size of the array is DELAYEDJOBSSIZE, the count may be less
    UA_Job jobs[DELAYEDJOBSSIZE]; // when it runs full, a new delayedJobs entry is created
};

/* Dispatched as an ordinary job when the DelayedJobs list is full */
static void getCounters(UA_Server *server, struct DelayedJobs *delayed) {
    UA_UInt32 *counters = UA_malloc(server->config.nThreads * sizeof(UA_UInt32));
    for(UA_UInt16 i = 0; i < server->config.nThreads; i++)
        counters[i] = server->workers[i].counter;
    delayed->workerCounters = counters;
}

// Call from the main thread only. This is the only function that modifies
// server->delayedWork. processDelayedWorkQueue modifies the "next" (after the
// head).
static void addDelayedJob(UA_Server *server, UA_Job *job) {
    struct DelayedJobs *dj = server->delayedJobs;
    if(!dj || dj->jobsCount >= DELAYEDJOBSSIZE) {
        /* create a new DelayedJobs and add it to the linked list */
        dj = UA_malloc(sizeof(struct DelayedJobs));
        if(!dj) {
            UA_LOG_ERROR(server->config.logger, UA_LOGCATEGORY_SERVER,
                         "Not enough memory to add a delayed job");
            return;
        }
        dj->jobsCount = 0;
        dj->workerCounters = NULL;
        dj->next = server->delayedJobs;
        server->delayedJobs = dj;

        /* dispatch a method that sets the counter for the full list that comes afterwards */
        if(dj->next) {
            UA_Job *setCounter = UA_malloc(sizeof(UA_Job));
            *setCounter = (UA_Job) {.type = UA_JOBTYPE_METHODCALL, .job.methodCall =
                                    {.method = (void (*)(UA_Server*, void*))getCounters, .data = dj->next}};
            dispatchJobs(server, setCounter, 1);
        }
    }
    dj->jobs[dj->jobsCount] = *job;
    dj->jobsCount++;
}

static void addDelayedJobAsync(UA_Server *server, UA_Job *job) {
    addDelayedJob(server, job);
    UA_free(job);
}

static void server_free(UA_Server *server, void *data) {
    UA_free(data);
}

UA_StatusCode UA_Server_delayedFree(UA_Server *server, void *data) {
    UA_Job *j = UA_malloc(sizeof(UA_Job));
    if(!j)
        return UA_STATUSCODE_BADOUTOFMEMORY;
    j->type = UA_JOBTYPE_METHODCALL;
    j->job.methodCall.data = data;
    j->job.methodCall.method = server_free;
    struct MainLoopJob *mlw = UA_malloc(sizeof(struct MainLoopJob));
    mlw->job = (UA_Job) {.type = UA_JOBTYPE_METHODCALL, .job.methodCall =
                         {.data = j, .method = (UA_ServerCallback)addDelayedJobAsync}};
    cds_lfs_push(&server->mainLoopJobs, &mlw->node);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
UA_Server_delayedCallback(UA_Server *server, UA_ServerCallback callback, void *data) {
    UA_Job *j = UA_malloc(sizeof(UA_Job));
    if(!j)
        return UA_STATUSCODE_BADOUTOFMEMORY;
    j->type = UA_JOBTYPE_METHODCALL;
    j->job.methodCall.data = data;
    j->job.methodCall.method = callback;
    struct MainLoopJob *mlw = UA_malloc(sizeof(struct MainLoopJob));
    mlw->job = (UA_Job) {.type = UA_JOBTYPE_METHODCALL, .job.methodCall =
                         {.data = j, .method = (UA_ServerCallback)addDelayedJobAsync}};
    cds_lfs_push(&server->mainLoopJobs, &mlw->node);
    return UA_STATUSCODE_GOOD;
}

/* Find out which delayed jobs can be executed now */
static void
dispatchDelayedJobs(UA_Server *server, void *_) {
    /* start at the second */
    struct DelayedJobs *dw = server->delayedJobs, *beforedw = dw;
    if(dw)
        dw = dw->next;

    /* find the first delayedwork where the counters have been set and have moved */
    while(dw) {
        if(!dw->workerCounters) {
            beforedw = dw;
            dw = dw->next;
            continue;
        }
        UA_Boolean allMoved = true;
        for(size_t i = 0; i < server->config.nThreads; i++) {
            if(dw->workerCounters[i] == server->workers[i].counter) {
                allMoved = false;
                break;
            }
        }
        if(allMoved)
            break;
        beforedw = dw;
        dw = dw->next;
    }

#if (__GNUC__ <= 4 && __GNUC_MINOR__ <= 6)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Wunused-value"
#endif
    /* process and free all delayed jobs from here on */
    while(dw) {
        processJobs(server, dw->jobs, dw->jobsCount);
        struct DelayedJobs *next = uatomic_xchg(&beforedw->next, NULL);
        UA_free(dw);
        UA_free(dw->workerCounters);
        dw = next;
    }
#if (__GNUC__ <= 4 && __GNUC_MINOR__ <= 6)
#pragma GCC diagnostic pop
#endif

}

#endif

/********************/
/* Main Server Loop */
/********************/

#ifdef UA_ENABLE_MULTITHREADING
static void processMainLoopJobs(UA_Server *server) {
    /* no synchronization required if we only use push and pop_all */
    struct cds_lfs_head *head = __cds_lfs_pop_all(&server->mainLoopJobs);
    if(!head)
        return;
    struct MainLoopJob *mlw = (struct MainLoopJob*)&head->node;
    struct MainLoopJob *next;
    do {
        processJobs(server, &mlw->job, 1);
        next = (struct MainLoopJob*)mlw->node.next;
        UA_free(mlw);
        //cppcheck-suppress unreadVariable
    } while((mlw = next));
    //UA_free(head);
}
#endif

UA_StatusCode UA_Server_run_startup(UA_Server *server) {
#ifdef UA_ENABLE_MULTITHREADING
    /* Spin up the worker threads */
    UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_SERVER,
                "Spinning up %u worker thread(s)", server->config.nThreads);
    pthread_cond_init(&server->dispatchQueue_condition, 0);
    server->workers = UA_malloc(server->config.nThreads * sizeof(UA_Worker));
    if(!server->workers)
        return UA_STATUSCODE_BADOUTOFMEMORY;
    for(size_t i = 0; i < server->config.nThreads; i++) {
        UA_Worker *worker = &server->workers[i];
        worker->server = server;
        worker->counter = 0;
        worker->running = true;
        pthread_create(&worker->thr, NULL, (void* (*)(void*))workerLoop, worker);
    }

    /* Try to execute delayed callbacks every 10 sec */
    UA_Job processDelayed = {.type = UA_JOBTYPE_METHODCALL,
                             .job.methodCall = {.method = dispatchDelayedJobs, .data = NULL} };
    UA_Server_addRepeatedJob(server, processDelayed, 10000, NULL);
#endif

    /* Start the networklayers */
    UA_StatusCode result = UA_STATUSCODE_GOOD;
    for(size_t i = 0; i < server->config.networkLayersSize; i++) {
        UA_ServerNetworkLayer *nl = &server->config.networkLayers[i];
        result |= nl->start(nl, server->config.logger);
        for(size_t j = 0; j < server->endpointDescriptionsSize; j++) {
            UA_String_copy(&nl->discoveryUrl, &server->endpointDescriptions[j].endpointUrl);
        }
    }

    return result;
}

static void completeMessages(UA_Server *server, UA_Job *job) {
    UA_Boolean realloced = UA_FALSE;
    UA_StatusCode retval = UA_Connection_completeMessages(job->job.binaryMessage.connection,
                                                          &job->job.binaryMessage.message, &realloced);
    if(retval != UA_STATUSCODE_GOOD) {
        if(retval == UA_STATUSCODE_BADOUTOFMEMORY)
            UA_LOG_WARNING(server->config.logger, UA_LOGCATEGORY_NETWORK,
                           "Lost message(s) from Connection %i as memory could not be allocated",
                           job->job.binaryMessage.connection->sockfd);
        else if(retval != UA_STATUSCODE_GOOD)
            UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_NETWORK,
                        "Could not merge half-received messages on Connection %i with error 0x%08x",
                        job->job.binaryMessage.connection->sockfd, retval);
        job->type = UA_JOBTYPE_NOTHING;
        return;
    }
    if(realloced)
        job->type = UA_JOBTYPE_BINARYMESSAGE_ALLOCATED;
}

UA_UInt16 UA_Server_run_iterate(UA_Server *server, UA_Boolean waitInternal) {
#ifdef UA_ENABLE_MULTITHREADING
    /* Run work assigned for the main thread */
    processMainLoopJobs(server);
#endif
    /* Process repeated work */
    UA_DateTime now = UA_DateTime_nowMonotonic();
    UA_DateTime nextRepeated = processRepeatedJobs(server, now);

    UA_UInt16 timeout = 0;
    if(waitInternal)
        timeout = (UA_UInt16)((nextRepeated - now) / UA_MSEC_TO_DATETIME);

    /* Get work from the networklayer */
    for(size_t i = 0; i < server->config.networkLayersSize; i++) {
        UA_ServerNetworkLayer *nl = &server->config.networkLayers[i];
        UA_Job *jobs;
        size_t jobsSize;
        /* only the last networklayer waits on the tieout */
        if(i == server->config.networkLayersSize-1)
            jobsSize = nl->getJobs(nl, &jobs, timeout);
        else
            jobsSize = nl->getJobs(nl, &jobs, 0);

        for(size_t k = 0; k < jobsSize; k++) {
#ifdef UA_ENABLE_MULTITHREADING
            /* Filter out delayed work */
            if(jobs[k].type == UA_JOBTYPE_METHODCALL_DELAYED) {
                addDelayedJob(server, &jobs[k]);
                jobs[k].type = UA_JOBTYPE_NOTHING;
                continue;
            }
#endif
            /* Merge half-received messages */
            if(jobs[k].type == UA_JOBTYPE_BINARYMESSAGE_NETWORKLAYER)
                completeMessages(server, &jobs[k]);
        }

#ifdef UA_ENABLE_MULTITHREADING
        dispatchJobs(server, jobs, jobsSize);
        /* Wake up worker threads */
        if(jobsSize > 0)
            pthread_cond_broadcast(&server->dispatchQueue_condition);
#else
        processJobs(server, jobs, jobsSize);
        if(jobsSize > 0)
            UA_free(jobs);
#endif
    }

    now = UA_DateTime_nowMonotonic();
    timeout = 0;
    if(nextRepeated > now)
        timeout = (UA_UInt16)((nextRepeated - now) / UA_MSEC_TO_DATETIME);
    return timeout;
}

UA_StatusCode UA_Server_run_shutdown(UA_Server *server) {
    for(size_t i = 0; i < server->config.networkLayersSize; i++) {
        UA_ServerNetworkLayer *nl = &server->config.networkLayers[i];
        UA_Job *stopJobs;
        size_t stopJobsSize = nl->stop(nl, &stopJobs);
        processJobs(server, stopJobs, stopJobsSize);
        UA_free(stopJobs);
    }

#ifdef UA_ENABLE_MULTITHREADING
    UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_SERVER,
                "Shutting down %u worker thread(s)", server->config.nThreads);
    /* Wait for all worker threads to finish */
    for(size_t i = 0; i < server->config.nThreads; i++)
        server->workers[i].running = false;
    pthread_cond_broadcast(&server->dispatchQueue_condition);
    for(size_t i = 0; i < server->config.nThreads; i++)
        pthread_join(server->workers[i].thr, NULL);
    UA_free(server->workers);

    /* Manually finish the work still enqueued.
       This especially contains delayed frees */
    emptyDispatchQueue(server);
    UA_ASSERT_RCU_UNLOCKED();
    rcu_barrier(); // wait for all scheduled call_rcu work to complete
#endif
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode UA_Server_run(UA_Server *server, volatile UA_Boolean *running) {
    UA_StatusCode retval = UA_Server_run_startup(server);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;
    while(*running)
        UA_Server_run_iterate(server, true);
    return UA_Server_run_shutdown(server);
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_securechannel_manager.c" ***********************************/


UA_StatusCode
UA_SecureChannelManager_init(UA_SecureChannelManager *cm, size_t maxChannelCount,
                             UA_UInt32 tokenLifetime, UA_UInt32 startChannelId,
                             UA_UInt32 startTokenId, UA_Server *server) {
    LIST_INIT(&cm->channels);
    cm->lastChannelId = startChannelId;
    cm->lastTokenId = startTokenId;
    cm->maxChannelLifetime = tokenLifetime;
    cm->maxChannelCount = maxChannelCount;
    cm->currentChannelCount = 0;
    cm->server = server;
    return UA_STATUSCODE_GOOD;
}

void UA_SecureChannelManager_deleteMembers(UA_SecureChannelManager *cm) {
    channel_list_entry *entry, *temp;
    LIST_FOREACH_SAFE(entry, &cm->channels, pointers, temp) {
        LIST_REMOVE(entry, pointers);
        UA_SecureChannel_deleteMembersCleanup(&entry->channel);
        UA_free(entry);
    }
}

/* remove channels that were not renewed or who have no connection attached */
void UA_SecureChannelManager_cleanupTimedOut(UA_SecureChannelManager *cm, UA_DateTime now) {
    channel_list_entry *entry, *temp;
    LIST_FOREACH_SAFE(entry, &cm->channels, pointers, temp) {
        UA_DateTime timeout =
            entry->channel.securityToken.createdAt +
            (UA_DateTime)(entry->channel.securityToken.revisedLifetime * UA_MSEC_TO_DATETIME);
        if(timeout < now || !entry->channel.connection) {
            UA_LOG_DEBUG(cm->server->config.logger, UA_LOGCATEGORY_SECURECHANNEL,
                         "SecureChannel %i has timed out", entry->channel.securityToken.channelId);
            LIST_REMOVE(entry, pointers);
            UA_SecureChannel_deleteMembersCleanup(&entry->channel);
#ifndef UA_ENABLE_MULTITHREADING
            cm->currentChannelCount--;
            UA_free(entry);
#else
            cm->currentChannelCount = uatomic_add_return(&cm->currentChannelCount, -1);
            UA_Server_delayedFree(cm->server, entry);
#endif
        } else if(entry->channel.nextSecurityToken.tokenId > 0) {
            UA_SecureChannel_revolveTokens(&entry->channel);
        }
    }
}

UA_StatusCode
UA_SecureChannelManager_open(UA_SecureChannelManager *cm, UA_Connection *conn,
                             const UA_OpenSecureChannelRequest *request,
                             UA_OpenSecureChannelResponse *response) {
    if(request->securityMode != UA_MESSAGESECURITYMODE_NONE)
        return UA_STATUSCODE_BADSECURITYMODEREJECTED;
    if(cm->currentChannelCount >= cm->maxChannelCount)
        return UA_STATUSCODE_BADOUTOFMEMORY;
    channel_list_entry *entry = UA_malloc(sizeof(channel_list_entry));
    if(!entry)
        return UA_STATUSCODE_BADOUTOFMEMORY;
#ifndef UA_ENABLE_MULTITHREADING
    cm->currentChannelCount++;
#else
    cm->currentChannelCount = uatomic_add_return(&cm->currentChannelCount, 1);
#endif

    UA_SecureChannel_init(&entry->channel);
    response->responseHeader.stringTableSize = 0;
    response->responseHeader.timestamp = UA_DateTime_now();
    response->serverProtocolVersion = 0;

    entry->channel.securityToken.channelId = cm->lastChannelId++;
    entry->channel.securityToken.tokenId = cm->lastTokenId++;
    entry->channel.securityToken.createdAt = UA_DateTime_now();
    entry->channel.securityToken.revisedLifetime =
            (request->requestedLifetime > cm->maxChannelLifetime) ?
                    cm->maxChannelLifetime : request->requestedLifetime;
    /* pragmatic workaround to get clients requesting lifetime of 0 working */
    if(entry->channel.securityToken.revisedLifetime == 0)
        entry->channel.securityToken.revisedLifetime = cm->maxChannelLifetime;

    UA_ByteString_copy(&request->clientNonce, &entry->channel.clientNonce);
    entry->channel.serverAsymAlgSettings.securityPolicyUri = UA_STRING_ALLOC(
            "http://opcfoundation.org/UA/SecurityPolicy#None");

    UA_SecureChannel_generateNonce(&entry->channel.serverNonce);
    UA_ByteString_copy(&entry->channel.serverNonce, &response->serverNonce);
    UA_ChannelSecurityToken_copy(&entry->channel.securityToken,
            &response->securityToken);

    UA_Connection_attachSecureChannel(conn, &entry->channel);
    LIST_INSERT_HEAD(&cm->channels, entry, pointers);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
UA_SecureChannelManager_renew(UA_SecureChannelManager *cm, UA_Connection *conn,
                              const UA_OpenSecureChannelRequest *request,
                              UA_OpenSecureChannelResponse *response) {
    UA_SecureChannel *channel = conn->channel;
    if(!channel)
        return UA_STATUSCODE_BADINTERNALERROR;

    /* if no security token is already issued */
    if(channel->nextSecurityToken.tokenId == 0) {
        channel->nextSecurityToken.channelId = channel->securityToken.channelId;
        //FIXME: UaExpert seems not to use the new tokenid
        channel->nextSecurityToken.tokenId = cm->lastTokenId++;
        //channel->nextSecurityToken.tokenId = channel->securityToken.tokenId;
        channel->nextSecurityToken.createdAt = UA_DateTime_now();
        channel->nextSecurityToken.revisedLifetime =
                (request->requestedLifetime > cm->maxChannelLifetime) ?
                        cm->maxChannelLifetime : request->requestedLifetime;

        /* pragmatic workaround to get clients requesting lifetime of 0 working */
        if(channel->nextSecurityToken.revisedLifetime == 0)
            channel->nextSecurityToken.revisedLifetime = cm->maxChannelLifetime;
    }

    if(channel->clientNonce.data)
        UA_ByteString_deleteMembers(&channel->clientNonce);

    UA_ByteString_copy(&request->clientNonce, &channel->clientNonce);
    UA_ByteString_copy(&channel->serverNonce, &response->serverNonce);
    UA_ChannelSecurityToken_copy(&channel->nextSecurityToken, &response->securityToken);
    return UA_STATUSCODE_GOOD;
}

UA_SecureChannel * UA_SecureChannelManager_get(UA_SecureChannelManager *cm, UA_UInt32 channelId) {
    channel_list_entry *entry;
    LIST_FOREACH(entry, &cm->channels, pointers) {
        if(entry->channel.securityToken.channelId == channelId)
            return &entry->channel;
    }
    return NULL;
}

UA_StatusCode UA_SecureChannelManager_close(UA_SecureChannelManager *cm, UA_UInt32 channelId) {
    channel_list_entry *entry;
    LIST_FOREACH(entry, &cm->channels, pointers) {
        if(entry->channel.securityToken.channelId == channelId) {
            LIST_REMOVE(entry, pointers);
            UA_SecureChannel_deleteMembersCleanup(&entry->channel);
#ifndef UA_ENABLE_MULTITHREADING
            cm->currentChannelCount--;
            UA_free(entry);
#else
            cm->currentChannelCount = uatomic_add_return(&cm->currentChannelCount, -1);
            UA_Server_delayedFree(cm->server, entry);
#endif
            return UA_STATUSCODE_GOOD;
        }
    }
    return UA_STATUSCODE_BADINTERNALERROR;
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_session_manager.c" ***********************************/


UA_StatusCode
UA_SessionManager_init(UA_SessionManager *sm, UA_UInt32 maxSessionCount,
                       UA_UInt32 maxSessionLifeTime, UA_UInt32 startSessionId,
                       UA_Server *server) {
    LIST_INIT(&sm->sessions);
    sm->maxSessionCount = maxSessionCount;
    sm->lastSessionId   = startSessionId;
    sm->maxSessionLifeTime  = maxSessionLifeTime;
    sm->currentSessionCount = 0;
    sm->server = server;
    return UA_STATUSCODE_GOOD;
}

void UA_SessionManager_deleteMembers(UA_SessionManager *sm) {
    session_list_entry *current, *temp;
    LIST_FOREACH_SAFE(current, &sm->sessions, pointers, temp) {
        LIST_REMOVE(current, pointers);
        UA_Session_deleteMembersCleanup(&current->session, sm->server);
        UA_free(current);
    }
}

void UA_SessionManager_cleanupTimedOut(UA_SessionManager *sm, UA_DateTime now) {
    session_list_entry *sentry, *temp;
    LIST_FOREACH_SAFE(sentry, &sm->sessions, pointers, temp) {
        if(sentry->session.validTill < now) {
            UA_LOG_DEBUG(sm->server->config.logger, UA_LOGCATEGORY_SESSION,
                         "Session with token %i has timed out and is removed",
                         sentry->session.sessionId.identifier.numeric);
            LIST_REMOVE(sentry, pointers);
            UA_Session_deleteMembersCleanup(&sentry->session, sm->server);
#ifndef UA_ENABLE_MULTITHREADING
            sm->currentSessionCount--;
            UA_free(sentry);
#else
            sm->currentSessionCount = uatomic_add_return(&sm->currentSessionCount, -1);
            UA_Server_delayedFree(sm->server, sentry);
#endif
        }
    }
}

UA_Session *
UA_SessionManager_getSession(UA_SessionManager *sm, const UA_NodeId *token) {
    session_list_entry *current = NULL;
    LIST_FOREACH(current, &sm->sessions, pointers) {
        if(UA_NodeId_equal(&current->session.authenticationToken, token)) {
            if(UA_DateTime_now() > current->session.validTill) {
                UA_LOG_DEBUG(sm->server->config.logger, UA_LOGCATEGORY_SESSION,
                             "Try to use Session with token %i, but has timed out", token->identifier.numeric);
                return NULL;
            }
            return &current->session;
        }
    }
    UA_LOG_DEBUG(sm->server->config.logger, UA_LOGCATEGORY_SESSION,
                 "Try to use Session with token %i but is not found", token->identifier.numeric);
    return NULL;
}

/** Creates and adds a session. But it is not yet attached to a secure channel. */
UA_StatusCode
UA_SessionManager_createSession(UA_SessionManager *sm, UA_SecureChannel *channel,
                                const UA_CreateSessionRequest *request, UA_Session **session) {
    if(sm->currentSessionCount >= sm->maxSessionCount)
        return UA_STATUSCODE_BADTOOMANYSESSIONS;

    session_list_entry *newentry = UA_malloc(sizeof(session_list_entry));
    if(!newentry)
        return UA_STATUSCODE_BADOUTOFMEMORY;

    sm->currentSessionCount++;
    UA_Session_init(&newentry->session);
    newentry->session.sessionId = UA_NODEID_NUMERIC(1, sm->lastSessionId++);
    newentry->session.authenticationToken = UA_NODEID_GUID(1, UA_Guid_random());

    if(request->requestedSessionTimeout <= sm->maxSessionLifeTime &&
       request->requestedSessionTimeout > 0)
        newentry->session.timeout = request->requestedSessionTimeout;
    else
        newentry->session.timeout = sm->maxSessionLifeTime; // todo: remove when the CTT is fixed

    UA_Session_updateLifetime(&newentry->session);
    LIST_INSERT_HEAD(&sm->sessions, newentry, pointers);
    *session = &newentry->session;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
UA_SessionManager_removeSession(UA_SessionManager *sm, const UA_NodeId *token) {
    session_list_entry *current;
    LIST_FOREACH(current, &sm->sessions, pointers) {
        if(UA_NodeId_equal(&current->session.authenticationToken, token))
            break;
    }

    if(!current)
        return UA_STATUSCODE_BADSESSIONIDINVALID;

    LIST_REMOVE(current, pointers);
    UA_Session_deleteMembersCleanup(&current->session, sm->server);
#ifndef UA_ENABLE_MULTITHREADING
    sm->currentSessionCount--;
    UA_free(current);
#else
    sm->currentSessionCount = uatomic_add_return(&sm->currentSessionCount, -1);
    UA_Server_delayedFree(sm->server, current);
#endif
    return UA_STATUSCODE_GOOD;
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_services_discovery.c" ***********************************/


void Service_FindServers(UA_Server *server, UA_Session *session,
                         const UA_FindServersRequest *request, UA_FindServersResponse *response) {
    /* copy ApplicationDescription from the config */
    UA_ApplicationDescription *descr = UA_malloc(sizeof(UA_ApplicationDescription));
    if(!descr) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }
    response->responseHeader.serviceResult =
        UA_ApplicationDescription_copy(&server->config.applicationDescription, descr);
    if(response->responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
        UA_free(descr);
        return;
    }

    /* add the discoveryUrls from the networklayers */
    UA_String *disc = UA_realloc(descr->discoveryUrls, sizeof(UA_String) *
                                 (descr->discoveryUrlsSize + server->config.networkLayersSize));
    if(!disc) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        UA_ApplicationDescription_delete(descr);
        return;
    }
    size_t existing = descr->discoveryUrlsSize;
    descr->discoveryUrls = disc;
    descr->discoveryUrlsSize += server->config.networkLayersSize;
        
    // TODO: Add nl only if discoveryUrl not already present
    for(size_t i = 0; i < server->config.networkLayersSize; i++) {
        UA_ServerNetworkLayer *nl = &server->config.networkLayers[i];
        UA_String_copy(&nl->discoveryUrl, &descr->discoveryUrls[existing + i]);
    }

    response->servers = descr;
    response->serversSize = 1;
}

void Service_GetEndpoints(UA_Server *server, UA_Session *session, const UA_GetEndpointsRequest *request,
                          UA_GetEndpointsResponse *response) {
    /* Test if one of the networklayers exposes the discoveryUrl of the requested endpoint */
    /* Disabled, servers in a virtualbox don't know their external hostname */
    /* UA_Boolean foundUri = false; */
    /* for(size_t i = 0; i < server->config.networkLayersSize; i++) { */
    /*     if(UA_String_equal(&request->endpointUrl, &server->config.networkLayers[i].discoveryUrl)) { */
    /*         foundUri = true; */
    /*         break; */
    /*     } */
    /* } */
    /* if(!foundUri) { */
    /*     response->endpointsSize = 0; */
    /*     return; */
    /* } */
    
    /* test if the supported binary profile shall be returned */
#ifdef NO_ALLOCA
	UA_Boolean relevant_endpoints[server->endpointDescriptionsSize];
#else
	UA_Boolean *relevant_endpoints = UA_alloca(sizeof(UA_Byte) * server->endpointDescriptionsSize);
#endif
    size_t relevant_count = 0;
    for(size_t j = 0; j < server->endpointDescriptionsSize; j++) {
        relevant_endpoints[j] = false;
        if(request->profileUrisSize == 0) {
            relevant_endpoints[j] = true;
            relevant_count++;
            continue;
        }
        for(size_t i = 0; i < request->profileUrisSize; i++) {
            if(UA_String_equal(&request->profileUris[i], &server->endpointDescriptions->transportProfileUri)) {
                relevant_endpoints[j] = true;
                relevant_count++;
                break;
            }
        }
    }

    if(relevant_count == 0) {
        response->endpointsSize = 0;
        return;
    }

    response->endpoints = UA_malloc(sizeof(UA_EndpointDescription) * relevant_count);
    if(!response->endpoints) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }

    size_t k = 0;
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    for(size_t j = 0; j < server->endpointDescriptionsSize && retval == UA_STATUSCODE_GOOD; j++) {
        if(!relevant_endpoints[j])
            continue;
        retval = UA_EndpointDescription_copy(&server->endpointDescriptions[j], &response->endpoints[k]);
        if(retval != UA_STATUSCODE_GOOD)
            break;
        UA_String_deleteMembers(&response->endpoints[k].endpointUrl);
        retval = UA_String_copy(&request->endpointUrl, &response->endpoints[k].endpointUrl);
        k++;
    }

    if(retval != UA_STATUSCODE_GOOD) {
        response->responseHeader.serviceResult = retval;
        UA_Array_delete(response->endpoints, --k, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
        return;
    }
    response->endpointsSize = relevant_count;
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_services_securechannel.c" ***********************************/


void Service_OpenSecureChannel(UA_Server *server, UA_Connection *connection,
                               const UA_OpenSecureChannelRequest *request,
                               UA_OpenSecureChannelResponse *response) {
    // todo: if(request->clientProtocolVersion != protocolVersion)
    if(request->requestType == UA_SECURITYTOKENREQUESTTYPE_ISSUE) {
        response->responseHeader.serviceResult =
            UA_SecureChannelManager_open(&server->secureChannelManager, connection, request, response);

        if(response->responseHeader.serviceResult == UA_STATUSCODE_GOOD)
            UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SECURECHANNEL,
                         "Opened SecureChannel %i on Connection %i",
                         response->securityToken.channelId, connection->sockfd);
        else
            UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SECURECHANNEL,
                         "Opening SecureChannel on Connection %i failed", connection->sockfd);
    } else {
        response->responseHeader.serviceResult =
            UA_SecureChannelManager_renew(&server->secureChannelManager, connection, request, response);

        if(response->responseHeader.serviceResult == UA_STATUSCODE_GOOD)
            UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SECURECHANNEL,
                         "Renewed SecureChannel %i on Connection %i",
                         response->securityToken.channelId, connection->sockfd);
        else
            UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SECURECHANNEL,
                         "Renewing SecureChannel on Connection %i failed", connection->sockfd);
    }
}

/* The server does not send a CloseSecureChannel response */
void Service_CloseSecureChannel(UA_Server *server, UA_UInt32 channelId) {
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SECURECHANNEL,
                 "Closing SecureChannel %i", channelId);
    UA_SecureChannelManager_close(&server->secureChannelManager, channelId);
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_services_session.c" ***********************************/


void Service_CreateSession(UA_Server *server, UA_Session *session, const UA_CreateSessionRequest *request,
                           UA_CreateSessionResponse *response) {
    UA_SecureChannel *channel = session->channel;
    if(channel->securityToken.channelId == 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADSECURECHANNELIDINVALID;
        return;
    }
    response->responseHeader.serviceResult =
        UA_Array_copy(server->endpointDescriptions, server->endpointDescriptionsSize,
                      (void**)&response->serverEndpoints, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
    if(response->responseHeader.serviceResult != UA_STATUSCODE_GOOD)
        return;
    response->serverEndpointsSize = server->endpointDescriptionsSize;

	UA_Session *newSession;
    response->responseHeader.serviceResult =
        UA_SessionManager_createSession(&server->sessionManager, channel, request, &newSession);
	if(response->responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
        UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SESSION,
                     "Processing CreateSessionRequest on SecureChannel %i failed",
                     channel->securityToken.channelId);
		return;
    }

    //TODO get maxResponseMessageSize internally
    newSession->maxResponseMessageSize = request->maxResponseMessageSize;
    response->sessionId = newSession->sessionId;
    response->revisedSessionTimeout = (UA_Double)newSession->timeout;
    response->authenticationToken = newSession->authenticationToken;
    response->responseHeader.serviceResult = UA_String_copy(&request->sessionName, &newSession->sessionName);
    if(server->endpointDescriptions)
        response->responseHeader.serviceResult |=
            UA_ByteString_copy(&server->endpointDescriptions->serverCertificate, &response->serverCertificate);
    if(response->responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
        UA_SessionManager_removeSession(&server->sessionManager, &newSession->authenticationToken);
         return;
    }
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SESSION,
                 "Processing CreateSessionRequest on SecureChannel %i succeeded, created Session (ns=%i,i=%i)",
                 channel->securityToken.channelId, response->sessionId.namespaceIndex,
                 response->sessionId.identifier.numeric);
}

void
Service_ActivateSession(UA_Server *server, UA_Session *session, const UA_ActivateSessionRequest *request,
                        UA_ActivateSessionResponse *response) {
    UA_SecureChannel *channel = session->channel;
    // make the channel know about the session
	UA_Session *foundSession =
        UA_SessionManager_getSession(&server->sessionManager, &request->requestHeader.authenticationToken);

	if(!foundSession) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADSESSIONIDINVALID;
        UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SESSION,
                     "Processing ActivateSessionRequest on SecureChannel %i, "
                     "but no session found for the authentication token",
                     channel->securityToken.channelId);
        return;
	}

    if(foundSession->validTill < UA_DateTime_now()) {
        UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SESSION,
                     "Processing ActivateSessionRequest on SecureChannel %i, but the session has timed out",
                     channel->securityToken.channelId);
        response->responseHeader.serviceResult = UA_STATUSCODE_BADSESSIONIDINVALID;
        return;
	}

    if(request->userIdentityToken.encoding < UA_EXTENSIONOBJECT_DECODED ||
       (request->userIdentityToken.content.decoded.type != &UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN] &&
        request->userIdentityToken.content.decoded.type != &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN])) {
        UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SESSION,
                     "Invalided UserIdentityToken on SecureChannel %i for Session (ns=%i,i=%i)",
                     channel->securityToken.channelId, foundSession->sessionId.namespaceIndex,
                     foundSession->sessionId.identifier.numeric);
        response->responseHeader.serviceResult = UA_STATUSCODE_BADINTERNALERROR;
        return;
    }

    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SESSION,
                 "Processing ActivateSessionRequest on SecureChannel %i for Session (ns=%i,i=%i)",
                 channel->securityToken.channelId, foundSession->sessionId.namespaceIndex,
                 foundSession->sessionId.identifier.numeric);

    UA_String ap = UA_STRING(ANONYMOUS_POLICY);
    UA_String up = UA_STRING(USERNAME_POLICY);

    /* Compatibility notice: Siemens OPC Scout v10 provides an empty policyId,
       this is not okay For compatibility we will assume that empty policyId ==
       ANONYMOUS_POLICY
       if(token.policyId->data == NULL)
           response->responseHeader.serviceResult = UA_STATUSCODE_BADIDENTITYTOKENINVALID;
    */

    /* anonymous login */
    if(server->config.enableAnonymousLogin &&
       request->userIdentityToken.content.decoded.type == &UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN]) {
        const UA_AnonymousIdentityToken *token = request->userIdentityToken.content.decoded.data;
        if(token->policyId.data && !UA_String_equal(&token->policyId, &ap)) {
            response->responseHeader.serviceResult = UA_STATUSCODE_BADIDENTITYTOKENINVALID;
            return;
        }
        if(foundSession->channel && foundSession->channel != channel)
            UA_SecureChannel_detachSession(foundSession->channel, foundSession);
        UA_SecureChannel_attachSession(channel, foundSession);
        foundSession->activated = true;
        UA_Session_updateLifetime(foundSession);
        return;
    }

    /* username login */
    if(server->config.enableUsernamePasswordLogin &&
       request->userIdentityToken.content.decoded.type == &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN]) {
        const UA_UserNameIdentityToken *token = request->userIdentityToken.content.decoded.data;
        if(!UA_String_equal(&token->policyId, &up)) {
            response->responseHeader.serviceResult = UA_STATUSCODE_BADIDENTITYTOKENINVALID;
            return;
        }
        if(token->encryptionAlgorithm.length > 0) {
            /* we don't support encryption */
            response->responseHeader.serviceResult = UA_STATUSCODE_BADIDENTITYTOKENINVALID;
            return;
        }
        /* ok, trying to match the username */
        for(size_t i = 0; i < server->config.usernamePasswordLoginsSize; i++) {
            UA_String *user = &server->config.usernamePasswordLogins[i].username;
            UA_String *pw = &server->config.usernamePasswordLogins[i].password;
            if(!UA_String_equal(&token->userName, user) || !UA_String_equal(&token->password, pw))
                continue;
            /* success - activate */
            if(foundSession->channel && foundSession->channel != channel)
                UA_SecureChannel_detachSession(foundSession->channel, foundSession);
            UA_SecureChannel_attachSession(channel, foundSession);
            foundSession->activated = true;
            UA_Session_updateLifetime(foundSession);
            return;
        }
        /* no match */
        response->responseHeader.serviceResult = UA_STATUSCODE_BADUSERACCESSDENIED;
        return;
    }
    response->responseHeader.serviceResult = UA_STATUSCODE_BADIDENTITYTOKENINVALID;
}

void
Service_CloseSession(UA_Server *server, UA_Session *session, const UA_CloseSessionRequest *request,
                     UA_CloseSessionResponse *response) {
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SESSION,
                 "Processing CloseSessionRequest for Session (ns=%i,i=%i)",
                 session->sessionId.namespaceIndex, session->sessionId.identifier.numeric);
    response->responseHeader.serviceResult =
        UA_SessionManager_removeSession(&server->sessionManager, &session->authenticationToken);
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_services_attribute.c" ***********************************/


/******************/
/* Read Attribute */
/******************/

static size_t
readNumber(UA_Byte *buf, size_t buflen, UA_UInt32 *number) {
    UA_UInt32 n = 0;
    size_t progress = 0;
    /* read numbers until the end or a non-number character appears */
    while(progress < buflen) {
        UA_Byte c = buf[progress];
        if('0' > c || '9' < c)
            break;
        n = (n*10) + (UA_UInt32)(c-'0');
        progress++;
    }
    *number = n;
    return progress;
}

static size_t
readDimension(UA_Byte *buf, size_t buflen, struct UA_NumericRangeDimension *dim) {
    size_t progress = readNumber(buf, buflen, &dim->min);
    if(progress == 0)
        return 0;
    if(buflen <= progress || buf[progress] != ':') {
        dim->max = dim->min;
        return progress;
    }
    progress++;
    size_t progress2 = readNumber(&buf[progress], buflen - progress, &dim->max);
    if(progress2 == 0)
        return 0;
    return progress + progress2;
}

#ifndef UA_BUILD_UNIT_TESTS
static
#endif
UA_StatusCode parse_numericrange(const UA_String *str, UA_NumericRange *range) {
    size_t idx = 0;
    size_t dimensionsMax = 0;
    struct UA_NumericRangeDimension *dimensions = NULL;
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    size_t pos = 0;
    do {
        /* alloc dimensions */
        if(idx >= dimensionsMax) {
            struct UA_NumericRangeDimension *newds;
            newds = UA_realloc(dimensions, sizeof(struct UA_NumericRangeDimension) * (dimensionsMax + 2));
            if(!newds) {
                retval = UA_STATUSCODE_BADOUTOFMEMORY;
                break;
            }
            dimensions = newds;
            dimensionsMax = dimensionsMax + 2;
        }

        /* read the dimension */
        size_t progress = readDimension(&str->data[pos], str->length - pos, &dimensions[idx]);
        if(progress == 0) {
            retval = UA_STATUSCODE_BADINDEXRANGEINVALID;
            break;
        }
        pos += progress;
        idx++;

        /* loop into the next dimension */
        if(pos >= str->length)
            break;
    } while(str->data[pos] == ',' && pos++);

    if(retval == UA_STATUSCODE_GOOD && idx > 0) {
        range->dimensions = dimensions;
        range->dimensionsSize = idx;
    } else
        UA_free(dimensions);

    return retval;
}

#define CHECK_NODECLASS(CLASS)                                  \
    if(!(node->nodeClass & (CLASS))) {                          \
        retval = UA_STATUSCODE_BADATTRIBUTEIDINVALID;           \
        break;                                                  \
    }

static void handleServerTimestamps(UA_TimestampsToReturn timestamps, UA_DataValue* v) {
	if(v && (timestamps == UA_TIMESTAMPSTORETURN_SERVER || timestamps == UA_TIMESTAMPSTORETURN_BOTH)) {
		v->hasServerTimestamp = true;
		v->serverTimestamp = UA_DateTime_now();
	}
}

static void handleSourceTimestamps(UA_TimestampsToReturn timestamps, UA_DataValue* v) {
	if(timestamps == UA_TIMESTAMPSTORETURN_SOURCE || timestamps == UA_TIMESTAMPSTORETURN_BOTH) {
		v->hasSourceTimestamp = true;
		v->sourceTimestamp = UA_DateTime_now();
	}
}

/* force cast for zero-copy reading. ensure that the variant is never written into. */
static void forceVariantSetScalar(UA_Variant *v, const void *p, const UA_DataType *t) {
    UA_Variant_init(v);
    v->type = t;
    v->data = (void*)(uintptr_t)p;
    v->storageType = UA_VARIANT_DATA_NODELETE;
}

static UA_StatusCode getVariableNodeValue(const UA_VariableNode *vn, const UA_TimestampsToReturn timestamps,
                                          const UA_ReadValueId *id, UA_DataValue *v) {
    UA_NumericRange range;
    UA_NumericRange *rangeptr = NULL;
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    if(id->indexRange.length > 0) {
        retval = parse_numericrange(&id->indexRange, &range);
        if(retval != UA_STATUSCODE_GOOD)
            return retval;
        rangeptr = &range;
    }

    if(vn->valueSource == UA_VALUESOURCE_VARIANT) {
        if(vn->value.variant.callback.onRead)
            vn->value.variant.callback.onRead(vn->value.variant.callback.handle, vn->nodeId,
                                              &v->value, rangeptr);
        if(!rangeptr) {
            v->value = vn->value.variant.value;
            v->value.storageType = UA_VARIANT_DATA_NODELETE;
        } else
            retval = UA_Variant_copyRange(&vn->value.variant.value, &v->value, range);
        if(retval == UA_STATUSCODE_GOOD)
            handleSourceTimestamps(timestamps, v);
    } else {
        if(vn->value.dataSource.read == NULL) {
            retval = UA_STATUSCODE_BADINTERNALERROR;
        } else {
            UA_Boolean sourceTimeStamp = (timestamps == UA_TIMESTAMPSTORETURN_SOURCE ||
                                          timestamps == UA_TIMESTAMPSTORETURN_BOTH);
            retval = vn->value.dataSource.read(vn->value.dataSource.handle, vn->nodeId,
                                               sourceTimeStamp, rangeptr, v);
        }
    }

    if(rangeptr)
        UA_free(range.dimensions);
    return retval;
}

static UA_StatusCode getVariableNodeDataType(const UA_VariableNode *vn, UA_DataValue *v) {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    if(vn->valueSource == UA_VALUESOURCE_VARIANT) {
        forceVariantSetScalar(&v->value, &vn->value.variant.value.type->typeId,
                              &UA_TYPES[UA_TYPES_NODEID]);
    } else {
        if(vn->value.dataSource.read == NULL)
            return UA_STATUSCODE_BADINTERNALERROR;
        /* Read from the datasource to see the data type */
        UA_DataValue val;
        UA_DataValue_init(&val);
        val.hasValue = false; // always assume we are not given a value by userspace
        retval = vn->value.dataSource.read(vn->value.dataSource.handle, vn->nodeId, false, NULL, &val);
        if(retval != UA_STATUSCODE_GOOD)
            return retval;
        if (val.hasValue && val.value.type != NULL)
          retval = UA_Variant_setScalarCopy(&v->value, &val.value.type->typeId, &UA_TYPES[UA_TYPES_NODEID]);
        UA_DataValue_deleteMembers(&val);
    }
    return retval;
}

static UA_StatusCode getVariableNodeArrayDimensions(const UA_VariableNode *vn, UA_DataValue *v) {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    if(vn->valueSource == UA_VALUESOURCE_VARIANT) {
        UA_Variant_setArray(&v->value, vn->value.variant.value.arrayDimensions,
                            vn->value.variant.value.arrayDimensionsSize, &UA_TYPES[UA_TYPES_INT32]);
        v->value.storageType = UA_VARIANT_DATA_NODELETE;
    } else {
        if(vn->value.dataSource.read == NULL)
            return UA_STATUSCODE_BADINTERNALERROR;
        /* Read the datasource to see the array dimensions */
        UA_DataValue val;
        UA_DataValue_init(&val);
        retval = vn->value.dataSource.read(vn->value.dataSource.handle, vn->nodeId, false, NULL, &val);
        if(retval != UA_STATUSCODE_GOOD)
            return retval;
        retval = UA_Variant_setArrayCopy(&v->value, val.value.arrayDimensions,
                                         val.value.arrayDimensionsSize, &UA_TYPES[UA_TYPES_INT32]);
        UA_DataValue_deleteMembers(&val);
    }
    return retval;
}

static const UA_String binEncoding = {sizeof("DefaultBinary")-1, (UA_Byte*)"DefaultBinary"};
/* clang complains about unused variables */
// static const UA_String xmlEncoding = {sizeof("DefaultXml")-1, (UA_Byte*)"DefaultXml"};

/** Reads a single attribute from a node in the nodestore. */
void Service_Read_single(UA_Server *server, UA_Session *session, const UA_TimestampsToReturn timestamps,
                         const UA_ReadValueId *id, UA_DataValue *v) {
	if(id->dataEncoding.name.length > 0 && !UA_String_equal(&binEncoding, &id->dataEncoding.name)) {
           v->hasStatus = true;
           v->status = UA_STATUSCODE_BADDATAENCODINGUNSUPPORTED;
           return;
	}

	//index range for a non-value
	if(id->indexRange.length > 0 && id->attributeId != UA_ATTRIBUTEID_VALUE){
		v->hasStatus = true;
		v->status = UA_STATUSCODE_BADINDEXRANGENODATA;
		return;
	}

    UA_Node const *node = UA_NodeStore_get(server->nodestore, &id->nodeId);
    if(!node) {
        v->hasStatus = true;
        v->status = UA_STATUSCODE_BADNODEIDUNKNOWN;
        return;
    }

    /* When setting the value fails in the switch, we get an error code and set hasValue to false */
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    v->hasValue = true;
    switch(id->attributeId) {
    case UA_ATTRIBUTEID_NODEID:
        forceVariantSetScalar(&v->value, &node->nodeId, &UA_TYPES[UA_TYPES_NODEID]);
        break;
    case UA_ATTRIBUTEID_NODECLASS:
        forceVariantSetScalar(&v->value, &node->nodeClass, &UA_TYPES[UA_TYPES_NODECLASS]);
        break;
    case UA_ATTRIBUTEID_BROWSENAME:
        forceVariantSetScalar(&v->value, &node->browseName, &UA_TYPES[UA_TYPES_QUALIFIEDNAME]);
        break;
    case UA_ATTRIBUTEID_DISPLAYNAME:
        forceVariantSetScalar(&v->value, &node->displayName, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
        break;
    case UA_ATTRIBUTEID_DESCRIPTION:
        forceVariantSetScalar(&v->value, &node->description, &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
        break;
    case UA_ATTRIBUTEID_WRITEMASK:
        forceVariantSetScalar(&v->value, &node->writeMask, &UA_TYPES[UA_TYPES_UINT32]);
        break;
    case UA_ATTRIBUTEID_USERWRITEMASK:
        forceVariantSetScalar(&v->value, &node->userWriteMask, &UA_TYPES[UA_TYPES_UINT32]);
        break;
    case UA_ATTRIBUTEID_ISABSTRACT:
        CHECK_NODECLASS(UA_NODECLASS_REFERENCETYPE | UA_NODECLASS_OBJECTTYPE |
                        UA_NODECLASS_VARIABLETYPE | UA_NODECLASS_DATATYPE);
        forceVariantSetScalar(&v->value, &((const UA_ReferenceTypeNode*)node)->isAbstract,
                              &UA_TYPES[UA_TYPES_BOOLEAN]);
        break;
    case UA_ATTRIBUTEID_SYMMETRIC:
        CHECK_NODECLASS(UA_NODECLASS_REFERENCETYPE);
        forceVariantSetScalar(&v->value, &((const UA_ReferenceTypeNode*)node)->symmetric,
                              &UA_TYPES[UA_TYPES_BOOLEAN]);
        break;
    case UA_ATTRIBUTEID_INVERSENAME:
        CHECK_NODECLASS(UA_NODECLASS_REFERENCETYPE);
        forceVariantSetScalar(&v->value, &((const UA_ReferenceTypeNode*)node)->inverseName,
                              &UA_TYPES[UA_TYPES_LOCALIZEDTEXT]);
        break;
    case UA_ATTRIBUTEID_CONTAINSNOLOOPS:
        CHECK_NODECLASS(UA_NODECLASS_VIEW);
        forceVariantSetScalar(&v->value, &((const UA_ViewNode*)node)->containsNoLoops,
                              &UA_TYPES[UA_TYPES_BOOLEAN]);
        break;
    case UA_ATTRIBUTEID_EVENTNOTIFIER:
        CHECK_NODECLASS(UA_NODECLASS_VIEW | UA_NODECLASS_OBJECT);
        forceVariantSetScalar(&v->value, &((const UA_ViewNode*)node)->eventNotifier,
                              &UA_TYPES[UA_TYPES_BYTE]);
        break;
    case UA_ATTRIBUTEID_VALUE:
        CHECK_NODECLASS(UA_NODECLASS_VARIABLE | UA_NODECLASS_VARIABLETYPE);
        retval = getVariableNodeValue((const UA_VariableNode*)node, timestamps, id, v);
        break;
    case UA_ATTRIBUTEID_DATATYPE:
		CHECK_NODECLASS(UA_NODECLASS_VARIABLE | UA_NODECLASS_VARIABLETYPE);
        retval = getVariableNodeDataType((const UA_VariableNode*)node, v);
        break;
    case UA_ATTRIBUTEID_VALUERANK:
        CHECK_NODECLASS(UA_NODECLASS_VARIABLE | UA_NODECLASS_VARIABLETYPE);
        forceVariantSetScalar(&v->value, &((const UA_VariableTypeNode*)node)->valueRank,
                              &UA_TYPES[UA_TYPES_INT32]);
        break;
    case UA_ATTRIBUTEID_ARRAYDIMENSIONS:
        CHECK_NODECLASS(UA_NODECLASS_VARIABLE | UA_NODECLASS_VARIABLETYPE);
        retval = getVariableNodeArrayDimensions((const UA_VariableNode*)node, v);
        break;
    case UA_ATTRIBUTEID_ACCESSLEVEL:
        CHECK_NODECLASS(UA_NODECLASS_VARIABLE);
        forceVariantSetScalar(&v->value, &((const UA_VariableNode*)node)->accessLevel,
                              &UA_TYPES[UA_TYPES_BYTE]);
        break;
    case UA_ATTRIBUTEID_USERACCESSLEVEL:
        CHECK_NODECLASS(UA_NODECLASS_VARIABLE);
        forceVariantSetScalar(&v->value, &((const UA_VariableNode*)node)->userAccessLevel,
                              &UA_TYPES[UA_TYPES_BYTE]);
        break;
    case UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL:
        CHECK_NODECLASS(UA_NODECLASS_VARIABLE);
        forceVariantSetScalar(&v->value, &((const UA_VariableNode*)node)->minimumSamplingInterval,
                              &UA_TYPES[UA_TYPES_DOUBLE]);
        break;
    case UA_ATTRIBUTEID_HISTORIZING:
        CHECK_NODECLASS(UA_NODECLASS_VARIABLE);
        forceVariantSetScalar(&v->value, &((const UA_VariableNode*)node)->historizing,
                              &UA_TYPES[UA_TYPES_BOOLEAN]);
        break;
    case UA_ATTRIBUTEID_EXECUTABLE:
        CHECK_NODECLASS(UA_NODECLASS_METHOD);
        forceVariantSetScalar(&v->value, &((const UA_MethodNode*)node)->executable,
                              &UA_TYPES[UA_TYPES_BOOLEAN]);
        break;
    case UA_ATTRIBUTEID_USEREXECUTABLE:
        CHECK_NODECLASS(UA_NODECLASS_METHOD);
        forceVariantSetScalar(&v->value, &((const UA_MethodNode*)node)->userExecutable,
                              &UA_TYPES[UA_TYPES_BOOLEAN]);
        break;
    default:
        retval = UA_STATUSCODE_BADATTRIBUTEIDINVALID;
        break;
    }

    if(retval != UA_STATUSCODE_GOOD) {
        v->hasValue = false;
        v->hasStatus = true;
        v->status = retval;
    }

    // Todo: what if the timestamp from the datasource are already present?
    handleServerTimestamps(timestamps, v);
}

void Service_Read(UA_Server *server, UA_Session *session, const UA_ReadRequest *request,
                  UA_ReadResponse *response) {
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SESSION,
                 "Processing ReadRequest for Session (ns=%i,i=%i)",
                 session->sessionId.namespaceIndex, session->sessionId.identifier.numeric);
    if(request->nodesToReadSize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }

    if(request->timestampsToReturn > 3){
    	response->responseHeader.serviceResult = UA_STATUSCODE_BADTIMESTAMPSTORETURNINVALID;
    	return;
    }

    size_t size = request->nodesToReadSize;
    response->results = UA_Array_new(size, &UA_TYPES[UA_TYPES_DATAVALUE]);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }

    response->resultsSize = size;
    if(request->maxAge < 0) {
    	response->responseHeader.serviceResult = UA_STATUSCODE_BADMAXAGEINVALID;
        return;
    }

#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
    UA_Boolean isExternal[size];
    UA_UInt32 indices[size];
    memset(isExternal, false, sizeof(UA_Boolean) * size);
    for(size_t j = 0;j<server->externalNamespacesSize;j++) {
        size_t indexSize = 0;
        for(size_t i = 0;i < size;i++) {
            if(request->nodesToRead[i].nodeId.namespaceIndex != server->externalNamespaces[j].index)
                continue;
            isExternal[i] = true;
            indices[indexSize] = i;
            indexSize++;
        }
        if(indexSize == 0)
            continue;
        UA_ExternalNodeStore *ens = &server->externalNamespaces[j].externalNodeStore;
        ens->readNodes(ens->ensHandle, &request->requestHeader, request->nodesToRead,
                       indices, indexSize, response->results, false, response->diagnosticInfos);
    }
#endif

    for(size_t i = 0;i < size;i++) {
#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
        if(!isExternal[i])
#endif
            Service_Read_single(server, session, request->timestampsToReturn,
                                &request->nodesToRead[i], &response->results[i]);
    }

#ifdef UA_ENABLE_NONSTANDARD_STATELESS
    /* Add an expiry header for caching */
    if(session->sessionId.namespaceIndex == 0 &&
       session->sessionId.identifierType == UA_NODEIDTYPE_NUMERIC &&
       session->sessionId.identifier.numeric == 0){
        UA_ExtensionObject additionalHeader;
        UA_ExtensionObject_init(&additionalHeader);
        additionalHeader.encoding = UA_EXTENSIONOBJECT_ENCODED_BYTESTRING;
        additionalHeader.content.encoded.typeId =UA_TYPES[UA_TYPES_VARIANT].typeId;

        UA_Variant variant;
        UA_Variant_init(&variant);

        UA_DateTime* expireArray = NULL;
        expireArray = UA_Array_new(request->nodesToReadSize, &UA_TYPES[UA_TYPES_DATETIME]);
        variant.data = expireArray;

        /* expires in 20 seconds */
        for(UA_UInt32 i = 0;i < response->resultsSize;i++) {
            expireArray[i] = UA_DateTime_now() + 20 * 100 * 1000 * 1000;
        }
        UA_Variant_setArray(&variant, expireArray, request->nodesToReadSize, &UA_TYPES[UA_TYPES_DATETIME]);

        size_t offset = 0;
        UA_ByteString str;
        UA_ByteString_allocBuffer(&str, UA_calcSizeBinary(&variant, &UA_TYPES[UA_TYPES_VARIANT]));
        /* No chunking callback for the encoding */
        UA_StatusCode retval = UA_encodeBinary(&variant, &UA_TYPES[UA_TYPES_VARIANT], NULL, NULL, &str, &offset);
        UA_Array_delete(expireArray, request->nodesToReadSize, &UA_TYPES[UA_TYPES_DATETIME]);
        if(retval == UA_STATUSCODE_GOOD){
            additionalHeader.content.encoded.body.data = str.data;
            additionalHeader.content.encoded.body.length = offset;
            response->responseHeader.additionalHeader = additionalHeader;
        }
    }
#endif
}

/*******************/
/* Write Attribute */
/*******************/

UA_StatusCode UA_Server_editNode(UA_Server *server, UA_Session *session, const UA_NodeId *nodeId,
                                 UA_EditNodeCallback callback, const void *data) {
    UA_StatusCode retval;
    do {
#ifndef UA_ENABLE_MULTITHREADING
        const UA_Node *node = UA_NodeStore_get(server->nodestore, nodeId);
        if(!node)
            return UA_STATUSCODE_BADNODEIDUNKNOWN;
        UA_Node *editNode = (UA_Node*)(uintptr_t)node; // dirty cast. use only here.
        retval = callback(server, session, editNode, data);
        return retval;
#else
        UA_Node *copy = UA_NodeStore_getCopy(server->nodestore, nodeId);
        if(!copy)
            return UA_STATUSCODE_BADOUTOFMEMORY;
        retval = callback(server, session, copy, data);
        if(retval != UA_STATUSCODE_GOOD) {
            UA_NodeStore_deleteNode(copy);
            return retval;
        }
        retval = UA_NodeStore_replace(server->nodestore, copy);
#endif
    } while(retval != UA_STATUSCODE_GOOD);
    return UA_STATUSCODE_GOOD;
}

#define CHECK_DATATYPE(EXP_DT)                                          \
    if(!wvalue->value.hasValue ||                                       \
       &UA_TYPES[UA_TYPES_##EXP_DT] != wvalue->value.value.type ||      \
       !UA_Variant_isScalar(&wvalue->value.value)) {                    \
        retval = UA_STATUSCODE_BADTYPEMISMATCH;                         \
        break;                                                          \
    }

#define CHECK_NODECLASS_WRITE(CLASS)                                    \
    if((node->nodeClass & (CLASS)) == 0) {                              \
        retval = UA_STATUSCODE_BADNODECLASSINVALID;                     \
        break;                                                          \
    }

static UA_StatusCode
Service_Write_single_ValueDataSource(UA_Server *server, UA_Session *session, const UA_VariableNode *node,
                                     const UA_WriteValue *wvalue) {
    UA_assert(wvalue->attributeId == UA_ATTRIBUTEID_VALUE);
    UA_assert(node->nodeClass == UA_NODECLASS_VARIABLE || node->nodeClass == UA_NODECLASS_VARIABLETYPE);
    UA_assert(node->valueSource == UA_VALUESOURCE_DATASOURCE);

    if(node->value.dataSource.write == NULL)
        return UA_STATUSCODE_BADWRITENOTSUPPORTED;

    UA_StatusCode retval;
    if(wvalue->indexRange.length <= 0) {
        retval = node->value.dataSource.write(node->value.dataSource.handle, node->nodeId,
                                              &wvalue->value.value, NULL);
    } else {
        UA_NumericRange range;
        retval = parse_numericrange(&wvalue->indexRange, &range);
        if(retval != UA_STATUSCODE_GOOD)
            return retval;
        retval = node->value.dataSource.write(node->value.dataSource.handle, node->nodeId,
                                              &wvalue->value.value, &range);
        UA_free(range.dimensions);
    }
    return retval;
}

enum type_equivalence {
    TYPE_EQUIVALENCE_NONE,
    TYPE_EQUIVALENCE_ENUM,
    TYPE_EQUIVALENCE_OPAQUE
};

static enum type_equivalence typeEquivalence(const UA_DataType *t) {
    if(t->membersSize != 1 || !t->members[0].namespaceZero)
        return TYPE_EQUIVALENCE_NONE;
    if(t->members[0].memberTypeIndex == UA_TYPES_INT32)
        return TYPE_EQUIVALENCE_ENUM;
    if(t->members[0].memberTypeIndex == UA_TYPES_BYTE && t->members[0].isArray)
        return TYPE_EQUIVALENCE_OPAQUE;
    return TYPE_EQUIVALENCE_NONE;
}

static UA_StatusCode
CopyValueIntoNode(UA_VariableNode *node, const UA_WriteValue *wvalue) {
    UA_assert(wvalue->attributeId == UA_ATTRIBUTEID_VALUE);
    UA_assert(node->nodeClass == UA_NODECLASS_VARIABLE || node->nodeClass == UA_NODECLASS_VARIABLETYPE);
    UA_assert(node->valueSource == UA_VALUESOURCE_VARIANT);

    /* Parse the range */
    UA_NumericRange range;
    UA_NumericRange *rangeptr = NULL;
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    if(wvalue->indexRange.length > 0) {
        retval = parse_numericrange(&wvalue->indexRange, &range);
        if(retval != UA_STATUSCODE_GOOD)
            return retval;
        rangeptr = &range;
    }

    /* The nodeid on the wire may be != the nodeid in the node: opaque types, enums and bytestrings.
       nodeV contains the correct type definition. */
    const UA_Variant *newV = &wvalue->value.value;
    UA_Variant *oldV = &node->value.variant.value;
    UA_Variant cast_v;
    if (oldV->type != NULL) { // Don't run NodeId_equal on a NULL pointer (happens if the variable never held a variant)
      if(!UA_NodeId_equal(&oldV->type->typeId, &newV->type->typeId)) {
          cast_v = wvalue->value.value;
          newV = &cast_v;
          enum type_equivalence te1 = typeEquivalence(oldV->type);
          enum type_equivalence te2 = typeEquivalence(newV->type);
          if(te1 != TYPE_EQUIVALENCE_NONE && te1 == te2) {
              /* An enum was sent as an int32, or an opaque type as a bytestring. This is
                detected with the typeIndex indicated the "true" datatype. */
              cast_v.type = oldV->type;
          } else if(oldV->type == &UA_TYPES[UA_TYPES_BYTE] && !UA_Variant_isScalar(oldV) &&
                    newV->type == &UA_TYPES[UA_TYPES_BYTESTRING] && UA_Variant_isScalar(newV)) {
              /* a string is written to a byte array */
              UA_ByteString *str = (UA_ByteString*) newV->data;
              cast_v.arrayLength = str->length;
              cast_v.data = str->data;
              cast_v.type = &UA_TYPES[UA_TYPES_BYTE];
          } else {
              if(rangeptr)
                  UA_free(range.dimensions);
              return UA_STATUSCODE_BADTYPEMISMATCH;
          }
      }
    }
    
    if(!rangeptr) {
        UA_Variant_deleteMembers(&node->value.variant.value);
        UA_Variant_copy(newV, &node->value.variant.value);
    } else
        retval = UA_Variant_setRangeCopy(&node->value.variant.value, newV->data, newV->arrayLength, range);
    if(node->value.variant.callback.onWrite)
        node->value.variant.callback.onWrite(node->value.variant.callback.handle, node->nodeId,
                                             &node->value.variant.value, rangeptr);
    if(rangeptr)
        UA_free(range.dimensions);
    return retval;
}

static UA_StatusCode
CopyAttributeIntoNode(UA_Server *server, UA_Session *session,
                      UA_Node *node, const UA_WriteValue *wvalue) {
    if(!wvalue->value.hasValue)
        return UA_STATUSCODE_BADNODATA;

    void *value = wvalue->value.value.data;
    void *target = NULL;
    const UA_DataType *attr_type = NULL;

    UA_StatusCode retval = UA_STATUSCODE_GOOD;
	switch(wvalue->attributeId) {
    case UA_ATTRIBUTEID_NODEID:
    case UA_ATTRIBUTEID_NODECLASS:
    case UA_ATTRIBUTEID_DATATYPE:
		retval = UA_STATUSCODE_BADWRITENOTSUPPORTED;
		break;
	case UA_ATTRIBUTEID_BROWSENAME:
		CHECK_DATATYPE(QUALIFIEDNAME);
        target = &node->browseName;
        attr_type = &UA_TYPES[UA_TYPES_QUALIFIEDNAME];
		break;
	case UA_ATTRIBUTEID_DISPLAYNAME:
		CHECK_DATATYPE(LOCALIZEDTEXT);
        target = &node->displayName;
        attr_type = &UA_TYPES[UA_TYPES_LOCALIZEDTEXT];
		break;
	case UA_ATTRIBUTEID_DESCRIPTION:
		CHECK_DATATYPE(LOCALIZEDTEXT);
        target = &node->description;
        attr_type = &UA_TYPES[UA_TYPES_LOCALIZEDTEXT];
		break;
	case UA_ATTRIBUTEID_WRITEMASK:
		CHECK_DATATYPE(UINT32);
		node->writeMask = *(UA_UInt32*)value;
		break;
	case UA_ATTRIBUTEID_USERWRITEMASK:
		CHECK_DATATYPE(UINT32);
		node->userWriteMask = *(UA_UInt32*)value;
		break;    
	case UA_ATTRIBUTEID_ISABSTRACT:
		CHECK_NODECLASS_WRITE(UA_NODECLASS_OBJECTTYPE | UA_NODECLASS_REFERENCETYPE |
                              UA_NODECLASS_VARIABLETYPE | UA_NODECLASS_DATATYPE);
		CHECK_DATATYPE(BOOLEAN);
		((UA_ObjectTypeNode*)node)->isAbstract = *(UA_Boolean*)value;
		break;
	case UA_ATTRIBUTEID_SYMMETRIC:
		CHECK_NODECLASS_WRITE(UA_NODECLASS_REFERENCETYPE);
		CHECK_DATATYPE(BOOLEAN);
		((UA_ReferenceTypeNode*)node)->symmetric = *(UA_Boolean*)value;
		break;
	case UA_ATTRIBUTEID_INVERSENAME:
		CHECK_NODECLASS_WRITE(UA_NODECLASS_REFERENCETYPE);
		CHECK_DATATYPE(LOCALIZEDTEXT);
        target = &((UA_ReferenceTypeNode*)node)->inverseName;
        attr_type = &UA_TYPES[UA_TYPES_LOCALIZEDTEXT];
		break;
	case UA_ATTRIBUTEID_CONTAINSNOLOOPS:
		CHECK_NODECLASS_WRITE(UA_NODECLASS_VIEW);
		CHECK_DATATYPE(BOOLEAN);
        ((UA_ViewNode*)node)->containsNoLoops = *(UA_Boolean*)value;
		break;
	case UA_ATTRIBUTEID_EVENTNOTIFIER:
		CHECK_NODECLASS_WRITE(UA_NODECLASS_VIEW | UA_NODECLASS_OBJECT);
		CHECK_DATATYPE(BYTE);
        ((UA_ViewNode*)node)->eventNotifier = *(UA_Byte*)value;
		break;
	case UA_ATTRIBUTEID_VALUE:
		CHECK_NODECLASS_WRITE(UA_NODECLASS_VARIABLE | UA_NODECLASS_VARIABLETYPE);
        if(((const UA_VariableNode*)node)->valueSource == UA_VALUESOURCE_VARIANT)
            retval = CopyValueIntoNode((UA_VariableNode*)node, wvalue);
        else
            retval = Service_Write_single_ValueDataSource(server, session, (const UA_VariableNode*)node, wvalue);
		break;
	case UA_ATTRIBUTEID_ACCESSLEVEL:
		CHECK_NODECLASS_WRITE(UA_NODECLASS_VARIABLE);
		CHECK_DATATYPE(BYTE);
		((UA_VariableNode*)node)->accessLevel = *(UA_Byte*)value;
		break;
	case UA_ATTRIBUTEID_USERACCESSLEVEL:
		CHECK_NODECLASS_WRITE(UA_NODECLASS_VARIABLE);
		CHECK_DATATYPE(BYTE);
		((UA_VariableNode*)node)->userAccessLevel = *(UA_Byte*)value;
		break;
	case UA_ATTRIBUTEID_MINIMUMSAMPLINGINTERVAL:
		CHECK_NODECLASS_WRITE(UA_NODECLASS_VARIABLE);
		CHECK_DATATYPE(DOUBLE);
		((UA_VariableNode*)node)->minimumSamplingInterval = *(UA_Double*)value;
		break;
	case UA_ATTRIBUTEID_HISTORIZING:
		CHECK_NODECLASS_WRITE(UA_NODECLASS_VARIABLE);
		CHECK_DATATYPE(BOOLEAN);
		((UA_VariableNode*)node)->historizing = *(UA_Boolean*)value;
		break;
	case UA_ATTRIBUTEID_EXECUTABLE:
		CHECK_NODECLASS_WRITE(UA_NODECLASS_METHOD);
		CHECK_DATATYPE(BOOLEAN);
		((UA_MethodNode*)node)->executable = *(UA_Boolean*)value;
		break;
	case UA_ATTRIBUTEID_USEREXECUTABLE:
		CHECK_NODECLASS_WRITE(UA_NODECLASS_METHOD);
		CHECK_DATATYPE(BOOLEAN);
		((UA_MethodNode*)node)->userExecutable = *(UA_Boolean*)value;
		break;
	default:
		retval = UA_STATUSCODE_BADATTRIBUTEIDINVALID;
		break;
	}
    if(attr_type) {
        UA_deleteMembers(target, attr_type);
        retval = UA_copy(value, target, attr_type);
    }
    return retval;
}

UA_StatusCode Service_Write_single(UA_Server *server, UA_Session *session, const UA_WriteValue *wvalue) {
    return UA_Server_editNode(server, session, &wvalue->nodeId, (UA_EditNodeCallback)CopyAttributeIntoNode, wvalue);
}

void Service_Write(UA_Server *server, UA_Session *session, const UA_WriteRequest *request,
                   UA_WriteResponse *response) {
    UA_assert(server != NULL && session != NULL && request != NULL && response != NULL);
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SESSION,
                 "Processing WriteRequest for Session (ns=%i,i=%i)",
                 session->sessionId.namespaceIndex, session->sessionId.identifier.numeric);

    if(request->nodesToWriteSize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }

    response->results = UA_Array_new(request->nodesToWriteSize, &UA_TYPES[UA_TYPES_STATUSCODE]);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }

#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
    UA_Boolean isExternal[request->nodesToWriteSize];
    UA_UInt32 indices[request->nodesToWriteSize];
    memset(isExternal, false, sizeof(UA_Boolean)*request->nodesToWriteSize);
    for(size_t j = 0; j < server->externalNamespacesSize; j++) {
        UA_UInt32 indexSize = 0;
        for(size_t i = 0; i < request->nodesToWriteSize; i++) {
            if(request->nodesToWrite[i].nodeId.namespaceIndex !=
               server->externalNamespaces[j].index)
                continue;
            isExternal[i] = true;
            indices[indexSize] = i;
            indexSize++;
        }
        if(indexSize == 0)
            continue;
        UA_ExternalNodeStore *ens = &server->externalNamespaces[j].externalNodeStore;
        ens->writeNodes(ens->ensHandle, &request->requestHeader, request->nodesToWrite,
                        indices, indexSize, response->results, response->diagnosticInfos);
    }
#endif
    
    response->resultsSize = request->nodesToWriteSize;
    for(size_t i = 0;i < request->nodesToWriteSize;i++) {
#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
        if(!isExternal[i])
#endif
		  response->results[i] = Service_Write_single(server, session, &request->nodesToWrite[i]);
    }
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_services_nodemanagement.c" ***********************************/


/************/
/* Add Node */
/************/

void
Service_AddNodes_existing(UA_Server *server, UA_Session *session, UA_Node *node,
                          const UA_NodeId *parentNodeId, const UA_NodeId *referenceTypeId,
                          UA_AddNodesResult *result) {
    if(node->nodeId.namespaceIndex >= server->namespacesSize) {
        result->statusCode = UA_STATUSCODE_BADNODEIDINVALID;
        UA_NodeStore_deleteNode(node);
        return;
    }

    const UA_Node *parent = UA_NodeStore_get(server->nodestore, parentNodeId);
    if(!parent) {
        result->statusCode = UA_STATUSCODE_BADPARENTNODEIDINVALID;
        UA_NodeStore_deleteNode(node);
        return;
    }

    const UA_ReferenceTypeNode *referenceType =
        (const UA_ReferenceTypeNode *)UA_NodeStore_get(server->nodestore, referenceTypeId);
    if(!referenceType) {
        result->statusCode = UA_STATUSCODE_BADREFERENCETYPEIDINVALID;
        UA_NodeStore_deleteNode(node);
        return;
    }

    if(referenceType->nodeClass != UA_NODECLASS_REFERENCETYPE) {
        result->statusCode = UA_STATUSCODE_BADREFERENCETYPEIDINVALID;
        UA_NodeStore_deleteNode(node);
        return;
    }

    if(referenceType->isAbstract == true) {
        result->statusCode = UA_STATUSCODE_BADREFERENCENOTALLOWED;
        UA_NodeStore_deleteNode(node);
        return;
    }

    // todo: test if the referencetype is hierarchical
    // todo: namespace index is assumed to be valid
    result->statusCode = UA_NodeStore_insert(server->nodestore, node);
    if(result->statusCode == UA_STATUSCODE_GOOD)
        result->statusCode = UA_NodeId_copy(&node->nodeId, &result->addedNodeId);
    else
        return;
    
    // reference back to the parent
    UA_AddReferencesItem item;
    UA_AddReferencesItem_init(&item);
    item.sourceNodeId = node->nodeId;
    item.referenceTypeId = *referenceTypeId;
    item.isForward = false;
    item.targetNodeId.nodeId = *parentNodeId;
    Service_AddReferences_single(server, session, &item);
    // todo: error handling. remove new node from nodestore
}

static UA_StatusCode
instantiateVariableNode(UA_Server *server, UA_Session *session,
                        const UA_NodeId *nodeId, const UA_NodeId *typeId, 
                        UA_InstantiationCallback *instantiationCallback);
static UA_StatusCode
instantiateObjectNode(UA_Server *server, UA_Session *session,
                      const UA_NodeId *nodeId, const UA_NodeId *typeId, 
                      UA_InstantiationCallback *instantiationCallback);

/* copy an existing variable under the given parent. then instantiate the
   variable for all hastypedefinitions of the original version. */
static UA_StatusCode
copyExistingVariable(UA_Server *server, UA_Session *session, const UA_NodeId *variable,
                     const UA_NodeId *referenceType, const UA_NodeId *parent,
                     UA_InstantiationCallback *instantiationCallback) {
    const UA_VariableNode *node = (const UA_VariableNode*)UA_NodeStore_get(server->nodestore, variable);
    if(!node)
        return UA_STATUSCODE_BADNODEIDINVALID;
    if(node->nodeClass != UA_NODECLASS_VARIABLE)
        return UA_STATUSCODE_BADNODECLASSINVALID;
    
    // copy the variable attributes
    UA_VariableAttributes attr;
    UA_VariableAttributes_init(&attr);
    UA_LocalizedText_copy(&node->displayName, &attr.displayName);
    UA_LocalizedText_copy(&node->description, &attr.description);
    attr.writeMask = node->writeMask;
    attr.userWriteMask = node->userWriteMask;
    // todo: handle data sources!!!!
    UA_Variant_copy(&node->value.variant.value, &attr.value);
    // datatype is taken from the value
    // valuerank is taken from the value
    // array dimensions are taken from the value
    attr.accessLevel = node->accessLevel;
    attr.userAccessLevel = node->userAccessLevel;
    attr.minimumSamplingInterval = node->minimumSamplingInterval;
    attr.historizing = node->historizing;

    UA_AddNodesItem item;
    UA_AddNodesItem_init(&item);
    UA_NodeId_copy(parent, &item.parentNodeId.nodeId);
    UA_NodeId_copy(referenceType, &item.referenceTypeId);
    UA_QualifiedName_copy(&node->browseName, &item.browseName);
    item.nodeClass = UA_NODECLASS_VARIABLE;
    item.nodeAttributes.encoding = UA_EXTENSIONOBJECT_DECODED_NODELETE;
    item.nodeAttributes.content.decoded.type = &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES];
    item.nodeAttributes.content.decoded.data = &attr;
    // don't add a typedefinition here.

    // add the new variable
    UA_AddNodesResult res;
    UA_AddNodesResult_init(&res);
    Service_AddNodes_single(server, session, &item, &res, instantiationCallback);
    UA_VariableAttributes_deleteMembers(&attr);
    UA_AddNodesItem_deleteMembers(&item);

    // now instantiate the variable for all hastypedefinition references
    for(size_t i = 0; i < node->referencesSize; i++) {
        UA_ReferenceNode *rn = &node->references[i];
        if(rn->isInverse)
            continue;
        const UA_NodeId hasTypeDef = UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION);
        if(!UA_NodeId_equal(&rn->referenceTypeId, &hasTypeDef))
            continue;
        instantiateVariableNode(server, session, &res.addedNodeId, &rn->targetId.nodeId, instantiationCallback);
    }
    
    if (instantiationCallback != NULL)
      instantiationCallback->method(res.addedNodeId, node->nodeId, instantiationCallback->handle);
    
    UA_AddNodesResult_deleteMembers(&res);
    return UA_STATUSCODE_GOOD;
}

/* copy an existing object under the given parent. then instantiate the
   variable for all hastypedefinitions of the original version. */
static UA_StatusCode
copyExistingObject(UA_Server *server, UA_Session *session, const UA_NodeId *variable,
                   const UA_NodeId *referenceType, const UA_NodeId *parent, 
                   UA_InstantiationCallback *instantiationCallback) {
    const UA_ObjectNode *node = (const UA_ObjectNode*)UA_NodeStore_get(server->nodestore, variable);  
    if(!node)
        return UA_STATUSCODE_BADNODEIDINVALID;
    if(node->nodeClass != UA_NODECLASS_OBJECT)
        return UA_STATUSCODE_BADNODECLASSINVALID;
    
    // copy the variable attributes
    UA_ObjectAttributes attr;
    UA_ObjectAttributes_init(&attr);
    UA_LocalizedText_copy(&node->displayName, &attr.displayName);
    UA_LocalizedText_copy(&node->description, &attr.description);
    attr.writeMask = node->writeMask;
    attr.userWriteMask = node->userWriteMask;
    attr.eventNotifier = node->eventNotifier;

    UA_AddNodesItem item;
    UA_AddNodesItem_init(&item);
    UA_NodeId_copy(parent, &item.parentNodeId.nodeId);
    UA_NodeId_copy(referenceType, &item.referenceTypeId);
    UA_QualifiedName_copy(&node->browseName, &item.browseName);
    item.nodeClass = UA_NODECLASS_OBJECT;
    item.nodeAttributes.encoding = UA_EXTENSIONOBJECT_DECODED_NODELETE;
    item.nodeAttributes.content.decoded.type = &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES];
    item.nodeAttributes.content.decoded.data = &attr;
    // don't add a typedefinition here.

    // add the new object
    UA_AddNodesResult res;
    UA_AddNodesResult_init(&res);
    Service_AddNodes_single(server, session, &item, &res, instantiationCallback);
    UA_ObjectAttributes_deleteMembers(&attr);
    UA_AddNodesItem_deleteMembers(&item);

    // now instantiate the object for all hastypedefinition references
    for(size_t i = 0; i < node->referencesSize; i++) {
        UA_ReferenceNode *rn = &node->references[i];
        if(rn->isInverse)
            continue;
        const UA_NodeId hasTypeDef = UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION);
        if(!UA_NodeId_equal(&rn->referenceTypeId, &hasTypeDef))
            continue;
        instantiateObjectNode(server, session, &res.addedNodeId, &rn->targetId.nodeId, instantiationCallback);
    }
    
    if (instantiationCallback != NULL)
      instantiationCallback->method(res.addedNodeId, node->nodeId, instantiationCallback->handle);
    
    UA_AddNodesResult_deleteMembers(&res);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
setObjectInstanceHandle(UA_Server *server, UA_Session *session, UA_ObjectNode* node, void *handle) {
    if(node->nodeClass != UA_NODECLASS_OBJECT)
        return UA_STATUSCODE_BADNODECLASSINVALID;
    node->instanceHandle = handle;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
instantiateObjectNode(UA_Server *server, UA_Session *session,
                      const UA_NodeId *nodeId, const UA_NodeId *typeId, 
                      UA_InstantiationCallback *instantiationCallback) {   
    const UA_ObjectTypeNode *typenode = (const UA_ObjectTypeNode*)UA_NodeStore_get(server->nodestore, typeId);
    if(!typenode)
      return UA_STATUSCODE_BADNODEIDINVALID;
    if(typenode->nodeClass != UA_NODECLASS_OBJECTTYPE)
      return UA_STATUSCODE_BADNODECLASSINVALID;
    
    /* Add all the child nodes */
    UA_BrowseDescription browseChildren;
    UA_BrowseDescription_init(&browseChildren);
    browseChildren.nodeId = *typeId;
    browseChildren.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_AGGREGATES);
    browseChildren.includeSubtypes = true;
    browseChildren.browseDirection = UA_BROWSEDIRECTION_FORWARD;
    browseChildren.nodeClassMask = UA_NODECLASS_OBJECT | UA_NODECLASS_VARIABLE | UA_NODECLASS_METHOD;
    browseChildren.resultMask = UA_BROWSERESULTMASK_REFERENCETYPEID | UA_BROWSERESULTMASK_NODECLASS;

    UA_BrowseResult browseResult;
    UA_BrowseResult_init(&browseResult);
    // todo: continuation points if there are too many results
    Service_Browse_single(server, session, NULL, &browseChildren, 100, &browseResult);

    for(size_t i = 0; i < browseResult.referencesSize; i++) {
        UA_ReferenceDescription *rd = &browseResult.references[i];
        if(rd->nodeClass == UA_NODECLASS_METHOD) {
            /* add a reference to the method in the objecttype */
            UA_AddReferencesItem item;
            UA_AddReferencesItem_init(&item);
            item.sourceNodeId = *nodeId;
            item.referenceTypeId = rd->referenceTypeId;
            item.isForward = true;
            item.targetNodeId = rd->nodeId;
            item.targetNodeClass = UA_NODECLASS_METHOD;
            Service_AddReferences_single(server, session, &item);
        } else if(rd->nodeClass == UA_NODECLASS_VARIABLE)
          copyExistingVariable(server, session, &rd->nodeId.nodeId,
                               &rd->referenceTypeId, nodeId, instantiationCallback);
        else if(rd->nodeClass == UA_NODECLASS_OBJECT)
          copyExistingObject(server, session, &rd->nodeId.nodeId,
                             &rd->referenceTypeId, nodeId, instantiationCallback);
    }

    /* add a hastypedefinition reference */
    UA_AddReferencesItem addref;
    UA_AddReferencesItem_init(&addref);
    addref.sourceNodeId = *nodeId;
    addref.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION);
    addref.isForward = true;
    addref.targetNodeId.nodeId = *typeId;
    addref.targetNodeClass = UA_NODECLASS_OBJECTTYPE;
    Service_AddReferences_single(server, session, &addref);

    /* call the constructor */
    const UA_ObjectLifecycleManagement *olm = &typenode->lifecycleManagement;
    if(olm->constructor)
        UA_Server_editNode(server, session, nodeId,
                           (UA_EditNodeCallback)setObjectInstanceHandle, olm->constructor(*nodeId));
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
instantiateVariableNode(UA_Server *server, UA_Session *session, const UA_NodeId *nodeId,
    const UA_NodeId *typeId, UA_InstantiationCallback *instantiationCallback) {
    const UA_ObjectTypeNode *typenode = (const UA_ObjectTypeNode*)UA_NodeStore_get(server->nodestore, typeId);
    if(!typenode)
        return UA_STATUSCODE_BADNODEIDINVALID;
    if(typenode->nodeClass != UA_NODECLASS_VARIABLETYPE)
        return UA_STATUSCODE_BADNODECLASSINVALID;
    
    /* get the references to child properties */
    UA_BrowseDescription browseChildren;
    UA_BrowseDescription_init(&browseChildren);
    browseChildren.nodeId = *typeId;
    browseChildren.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY);
    browseChildren.includeSubtypes = true;
    browseChildren.browseDirection = UA_BROWSEDIRECTION_FORWARD;
    browseChildren.nodeClassMask = UA_NODECLASS_VARIABLE;
    browseChildren.resultMask = UA_BROWSERESULTMASK_REFERENCETYPEID | UA_BROWSERESULTMASK_NODECLASS;

    UA_BrowseResult browseResult;
    UA_BrowseResult_init(&browseResult);
    // todo: continuation points if there are too many results
    Service_Browse_single(server, session, NULL, &browseChildren, 100, &browseResult);

    /* add the child properties */
    for(size_t i = 0; i < browseResult.referencesSize; i++) {
        UA_ReferenceDescription *rd = &browseResult.references[i];
        copyExistingVariable(server, session, &rd->nodeId.nodeId,
                             &rd->referenceTypeId, nodeId, instantiationCallback);
    }

    /* add a hastypedefinition reference */
    UA_AddReferencesItem addref;
    UA_AddReferencesItem_init(&addref);
    addref.sourceNodeId = *nodeId;
    addref.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASTYPEDEFINITION);
    addref.isForward = true;
    addref.targetNodeId.nodeId = *typeId;
    addref.targetNodeClass = UA_NODECLASS_OBJECTTYPE;
    Service_AddReferences_single(server, session, &addref);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
copyStandardAttributes(UA_Node *node, const UA_AddNodesItem *item, const UA_NodeAttributes *attr) {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    retval |= UA_NodeId_copy(&item->requestedNewNodeId.nodeId, &node->nodeId);
    retval |= UA_QualifiedName_copy(&item->browseName, &node->browseName);
    retval |= UA_LocalizedText_copy(&attr->displayName, &node->displayName);
    retval |= UA_LocalizedText_copy(&attr->description, &node->description);
    node->writeMask = attr->writeMask;
    node->userWriteMask = attr->userWriteMask;
    return retval;
}

static UA_Node *
variableNodeFromAttributes(const UA_AddNodesItem *item, const UA_VariableAttributes *attr) {
    UA_VariableNode *vnode = UA_NodeStore_newVariableNode();
    if(!vnode)
        return NULL;
    UA_StatusCode retval = copyStandardAttributes((UA_Node*)vnode, item, (const UA_NodeAttributes*)attr);
    // todo: test if the type / valueRank / value attributes are consistent
    vnode->accessLevel = attr->accessLevel;
    vnode->userAccessLevel = attr->userAccessLevel;
    vnode->historizing = attr->historizing;
    vnode->minimumSamplingInterval = attr->minimumSamplingInterval;
    vnode->valueRank = attr->valueRank;
    retval |= UA_Variant_copy(&attr->value, &vnode->value.variant.value);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_NodeStore_deleteNode((UA_Node*)vnode);
        return NULL;
    }
    return (UA_Node*)vnode;
}

static UA_Node *
objectNodeFromAttributes(const UA_AddNodesItem *item, const UA_ObjectAttributes *attr) {
    UA_ObjectNode *onode = UA_NodeStore_newObjectNode();
    if(!onode)
        return NULL;
    UA_StatusCode retval = copyStandardAttributes((UA_Node*)onode, item, (const UA_NodeAttributes*)attr);
    onode->eventNotifier = attr->eventNotifier;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_NodeStore_deleteNode((UA_Node*)onode);
        return NULL;
    }
    return (UA_Node*)onode;
}

static UA_Node *
referenceTypeNodeFromAttributes(const UA_AddNodesItem *item, const UA_ReferenceTypeAttributes *attr) {
    UA_ReferenceTypeNode *rtnode = UA_NodeStore_newReferenceTypeNode();
    if(!rtnode)
        return NULL;
    UA_StatusCode retval = copyStandardAttributes((UA_Node*)rtnode, item, (const UA_NodeAttributes*)attr);
    rtnode->isAbstract = attr->isAbstract;
    rtnode->symmetric = attr->symmetric;
    retval |= UA_LocalizedText_copy(&attr->inverseName, &rtnode->inverseName);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_NodeStore_deleteNode((UA_Node*)rtnode);
        return NULL;
    }
    return (UA_Node*)rtnode;
}

static UA_Node *
objectTypeNodeFromAttributes(const UA_AddNodesItem *item, const UA_ObjectTypeAttributes *attr) {
    UA_ObjectTypeNode *otnode = UA_NodeStore_newObjectTypeNode();
    if(!otnode)
        return NULL;
    UA_StatusCode retval = copyStandardAttributes((UA_Node*)otnode, item, (const UA_NodeAttributes*)attr);
    otnode->isAbstract = attr->isAbstract;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_NodeStore_deleteNode((UA_Node*)otnode);
        return NULL;
    }
    return (UA_Node*)otnode;
}

static UA_Node *
variableTypeNodeFromAttributes(const UA_AddNodesItem *item, const UA_VariableTypeAttributes *attr) {
    UA_VariableTypeNode *vtnode = UA_NodeStore_newVariableTypeNode();
    if(!vtnode)
        return NULL;
    UA_StatusCode retval = copyStandardAttributes((UA_Node*)vtnode, item, (const UA_NodeAttributes*)attr);
    UA_Variant_copy(&attr->value, &vtnode->value.variant.value);
    // datatype is taken from the value
    vtnode->valueRank = attr->valueRank;
    // array dimensions are taken from the value
    vtnode->isAbstract = attr->isAbstract;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_NodeStore_deleteNode((UA_Node*)vtnode);
        return NULL;
    }
    return (UA_Node*)vtnode;
}

static UA_Node *
viewNodeFromAttributes(const UA_AddNodesItem *item, const UA_ViewAttributes *attr) {
    UA_ViewNode *vnode = UA_NodeStore_newViewNode();
    if(!vnode)
        return NULL;
    UA_StatusCode retval = copyStandardAttributes((UA_Node*)vnode, item, (const UA_NodeAttributes*)attr);
    vnode->containsNoLoops = attr->containsNoLoops;
    vnode->eventNotifier = attr->eventNotifier;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_NodeStore_deleteNode((UA_Node*)vnode);
        return NULL;
    }
    return (UA_Node*)vnode;
}

static UA_Node *
dataTypeNodeFromAttributes(const UA_AddNodesItem *item, const UA_DataTypeAttributes *attr) {
    UA_DataTypeNode *dtnode = UA_NodeStore_newDataTypeNode();
    if(!dtnode)
        return NULL;
    UA_StatusCode retval = copyStandardAttributes((UA_Node*)dtnode, item, (const UA_NodeAttributes*)attr);
    dtnode->isAbstract = attr->isAbstract;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_NodeStore_deleteNode((UA_Node*)dtnode);
        return NULL;
    }
    return (UA_Node*)dtnode;
}

void Service_AddNodes_single(UA_Server *server, UA_Session *session, const UA_AddNodesItem *item,
                             UA_AddNodesResult *result, UA_InstantiationCallback *instantiationCallback) {
    if(item->nodeAttributes.encoding < UA_EXTENSIONOBJECT_DECODED ||
       !item->nodeAttributes.content.decoded.type) {
        result->statusCode = UA_STATUSCODE_BADNODEATTRIBUTESINVALID;
        return;
    }
    
    /* create the node */
    UA_Node *node;
    switch(item->nodeClass) {
    case UA_NODECLASS_OBJECT:
        if(item->nodeAttributes.content.decoded.type != &UA_TYPES[UA_TYPES_OBJECTATTRIBUTES]) {
            result->statusCode = UA_STATUSCODE_BADNODEATTRIBUTESINVALID;
            return;
        }
        node = objectNodeFromAttributes(item, item->nodeAttributes.content.decoded.data);
        break;
    case UA_NODECLASS_VARIABLE:
        if(item->nodeAttributes.content.decoded.type != &UA_TYPES[UA_TYPES_VARIABLEATTRIBUTES]) {
            result->statusCode = UA_STATUSCODE_BADNODEATTRIBUTESINVALID;
            return;
        }
        node = variableNodeFromAttributes(item, item->nodeAttributes.content.decoded.data);
        break;
    case UA_NODECLASS_OBJECTTYPE:
        if(item->nodeAttributes.content.decoded.type != &UA_TYPES[UA_TYPES_OBJECTTYPEATTRIBUTES]) {
            result->statusCode = UA_STATUSCODE_BADNODEATTRIBUTESINVALID;
            return;
        }
        node = objectTypeNodeFromAttributes(item, item->nodeAttributes.content.decoded.data);
        break;
    case UA_NODECLASS_VARIABLETYPE:
        if(item->nodeAttributes.content.decoded.type != &UA_TYPES[UA_TYPES_VARIABLETYPEATTRIBUTES]) {
            result->statusCode = UA_STATUSCODE_BADNODEATTRIBUTESINVALID;
            return;
        }
        node = variableTypeNodeFromAttributes(item, item->nodeAttributes.content.decoded.data);
        break;
    case UA_NODECLASS_REFERENCETYPE:
        if(item->nodeAttributes.content.decoded.type != &UA_TYPES[UA_TYPES_REFERENCETYPEATTRIBUTES]) {
            result->statusCode = UA_STATUSCODE_BADNODEATTRIBUTESINVALID;
            return;
        }
        node = referenceTypeNodeFromAttributes(item, item->nodeAttributes.content.decoded.data);
        break;
    case UA_NODECLASS_DATATYPE:
        if(item->nodeAttributes.content.decoded.type != &UA_TYPES[UA_TYPES_DATATYPEATTRIBUTES]) {
            result->statusCode = UA_STATUSCODE_BADNODEATTRIBUTESINVALID;
            return;
        }
        node = dataTypeNodeFromAttributes(item, item->nodeAttributes.content.decoded.data);
        break;
    case UA_NODECLASS_VIEW:
        if(item->nodeAttributes.content.decoded.type != &UA_TYPES[UA_TYPES_VIEWATTRIBUTES]) {
            result->statusCode = UA_STATUSCODE_BADNODEATTRIBUTESINVALID;
            return;
        }
        node = viewNodeFromAttributes(item, item->nodeAttributes.content.decoded.data);
        break;
    case UA_NODECLASS_METHOD:
    case UA_NODECLASS_UNSPECIFIED:
    default:
        result->statusCode = UA_STATUSCODE_BADNODECLASSINVALID;
        return;
    }

    if(!node) {
        result->statusCode = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }

    /* add it to the server */
    Service_AddNodes_existing(server, session, node, &item->parentNodeId.nodeId,
                              &item->referenceTypeId, result);
    if(result->statusCode != UA_STATUSCODE_GOOD)
        return;
    
    /* instantiate if it has a type */
    if(!UA_NodeId_isNull(&item->typeDefinition.nodeId)) {
        if (instantiationCallback != NULL)
          instantiationCallback->method(result->addedNodeId, item->typeDefinition.nodeId,
                                        instantiationCallback->handle); 
        
        if(item->nodeClass == UA_NODECLASS_OBJECT)
            result->statusCode = instantiateObjectNode(server, session, &result->addedNodeId,
                                                       &item->typeDefinition.nodeId, instantiationCallback);
        else if(item->nodeClass == UA_NODECLASS_VARIABLE)
            result->statusCode = instantiateVariableNode(server, session, &result->addedNodeId,
                                                         &item->typeDefinition.nodeId, instantiationCallback);
    }

    /* if instantiation failed, remove the node */
    if(result->statusCode != UA_STATUSCODE_GOOD)
        Service_DeleteNodes_single(server, session, &result->addedNodeId, true);
}

void Service_AddNodes(UA_Server *server, UA_Session *session, const UA_AddNodesRequest *request,
                      UA_AddNodesResponse *response) {
    if(request->nodesToAddSize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }
    size_t size = request->nodesToAddSize;

    response->results = UA_Array_new(size, &UA_TYPES[UA_TYPES_ADDNODESRESULT]);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }
    
#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
#ifdef _MSC_VER
    UA_Boolean *isExternal = UA_alloca(size);
    UA_UInt32 *indices = UA_alloca(sizeof(UA_UInt32)*size);
#else
    UA_Boolean isExternal[size];
    UA_UInt32 indices[size];
#endif
    memset(isExternal, false, sizeof(UA_Boolean) * size);
    for(size_t j = 0; j <server->externalNamespacesSize; j++) {
        size_t indexSize = 0;
        for(size_t i = 0;i < size;i++) {
            if(request->nodesToAdd[i].requestedNewNodeId.nodeId.namespaceIndex !=
               server->externalNamespaces[j].index)
                continue;
            isExternal[i] = true;
            indices[indexSize] = i;
            indexSize++;
        }
        if(indexSize == 0)
            continue;
        UA_ExternalNodeStore *ens = &server->externalNamespaces[j].externalNodeStore;
        ens->addNodes(ens->ensHandle, &request->requestHeader, request->nodesToAdd,
                      indices, indexSize, response->results, response->diagnosticInfos);
    }
#endif
    
    response->resultsSize = size;
    for(size_t i = 0; i < size; i++) {
#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
        if(!isExternal[i])
#endif
            Service_AddNodes_single(server, session, &request->nodesToAdd[i], &response->results[i], NULL);
    }
}

/**************************************************/
/* Add Special Nodes (not possible over the wire) */
/**************************************************/

UA_StatusCode
UA_Server_addDataSourceVariableNode(UA_Server *server, const UA_NodeId requestedNewNodeId,
                                    const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                                    const UA_QualifiedName browseName, const UA_NodeId typeDefinition,
                                    const UA_VariableAttributes attr, const UA_DataSource dataSource,
                                    UA_NodeId *outNewNodeId) {
    UA_AddNodesResult result;
    UA_AddNodesResult_init(&result);

    UA_AddNodesItem item;
    UA_AddNodesItem_init(&item);
    result.statusCode = UA_QualifiedName_copy(&browseName, &item.browseName);
    item.nodeClass = UA_NODECLASS_VARIABLE;
    result.statusCode |= UA_NodeId_copy(&parentNodeId, &item.parentNodeId.nodeId);
    result.statusCode |= UA_NodeId_copy(&referenceTypeId, &item.referenceTypeId);
    result.statusCode |= UA_NodeId_copy(&requestedNewNodeId, &item.requestedNewNodeId.nodeId);
    result.statusCode |= UA_NodeId_copy(&typeDefinition, &item.typeDefinition.nodeId);
    
    UA_VariableAttributes attrCopy;
    result.statusCode |= UA_VariableAttributes_copy(&attr, &attrCopy);
    if(result.statusCode != UA_STATUSCODE_GOOD) {
        UA_AddNodesItem_deleteMembers(&item);
        UA_VariableAttributes_deleteMembers(&attrCopy);
        return result.statusCode;
    }

    UA_VariableNode *node = UA_NodeStore_newVariableNode();
    if(!node) {
        UA_AddNodesItem_deleteMembers(&item);
        UA_VariableAttributes_deleteMembers(&attrCopy);
        return UA_STATUSCODE_BADOUTOFMEMORY;
    }

    copyStandardAttributes((UA_Node*)node, &item, (UA_NodeAttributes*)&attrCopy);
    node->valueSource = UA_VALUESOURCE_DATASOURCE;
    node->value.dataSource = dataSource;
    node->accessLevel = attr.accessLevel;
    node->userAccessLevel = attr.userAccessLevel;
    node->historizing = attr.historizing;
    node->minimumSamplingInterval = attr.minimumSamplingInterval;
    node->valueRank = attr.valueRank;
    UA_RCU_LOCK();
    Service_AddNodes_existing(server, &adminSession, (UA_Node*)node, &item.parentNodeId.nodeId,
                              &item.referenceTypeId, &result);
    UA_RCU_UNLOCK();
    UA_AddNodesItem_deleteMembers(&item);
    UA_VariableAttributes_deleteMembers(&attrCopy);

    if(outNewNodeId && result.statusCode == UA_STATUSCODE_GOOD)
        *outNewNodeId = result.addedNodeId;
    else
        UA_AddNodesResult_deleteMembers(&result);
    return result.statusCode;
}

#ifdef UA_ENABLE_METHODCALLS

UA_StatusCode
UA_Server_addMethodNode(UA_Server *server, const UA_NodeId requestedNewNodeId,
                        const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                        const UA_QualifiedName browseName, const UA_MethodAttributes attr,
                        UA_MethodCallback method, void *handle,
                        size_t inputArgumentsSize, const UA_Argument* inputArguments, 
                        size_t outputArgumentsSize, const UA_Argument* outputArguments,
                        UA_NodeId *outNewNodeId) {
    UA_AddNodesResult result;
    UA_AddNodesResult_init(&result);
    
    UA_AddNodesItem item;
    UA_AddNodesItem_init(&item);
    result.statusCode = UA_QualifiedName_copy(&browseName, &item.browseName);
    item.nodeClass = UA_NODECLASS_METHOD;
    result.statusCode |= UA_NodeId_copy(&parentNodeId, &item.parentNodeId.nodeId);
    result.statusCode |= UA_NodeId_copy(&referenceTypeId, &item.referenceTypeId);
    result.statusCode |= UA_NodeId_copy(&requestedNewNodeId, &item.requestedNewNodeId.nodeId);
    
    UA_MethodAttributes attrCopy;
    result.statusCode |= UA_MethodAttributes_copy(&attr, &attrCopy);
    if(result.statusCode != UA_STATUSCODE_GOOD) {
        UA_AddNodesItem_deleteMembers(&item);
        UA_MethodAttributes_deleteMembers(&attrCopy);
        return result.statusCode;
    }

    UA_MethodNode *node = UA_NodeStore_newMethodNode();
    if(!node) {
        result.statusCode = UA_STATUSCODE_BADOUTOFMEMORY;
        UA_AddNodesItem_deleteMembers(&item);
        UA_MethodAttributes_deleteMembers(&attrCopy);
        return result.statusCode;
    }
    
    copyStandardAttributes((UA_Node*)node, &item, (UA_NodeAttributes*)&attrCopy);
    node->executable = attrCopy.executable;
    node->userExecutable = attrCopy.executable;
    node->attachedMethod = method;
    node->methodHandle = handle;
    UA_AddNodesItem_deleteMembers(&item);
    UA_MethodAttributes_deleteMembers(&attrCopy);

    UA_RCU_LOCK();
    Service_AddNodes_existing(server, &adminSession, (UA_Node*)node, &item.parentNodeId.nodeId,
                              &item.referenceTypeId, &result);
    UA_RCU_UNLOCK();
    if(result.statusCode != UA_STATUSCODE_GOOD)
        return result.statusCode;
    
    UA_ExpandedNodeId parent;
    UA_ExpandedNodeId_init(&parent);
    parent.nodeId = result.addedNodeId;
    
    const UA_NodeId hasproperty = UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY);
    UA_VariableNode *inputArgumentsVariableNode = UA_NodeStore_newVariableNode();
    inputArgumentsVariableNode->nodeId.namespaceIndex = result.addedNodeId.namespaceIndex;
    inputArgumentsVariableNode->browseName = UA_QUALIFIEDNAME_ALLOC(0, "InputArguments");
    inputArgumentsVariableNode->displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", "InputArguments");
    inputArgumentsVariableNode->description = UA_LOCALIZEDTEXT_ALLOC("en_US", "InputArguments");
    inputArgumentsVariableNode->valueRank = 1;
    UA_Variant_setArrayCopy(&inputArgumentsVariableNode->value.variant.value, inputArguments,
                            inputArgumentsSize, &UA_TYPES[UA_TYPES_ARGUMENT]);
    UA_AddNodesResult inputAddRes;
    UA_RCU_LOCK();
    Service_AddNodes_existing(server, &adminSession, (UA_Node*)inputArgumentsVariableNode,
                             &parent.nodeId, &hasproperty, &inputAddRes);
    UA_RCU_UNLOCK();
    // todo: check if adding succeeded
    UA_AddNodesResult_deleteMembers(&inputAddRes);
    
    /* create OutputArguments */
    UA_VariableNode *outputArgumentsVariableNode  = UA_NodeStore_newVariableNode();
    outputArgumentsVariableNode->nodeId.namespaceIndex = result.addedNodeId.namespaceIndex;
    outputArgumentsVariableNode->browseName  = UA_QUALIFIEDNAME_ALLOC(0, "OutputArguments");
    outputArgumentsVariableNode->displayName = UA_LOCALIZEDTEXT_ALLOC("en_US", "OutputArguments");
    outputArgumentsVariableNode->description = UA_LOCALIZEDTEXT_ALLOC("en_US", "OutputArguments");
    outputArgumentsVariableNode->valueRank = 1;
    UA_Variant_setArrayCopy(&outputArgumentsVariableNode->value.variant.value, outputArguments,
                            outputArgumentsSize, &UA_TYPES[UA_TYPES_ARGUMENT]);
    UA_AddNodesResult outputAddRes;
    UA_RCU_LOCK();
    Service_AddNodes_existing(server, &adminSession, (UA_Node*)outputArgumentsVariableNode,
                              &parent.nodeId, &hasproperty, &outputAddRes);
    UA_RCU_UNLOCK();
    // todo: check if adding succeeded
    UA_AddNodesResult_deleteMembers(&outputAddRes);
    
    if(outNewNodeId)
        *outNewNodeId = result.addedNodeId; // don't deleteMember the result
    else
        UA_AddNodesResult_deleteMembers(&result);
    return result.statusCode;
}

#endif

/******************/
/* Add References */
/******************/

/* Adds a one-way reference to the local nodestore */
static UA_StatusCode
addOneWayReference(UA_Server *server, UA_Session *session, UA_Node *node, const UA_AddReferencesItem *item) {
	size_t i = node->referencesSize;
    size_t refssize = (i+1) | 3; // so the realloc is not necessary every time
	UA_ReferenceNode *new_refs = UA_realloc(node->references, sizeof(UA_ReferenceNode) * refssize);
	if(!new_refs)
		return UA_STATUSCODE_BADOUTOFMEMORY;
    node->references = new_refs;
    UA_ReferenceNode_init(&new_refs[i]);
    UA_StatusCode retval = UA_NodeId_copy(&item->referenceTypeId, &new_refs[i].referenceTypeId);
    retval |= UA_ExpandedNodeId_copy(&item->targetNodeId, &new_refs[i].targetId);
    new_refs[i].isInverse = !item->isForward;
    if(retval == UA_STATUSCODE_GOOD) 
        node->referencesSize = i+1;
    else
        UA_ReferenceNode_deleteMembers(&new_refs[i]);
	return retval;
}

UA_StatusCode
Service_AddReferences_single(UA_Server *server, UA_Session *session, const UA_AddReferencesItem *item) {
    if(item->targetServerUri.length > 0)
        return UA_STATUSCODE_BADNOTIMPLEMENTED; // currently no expandednodeids are allowed

    /* cast away the const to loop the call through UA_Server_editNode */
    UA_StatusCode retval = UA_Server_editNode(server, session, &item->sourceNodeId,
                                              (UA_EditNodeCallback)addOneWayReference, item);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;

    UA_AddReferencesItem secondItem;
    secondItem = *item;
    secondItem.targetNodeId.nodeId = item->sourceNodeId;
    secondItem.sourceNodeId = item->targetNodeId.nodeId;
    secondItem.isForward = !item->isForward;
    retval = UA_Server_editNode(server, session, &secondItem.sourceNodeId,
                                (UA_EditNodeCallback)addOneWayReference, &secondItem);

    // todo: remove reference if the second direction failed
    return retval;
} 

void Service_AddReferences(UA_Server *server, UA_Session *session,
                           const UA_AddReferencesRequest *request,
                           UA_AddReferencesResponse *response) {
	if(request->referencesToAddSize <= 0) {
		response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
		return;
	}
    size_t size = request->referencesToAddSize;
	
    if(!(response->results = UA_malloc(sizeof(UA_StatusCode) * size))) {
		response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
		return;
	}
	response->resultsSize = size;

#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
#ifdef NO_ALLOCA
    UA_Boolean isExternal[size];
    UA_UInt32 indices[size];
#else
    UA_Boolean *isExternal = UA_alloca(sizeof(UA_Boolean) * size);
    UA_UInt32 *indices = UA_alloca(sizeof(UA_UInt32) * size);
#endif /*NO_ALLOCA */
    memset(isExternal, false, sizeof(UA_Boolean) * size);
	for(size_t j = 0; j < server->externalNamespacesSize; j++) {
		size_t indicesSize = 0;
		for(size_t i = 0;i < size;i++) {
			if(request->referencesToAdd[i].sourceNodeId.namespaceIndex
               != server->externalNamespaces[j].index)
				continue;
			isExternal[i] = true;
			indices[indicesSize] = i;
			indicesSize++;
		}
		if (indicesSize == 0)
			continue;
		UA_ExternalNodeStore *ens = &server->externalNamespaces[j].externalNodeStore;
		ens->addReferences(ens->ensHandle, &request->requestHeader, request->referencesToAdd,
                           indices, indicesSize, response->results, response->diagnosticInfos);
	}
#endif

	for(size_t i = 0; i < response->resultsSize; i++) {
#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
		if(!isExternal[i])
#endif
            Service_AddReferences_single(server, session, &request->referencesToAdd[i]);
	}
}

/****************/
/* Delete Nodes */
/****************/

// TODO: Check consistency constraints, remove the references.

UA_StatusCode
Service_DeleteNodes_single(UA_Server *server, UA_Session *session, const UA_NodeId *nodeId,
                           UA_Boolean deleteReferences) {
    const UA_Node *node = UA_NodeStore_get(server->nodestore, nodeId);
    if(!node)
        return UA_STATUSCODE_BADNODEIDINVALID;
    if(deleteReferences == true) {
        UA_DeleteReferencesItem delItem;
        UA_DeleteReferencesItem_init(&delItem);
        delItem.deleteBidirectional = false;
        delItem.targetNodeId.nodeId = *nodeId;
        for(size_t i = 0; i < node->referencesSize; i++) {
            delItem.sourceNodeId = node->references[i].targetId.nodeId;
            delItem.isForward = node->references[i].isInverse;
            Service_DeleteReferences_single(server, session, &delItem);
        }
    }

    /* destroy an object before removing it */
    if(node->nodeClass == UA_NODECLASS_OBJECT) {
        /* find the object type(s) */
        UA_BrowseDescription bd;
        UA_BrowseDescription_init(&bd);
        bd.browseDirection = UA_BROWSEDIRECTION_INVERSE;
        bd.nodeId = *nodeId;
        bd.referenceTypeId = UA_NODEID_NUMERIC(0, UA_NS0ID_HASSUBTYPE);
        bd.includeSubtypes = true;
        bd.nodeClassMask = UA_NODECLASS_OBJECTTYPE;
        
        /* browse type definitions with admin rights */
        UA_BrowseResult result;
        UA_BrowseResult_init(&result);
        Service_Browse_single(server, &adminSession, NULL, &bd, UA_UINT32_MAX, &result);
        for(size_t i = 0; i < result.referencesSize; i++) {
            /* call the destructor */
            UA_ReferenceDescription *rd = &result.references[i];
            const UA_ObjectTypeNode *typenode =
                (const UA_ObjectTypeNode*)UA_NodeStore_get(server->nodestore, &rd->nodeId.nodeId);
            if(!typenode)
                continue;
            if(typenode->nodeClass != UA_NODECLASS_OBJECTTYPE || !typenode->lifecycleManagement.destructor)
                continue;

            /* if there are several types with lifecycle management, call all the destructors */
            typenode->lifecycleManagement.destructor(*nodeId, ((const UA_ObjectNode*)node)->instanceHandle);
        }
        UA_BrowseResult_deleteMembers(&result);
    }
    
    return UA_NodeStore_remove(server->nodestore, nodeId);
}

void Service_DeleteNodes(UA_Server *server, UA_Session *session, const UA_DeleteNodesRequest *request,
                         UA_DeleteNodesResponse *response) {
    if(request->nodesToDeleteSize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }
    response->results = UA_malloc(sizeof(UA_StatusCode) * request->nodesToDeleteSize);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;;
        return;
    }
    response->resultsSize = request->nodesToDeleteSize;
    for(size_t i=0; i<request->nodesToDeleteSize; i++) {
        UA_DeleteNodesItem *item = &request->nodesToDelete[i];
        response->results[i] = Service_DeleteNodes_single(server, session, &item->nodeId,
                                                          item->deleteTargetReferences);
    }
}

/*********************/
/* Delete References */
/*********************/

static UA_StatusCode
deleteOneWayReference(UA_Server *server, UA_Session *session, UA_Node *node,
                      const UA_DeleteReferencesItem *item) {
    UA_Boolean edited = false;
    for(size_t i = node->referencesSize - 1; ; i--) {
        if(i > node->referencesSize)
            break; /* underflow after i == 0 */
        if(!UA_NodeId_equal(&item->targetNodeId.nodeId, &node->references[i].targetId.nodeId))
            continue;
        if(!UA_NodeId_equal(&item->referenceTypeId, &node->references[i].referenceTypeId))
            continue;
        if(item->isForward == node->references[i].isInverse)
            continue;
        /* move the last entry to override the current position */
        UA_ReferenceNode_deleteMembers(&node->references[i]);
        node->references[i] = node->references[node->referencesSize-1];
        node->referencesSize--;
        edited = true;
        break;
    }
    if(!edited)
        return UA_STATUSCODE_UNCERTAINREFERENCENOTDELETED;
    /* we removed the last reference */
    if(node->referencesSize == 0 && node->references)
        UA_free(node->references);
    return UA_STATUSCODE_GOOD;;
}

UA_StatusCode
Service_DeleteReferences_single(UA_Server *server, UA_Session *session,
                                const UA_DeleteReferencesItem *item) {
    UA_StatusCode retval = UA_Server_editNode(server, session, &item->sourceNodeId,
                                              (UA_EditNodeCallback)deleteOneWayReference, item);
    if(!item->deleteBidirectional || item->targetNodeId.serverIndex != 0)
        return retval;
    UA_DeleteReferencesItem secondItem;
    UA_DeleteReferencesItem_init(&secondItem);
    secondItem.isForward = !item->isForward;
    secondItem.sourceNodeId = item->targetNodeId.nodeId;
    secondItem.targetNodeId.nodeId = item->sourceNodeId;
    return UA_Server_editNode(server, session, &secondItem.sourceNodeId,
                              (UA_EditNodeCallback)deleteOneWayReference, &secondItem);
}

void
Service_DeleteReferences(UA_Server *server, UA_Session *session, const UA_DeleteReferencesRequest *request,
                         UA_DeleteReferencesResponse *response) {
    if(request->referencesToDeleteSize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }
    response->results = UA_malloc(sizeof(UA_StatusCode) * request->referencesToDeleteSize);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;;
        return;
    }
    response->resultsSize = request->referencesToDeleteSize;
    for(size_t i = 0; i < request->referencesToDeleteSize; i++)
        response->results[i] =
            Service_DeleteReferences_single(server, session, &request->referencesToDelete[i]);
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_services_view.c" ***********************************/


static UA_StatusCode
fillReferenceDescription(UA_NodeStore *ns, const UA_Node *curr, UA_ReferenceNode *ref,
                         UA_UInt32 mask, UA_ReferenceDescription *descr) {
    UA_ReferenceDescription_init(descr);
    UA_StatusCode retval = UA_NodeId_copy(&curr->nodeId, &descr->nodeId.nodeId);
    if(mask & UA_BROWSERESULTMASK_REFERENCETYPEID)
        retval |= UA_NodeId_copy(&ref->referenceTypeId, &descr->referenceTypeId);
    if(mask & UA_BROWSERESULTMASK_ISFORWARD)
        descr->isForward = !ref->isInverse;
    if(mask & UA_BROWSERESULTMASK_NODECLASS)
        retval |= UA_NodeClass_copy(&curr->nodeClass, &descr->nodeClass);
    if(mask & UA_BROWSERESULTMASK_BROWSENAME)
        retval |= UA_QualifiedName_copy(&curr->browseName, &descr->browseName);
    if(mask & UA_BROWSERESULTMASK_DISPLAYNAME)
        retval |= UA_LocalizedText_copy(&curr->displayName, &descr->displayName);
    if(mask & UA_BROWSERESULTMASK_TYPEDEFINITION){
        if(curr->nodeClass == UA_NODECLASS_OBJECT || curr->nodeClass == UA_NODECLASS_VARIABLE) {
            for(size_t i = 0; i < curr->referencesSize; i++) {
                UA_ReferenceNode *refnode = &curr->references[i];
                if(refnode->referenceTypeId.identifier.numeric != UA_NS0ID_HASTYPEDEFINITION)
                    continue;
                retval |= UA_ExpandedNodeId_copy(&refnode->targetId, &descr->typeDefinition);
                break;
            }
        }
    }
    return retval;
}

#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
static const UA_Node *
returnRelevantNodeExternal(UA_ExternalNodeStore *ens, const UA_BrowseDescription *descr,
                           const UA_ReferenceNode *reference) {
    /*	prepare a read request in the external nodestore	*/
    UA_ReadValueId *readValueIds = UA_Array_new(6,&UA_TYPES[UA_TYPES_READVALUEID]);
    UA_UInt32 *indices = UA_Array_new(6,&UA_TYPES[UA_TYPES_UINT32]);
    UA_UInt32 indicesSize = 6;
    UA_DataValue *readNodesResults = UA_Array_new(6,&UA_TYPES[UA_TYPES_DATAVALUE]);
    UA_DiagnosticInfo *diagnosticInfos = UA_Array_new(6,&UA_TYPES[UA_TYPES_DIAGNOSTICINFO]);
    for(UA_UInt32 i = 0; i < 6; i++) {
        readValueIds[i].nodeId = reference->targetId.nodeId;
        indices[i] = i;
    }
    readValueIds[0].attributeId = UA_ATTRIBUTEID_NODECLASS;
    readValueIds[1].attributeId = UA_ATTRIBUTEID_BROWSENAME;
    readValueIds[2].attributeId = UA_ATTRIBUTEID_DISPLAYNAME;
    readValueIds[3].attributeId = UA_ATTRIBUTEID_DESCRIPTION;
    readValueIds[4].attributeId = UA_ATTRIBUTEID_WRITEMASK;
    readValueIds[5].attributeId = UA_ATTRIBUTEID_USERWRITEMASK;

    ens->readNodes(ens->ensHandle, NULL, readValueIds, indices,
                   indicesSize, readNodesResults, false, diagnosticInfos);

    /* create and fill a dummy nodeStructure */
    UA_Node *node = (UA_Node*) UA_NodeStore_newObjectNode();
    UA_NodeId_copy(&(reference->targetId.nodeId), &(node->nodeId));
    if(readNodesResults[0].status == UA_STATUSCODE_GOOD)
        UA_NodeClass_copy((UA_NodeClass*)readNodesResults[0].value.data, &(node->nodeClass));
    if(readNodesResults[1].status == UA_STATUSCODE_GOOD)
        UA_QualifiedName_copy((UA_QualifiedName*)readNodesResults[1].value.data, &(node->browseName));
    if(readNodesResults[2].status == UA_STATUSCODE_GOOD)
        UA_LocalizedText_copy((UA_LocalizedText*)readNodesResults[2].value.data, &(node->displayName));
    if(readNodesResults[3].status == UA_STATUSCODE_GOOD)
        UA_LocalizedText_copy((UA_LocalizedText*)readNodesResults[3].value.data, &(node->description));
    if(readNodesResults[4].status == UA_STATUSCODE_GOOD)
        UA_UInt32_copy((UA_UInt32*)readNodesResults[4].value.data, &(node->writeMask));
    if(readNodesResults[5].status == UA_STATUSCODE_GOOD)
        UA_UInt32_copy((UA_UInt32*)readNodesResults[5].value.data, &(node->userWriteMask));
    UA_Array_delete(readValueIds,6, &UA_TYPES[UA_TYPES_READVALUEID]);
    UA_Array_delete(indices,6, &UA_TYPES[UA_TYPES_UINT32]);
    UA_Array_delete(readNodesResults,6, &UA_TYPES[UA_TYPES_DATAVALUE]);
    UA_Array_delete(diagnosticInfos,6, &UA_TYPES[UA_TYPES_DIAGNOSTICINFO]);
    if(node && descr->nodeClassMask != 0 && (node->nodeClass & descr->nodeClassMask) == 0) {
        UA_NodeStore_deleteNode(node);
        return NULL;
    }
    return node;
}
#endif

/* Tests if the node is relevant to the browse request and shall be returned. If
   so, it is retrieved from the Nodestore. If not, null is returned. */
static const UA_Node *
returnRelevantNode(UA_Server *server, const UA_BrowseDescription *descr, UA_Boolean return_all,
                   const UA_ReferenceNode *reference, const UA_NodeId *relevant, size_t relevant_count,
                   UA_Boolean *isExternal) {
    /* reference in the right direction? */
    if(reference->isInverse && descr->browseDirection == UA_BROWSEDIRECTION_FORWARD)
        return NULL;
    if(!reference->isInverse && descr->browseDirection == UA_BROWSEDIRECTION_INVERSE)
        return NULL;

    /* is the reference part of the hierarchy of references we look for? */
    if(!return_all) {
        UA_Boolean is_relevant = false;
        for(size_t i = 0; i < relevant_count; i++) {
            if(UA_NodeId_equal(&reference->referenceTypeId, &relevant[i])) {
                is_relevant = true;
                break;
            }
        }
        if(!is_relevant)
            return NULL;
    }

#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
    /* return the node from an external namespace*/
	for(size_t nsIndex = 0; nsIndex < server->externalNamespacesSize; nsIndex++) {
		if(reference->targetId.nodeId.namespaceIndex != server->externalNamespaces[nsIndex].index)
			continue;
        *isExternal = true;
        return returnRelevantNodeExternal(&server->externalNamespaces[nsIndex].externalNodeStore,
                                          descr, reference);
    }
#endif

    /* return from the internal nodestore */
    const UA_Node *node = UA_NodeStore_get(server->nodestore, &reference->targetId.nodeId);
    if(node && descr->nodeClassMask != 0 && (node->nodeClass & descr->nodeClassMask) == 0)
        return NULL;
    *isExternal = false;
    return node;
}

/**
 * We find all subtypes by a single iteration over the array. We start with an array with a single
 * root nodeid at the beginning. When we find relevant references, we add the nodeids to the back of
 * the array and increase the size. Since the hierarchy is not cyclic, we can safely progress in the
 * array to process the newly found referencetype nodeids (emulated recursion).
 */
static UA_StatusCode
findSubTypes(UA_NodeStore *ns, const UA_NodeId *root, UA_NodeId **reftypes, size_t *reftypes_count) {
    const UA_Node *node = UA_NodeStore_get(ns, root);
    if(!node)
        return UA_STATUSCODE_BADNOMATCH;
    if(node->nodeClass != UA_NODECLASS_REFERENCETYPE)
        return UA_STATUSCODE_BADREFERENCETYPEIDINVALID;

    size_t results_size = 20; // probably too big, but saves mallocs
    UA_NodeId *results = UA_malloc(sizeof(UA_NodeId) * results_size);
    if(!results)
        return UA_STATUSCODE_BADOUTOFMEMORY;

    UA_StatusCode retval = UA_NodeId_copy(root, &results[0]);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_free(results);
        return retval;
    }
        
    size_t idx = 0; // where are we currently in the array?
    size_t last = 0; // where is the last element in the array?
    do {
        node = UA_NodeStore_get(ns, &results[idx]);
        if(!node || node->nodeClass != UA_NODECLASS_REFERENCETYPE)
            continue;
        for(size_t i = 0; i < node->referencesSize; i++) {
            if(node->references[i].referenceTypeId.identifier.numeric != UA_NS0ID_HASSUBTYPE ||
               node->references[i].isInverse == true)
                continue;

            if(++last >= results_size) { // is the array big enough?
                UA_NodeId *new_results = UA_realloc(results, sizeof(UA_NodeId) * results_size * 2);
                if(!new_results) {
                    retval = UA_STATUSCODE_BADOUTOFMEMORY;
                    break;
                }
                results = new_results;
                results_size *= 2;
            }

            retval = UA_NodeId_copy(&node->references[i].targetId.nodeId, &results[last]);
            if(retval != UA_STATUSCODE_GOOD) {
                last--; // for array_delete
                break;
            }
        }
    } while(++idx <= last && retval == UA_STATUSCODE_GOOD);

    if(retval != UA_STATUSCODE_GOOD) {
        UA_Array_delete(results, last, &UA_TYPES[UA_TYPES_NODEID]);
        return retval;
    }

    *reftypes = results;
    *reftypes_count = last + 1;
    return UA_STATUSCODE_GOOD;
}

static void removeCp(struct ContinuationPointEntry *cp, UA_Session* session) {
    LIST_REMOVE(cp, pointers);
    UA_ByteString_deleteMembers(&cp->identifier);
    UA_BrowseDescription_deleteMembers(&cp->browseDescription);
    UA_free(cp);
    session->availableContinuationPoints++;
}

/**
 * Results for a single browsedescription. This is the inner loop for both Browse and BrowseNext
 * @param session Session to save continuationpoints
 * @param ns The nodstore where the to-be-browsed node can be found
 * @param cp If cp is not null, we continue from here
 *           If cp is null, we can add a new continuation point if possible and necessary.
 * @param descr If no cp is set, we take the browsedescription from there
 * @param maxrefs The maximum number of references the client has requested
 * @param result The entry in the request
 */
void
Service_Browse_single(UA_Server *server, UA_Session *session, struct ContinuationPointEntry *cp,
                      const UA_BrowseDescription *descr, UA_UInt32 maxrefs, UA_BrowseResult *result) { 
    size_t referencesCount = 0;
    size_t referencesIndex = 0;
    /* set the browsedescription if a cp is given */
    UA_UInt32 continuationIndex = 0;
    if(cp) {
        descr = &cp->browseDescription;
        maxrefs = cp->maxReferences;
        continuationIndex = cp->continuationIndex;
    }

    /* is the browsedirection valid? */
    if(descr->browseDirection != UA_BROWSEDIRECTION_BOTH &&
       descr->browseDirection != UA_BROWSEDIRECTION_FORWARD &&
       descr->browseDirection != UA_BROWSEDIRECTION_INVERSE) {
        result->statusCode = UA_STATUSCODE_BADBROWSEDIRECTIONINVALID;
        return;
    }
    
    /* get the references that match the browsedescription */
    size_t relevant_refs_size = 0;
    UA_NodeId *relevant_refs = NULL;
    UA_Boolean all_refs = UA_NodeId_isNull(&descr->referenceTypeId);
    if(!all_refs) {
        if(descr->includeSubtypes) {
            result->statusCode = findSubTypes(server->nodestore, &descr->referenceTypeId,
                                              &relevant_refs, &relevant_refs_size);
            if(result->statusCode != UA_STATUSCODE_GOOD)
                return;
        } else {
            const UA_Node *rootRef = UA_NodeStore_get(server->nodestore, &descr->referenceTypeId);
            if(!rootRef || rootRef->nodeClass != UA_NODECLASS_REFERENCETYPE) {
                result->statusCode = UA_STATUSCODE_BADREFERENCETYPEIDINVALID;
                return;
            }
            relevant_refs = (UA_NodeId*)(uintptr_t)&descr->referenceTypeId;
            relevant_refs_size = 1;
        }
    }

    /* get the node */
    const UA_Node *node = UA_NodeStore_get(server->nodestore, &descr->nodeId);
    if(!node) {
        result->statusCode = UA_STATUSCODE_BADNODEIDUNKNOWN;
        if(!all_refs && descr->includeSubtypes)
            UA_Array_delete(relevant_refs, relevant_refs_size, &UA_TYPES[UA_TYPES_NODEID]);
        return;
    }

    /* if the node has no references, just return */
    if(node->referencesSize == 0) {
        result->referencesSize = 0;
        if(!all_refs && descr->includeSubtypes)
            UA_Array_delete(relevant_refs, relevant_refs_size, &UA_TYPES[UA_TYPES_NODEID]);
        return;
    }

    /* how many references can we return at most? */
    size_t real_maxrefs = maxrefs;
    if(real_maxrefs == 0)
        real_maxrefs = node->referencesSize;
    if(node->referencesSize == 0)
        real_maxrefs = 0;
    else if(real_maxrefs > node->referencesSize)
        real_maxrefs = node->referencesSize;
    result->references = UA_Array_new(real_maxrefs, &UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION]);
    if(!result->references) {
        result->statusCode = UA_STATUSCODE_BADOUTOFMEMORY;
        goto cleanup;
    }

    /* loop over the node's references */
    size_t skipped = 0;
    UA_Boolean isExternal = false;
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    for(; referencesIndex < node->referencesSize && referencesCount < real_maxrefs; referencesIndex++) {
    	isExternal = false;
    	const UA_Node *current =
            returnRelevantNode(server, descr, all_refs, &node->references[referencesIndex],
                               relevant_refs, relevant_refs_size, &isExternal);
        if(!current)
            continue;

        if(skipped < continuationIndex) {
            skipped++;
        } else {
            retval |= fillReferenceDescription(server->nodestore, current, &node->references[referencesIndex],
                                               descr->resultMask, &result->references[referencesCount]);
            referencesCount++;
        }
#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
        /* relevant_node returns a node malloced by the nodestore.
           if it is external (there is no UA_Node_new function) */
   //     if(isExternal == true)
   //         UA_Node_deleteMembersAnyNodeClass(current);
   //TODO something's wrong here...
#endif
    }

    result->referencesSize = referencesCount;
    if(referencesCount == 0) {
        UA_free(result->references);
        result->references = NULL;
    }

    if(retval != UA_STATUSCODE_GOOD) {
        UA_Array_delete(result->references, result->referencesSize, &UA_TYPES[UA_TYPES_REFERENCEDESCRIPTION]);
        result->references = NULL;
        result->referencesSize = 0;
        result->statusCode = retval;
    }

    cleanup:
    if(!all_refs && descr->includeSubtypes)
        UA_Array_delete(relevant_refs, relevant_refs_size, &UA_TYPES[UA_TYPES_NODEID]);
    if(result->statusCode != UA_STATUSCODE_GOOD)
        return;

    /* create, update, delete continuation points */
    if(cp) {
        if(referencesIndex == node->referencesSize) {
            /* all done, remove a finished continuationPoint */
            removeCp(cp, session);
        } else {
            /* update the cp and return the cp identifier */
            cp->continuationIndex += (UA_UInt32)referencesCount;
            UA_ByteString_copy(&cp->identifier, &result->continuationPoint);
        }
    } else if(maxrefs != 0 && referencesCount >= maxrefs) {
        /* create a cp */
        if(session->availableContinuationPoints <= 0 ||
           !(cp = UA_malloc(sizeof(struct ContinuationPointEntry)))) {
            result->statusCode = UA_STATUSCODE_BADNOCONTINUATIONPOINTS;
            return;
        }
        UA_BrowseDescription_copy(descr, &cp->browseDescription);
        cp->maxReferences = maxrefs;
        cp->continuationIndex = (UA_UInt32)referencesCount;
        UA_Guid *ident = UA_Guid_new();
        *ident = UA_Guid_random();
        cp->identifier.data = (UA_Byte*)ident;
        cp->identifier.length = sizeof(UA_Guid);
        UA_ByteString_copy(&cp->identifier, &result->continuationPoint);

        /* store the cp */
        LIST_INSERT_HEAD(&session->continuationPoints, cp, pointers);
        session->availableContinuationPoints--;
    }
}

void Service_Browse(UA_Server *server, UA_Session *session, const UA_BrowseRequest *request,
                    UA_BrowseResponse *response) {
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SESSION,
                 "Processing BrowseRequest for Session (ns=%i,i=%i)",
                 session->sessionId.namespaceIndex, session->sessionId.identifier.numeric);
    if(!UA_NodeId_isNull(&request->view.viewId)) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADVIEWIDUNKNOWN;
        return;
    }
    
    if(request->nodesToBrowseSize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }

    size_t size = request->nodesToBrowseSize;
    response->results = UA_Array_new(size, &UA_TYPES[UA_TYPES_BROWSERESULT]);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }
    response->resultsSize = size;
    
#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
#ifdef NO_ALLOCA
    UA_Boolean isExternal[size];
    UA_UInt32 indices[size];
#else
    UA_Boolean *isExternal = UA_alloca(sizeof(UA_Boolean) * size);
    UA_UInt32 *indices = UA_alloca(sizeof(UA_UInt32) * size);
#endif /*NO_ALLOCA */
    memset(isExternal, false, sizeof(UA_Boolean) * size);
    for(size_t j = 0; j < server->externalNamespacesSize; j++) {
        size_t indexSize = 0;
        for(size_t i = 0; i < size; i++) {
            if(request->nodesToBrowse[i].nodeId.namespaceIndex != server->externalNamespaces[j].index)
                continue;
            isExternal[i] = true;
            indices[indexSize] = i;
            indexSize++;
        }
        if(indexSize == 0)
            continue;
        UA_ExternalNodeStore *ens = &server->externalNamespaces[j].externalNodeStore;
        ens->browseNodes(ens->ensHandle, &request->requestHeader, request->nodesToBrowse, indices, indexSize,
                         request->requestedMaxReferencesPerNode, response->results, response->diagnosticInfos);
    }
#endif

    for(size_t i = 0; i < size; i++) {
#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
        if(!isExternal[i])
#endif
            Service_Browse_single(server, session, NULL, &request->nodesToBrowse[i],
                                  request->requestedMaxReferencesPerNode, &response->results[i]);
    }
}

void
UA_Server_browseNext_single(UA_Server *server, UA_Session *session, UA_Boolean releaseContinuationPoint,
                            const UA_ByteString *continuationPoint, UA_BrowseResult *result) {
    result->statusCode = UA_STATUSCODE_BADCONTINUATIONPOINTINVALID;
    struct ContinuationPointEntry *cp, *temp;
    LIST_FOREACH_SAFE(cp, &session->continuationPoints, pointers, temp) {
        if(UA_ByteString_equal(&cp->identifier, continuationPoint)) {
            result->statusCode = UA_STATUSCODE_GOOD;
            if(!releaseContinuationPoint)
                Service_Browse_single(server, session, cp, NULL, 0, result);
            else
                removeCp(cp, session);
            break;
        }
    }
}

void Service_BrowseNext(UA_Server *server, UA_Session *session, const UA_BrowseNextRequest *request,
                        UA_BrowseNextResponse *response) {
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SESSION,
                 "Processing BrowseNextRequest for Session (ns=%i,i=%i)",
                 session->sessionId.namespaceIndex, session->sessionId.identifier.numeric);
   if(request->continuationPointsSize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
   }
   size_t size = request->continuationPointsSize;
   response->results = UA_Array_new(size, &UA_TYPES[UA_TYPES_BROWSERESULT]);
   if(!response->results) {
       response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
       return;
   }

   response->resultsSize = size;
   for(size_t i = 0; i < size; i++)
       UA_Server_browseNext_single(server, session, request->releaseContinuationPoints,
                                   &request->continuationPoints[i], &response->results[i]);
}

/***********************/
/* TranslateBrowsePath */
/***********************/

static UA_StatusCode
walkBrowsePath(UA_Server *server, UA_Session *session, const UA_Node *node, const UA_RelativePath *path,
               size_t pathindex, UA_BrowsePathTarget **targets, size_t *targets_size,
               size_t *target_count) {
    const UA_RelativePathElement *elem = &path->elements[pathindex];
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_NodeId *reftypes = NULL;
    size_t reftypes_count = 1; // all_refs or no subtypes => 1
    UA_Boolean all_refs = false;
    if(UA_NodeId_isNull(&elem->referenceTypeId))
        all_refs = true;
    else if(!elem->includeSubtypes)
        reftypes = (UA_NodeId*)(uintptr_t)&elem->referenceTypeId; // ptr magic due to const cast
    else {
        retval = findSubTypes(server->nodestore, &elem->referenceTypeId, &reftypes, &reftypes_count);
        if(retval != UA_STATUSCODE_GOOD)
            return retval;
    }

    for(size_t i = 0; i < node->referencesSize && retval == UA_STATUSCODE_GOOD; i++) {
        UA_Boolean match = all_refs;
        for(size_t j = 0; j < reftypes_count && !match; j++) {
            if(node->references[i].isInverse == elem->isInverse &&
               UA_NodeId_equal(&node->references[i].referenceTypeId, &reftypes[j]))
                match = true;
        }
        if(!match)
            continue;

        // get the node, todo: expandednodeid
        const UA_Node *next = UA_NodeStore_get(server->nodestore, &node->references[i].targetId.nodeId);
        if(!next)
            continue;

        // test the browsename
        if(elem->targetName.namespaceIndex != next->browseName.namespaceIndex ||
           !UA_String_equal(&elem->targetName.name, &next->browseName.name)) {
            continue;
        }

        if(pathindex + 1 < path->elementsSize) {
            // recursion if the path is longer
            retval = walkBrowsePath(server, session, next, path, pathindex + 1,
                                    targets, targets_size, target_count);
        } else {
            // add the browsetarget
            if(*target_count >= *targets_size) {
                UA_BrowsePathTarget *newtargets;
                newtargets = UA_realloc(targets, sizeof(UA_BrowsePathTarget) * (*targets_size) * 2);
                if(!newtargets) {
                    retval = UA_STATUSCODE_BADOUTOFMEMORY;
                    break;
                }
                *targets = newtargets;
                *targets_size *= 2;
            }

            UA_BrowsePathTarget *res = *targets;
            UA_ExpandedNodeId_init(&res[*target_count].targetId);
            retval = UA_NodeId_copy(&next->nodeId, &res[*target_count].targetId.nodeId);
            if(retval != UA_STATUSCODE_GOOD)
                break;
            res[*target_count].remainingPathIndex = UA_UINT32_MAX;
            *target_count += 1;
        }
    }

    if(!all_refs && elem->includeSubtypes)
        UA_Array_delete(reftypes, reftypes_count, &UA_TYPES[UA_TYPES_NODEID]);
    return retval;
}

void Service_TranslateBrowsePathsToNodeIds_single(UA_Server *server, UA_Session *session,
                                                  const UA_BrowsePath *path, UA_BrowsePathResult *result) {
    if(path->relativePath.elementsSize <= 0) {
        result->statusCode = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }
        
    size_t arraySize = 10;
    result->targets = UA_malloc(sizeof(UA_BrowsePathTarget) * arraySize);
    if(!result->targets) {
        result->statusCode = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }
    result->targetsSize = 0;
    const UA_Node *firstNode = UA_NodeStore_get(server->nodestore, &path->startingNode);
    if(!firstNode) {
        result->statusCode = UA_STATUSCODE_BADNODEIDUNKNOWN;
        UA_free(result->targets);
        result->targets = NULL;
        return;
    }
    result->statusCode = walkBrowsePath(server, session, firstNode, &path->relativePath, 0,
                                        &result->targets, &arraySize, &result->targetsSize);
    if(result->targetsSize == 0 && result->statusCode == UA_STATUSCODE_GOOD)
        result->statusCode = UA_STATUSCODE_BADNOMATCH;
    if(result->statusCode != UA_STATUSCODE_GOOD) {
        UA_Array_delete(result->targets, result->targetsSize, &UA_TYPES[UA_TYPES_BROWSEPATHTARGET]);
        result->targets = NULL;
        result->targetsSize = 0;
    }
}

void Service_TranslateBrowsePathsToNodeIds(UA_Server *server, UA_Session *session,
                                           const UA_TranslateBrowsePathsToNodeIdsRequest *request,
                                           UA_TranslateBrowsePathsToNodeIdsResponse *response) {
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SESSION,
                 "Processing TranslateBrowsePathsToNodeIdsRequest for Session (ns=%i,i=%i)",
                 session->sessionId.namespaceIndex, session->sessionId.identifier.numeric);
	if(request->browsePathsSize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }

    size_t size = request->browsePathsSize;
    response->results = UA_Array_new(size, &UA_TYPES[UA_TYPES_BROWSEPATHRESULT]);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }

#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
#ifdef NO_ALLOCA
    UA_Boolean isExternal[size];
    UA_UInt32 indices[size];
#else
    UA_Boolean *isExternal = UA_alloca(sizeof(UA_Boolean) * size);
    UA_UInt32 *indices = UA_alloca(sizeof(UA_UInt32) * size);
#endif /*NO_ALLOCA */
    memset(isExternal, false, sizeof(UA_Boolean) * size);
    for(size_t j = 0; j < server->externalNamespacesSize; j++) {
    	size_t indexSize = 0;
    	for(size_t i = 0;i < size;i++) {
    		if(request->browsePaths[i].startingNode.namespaceIndex != server->externalNamespaces[j].index)
    			continue;
    		isExternal[i] = true;
    		indices[indexSize] = i;
    		indexSize++;
    	}
    	if(indexSize == 0)
    		continue;
    	UA_ExternalNodeStore *ens = &server->externalNamespaces[j].externalNodeStore;
    	ens->translateBrowsePathsToNodeIds(ens->ensHandle, &request->requestHeader, request->browsePaths,
    			indices, indexSize, response->results, response->diagnosticInfos);
    }
#endif

    response->resultsSize = size;
    for(size_t i = 0; i < size; i++) {
#ifdef UA_ENABLE_EXTERNAL_NAMESPACES
    	if(!isExternal[i])
#endif
    		Service_TranslateBrowsePathsToNodeIds_single(server, session, &request->browsePaths[i],
                                                         &response->results[i]);
    }
}

void Service_RegisterNodes(UA_Server *server, UA_Session *session, const UA_RegisterNodesRequest *request,
                           UA_RegisterNodesResponse *response) {
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SESSION,
                 "Processing RegisterNodesRequest for Session (ns=%i,i=%i)",
                 session->sessionId.namespaceIndex, session->sessionId.identifier.numeric);

	//TODO: hang the nodeids to the session if really needed
	response->responseHeader.timestamp = UA_DateTime_now();
    if(request->nodesToRegisterSize <= 0)
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
    else {
        response->responseHeader.serviceResult =
            UA_Array_copy(request->nodesToRegister, request->nodesToRegisterSize,
                          (void**)&response->registeredNodeIds, &UA_TYPES[UA_TYPES_NODEID]);
        if(response->responseHeader.serviceResult == UA_STATUSCODE_GOOD)
            response->registeredNodeIdsSize = request->nodesToRegisterSize;
    }
}

void Service_UnregisterNodes(UA_Server *server, UA_Session *session, const UA_UnregisterNodesRequest *request,
                             UA_UnregisterNodesResponse *response) {
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SESSION,
                 "Processing UnRegisterNodesRequest for Session (ns=%i,i=%i)",
                 session->sessionId.namespaceIndex, session->sessionId.identifier.numeric);

	//TODO: remove the nodeids from the session if really needed
	response->responseHeader.timestamp = UA_DateTime_now();
	if(request->nodesToUnregisterSize==0)
		response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/client/ua_client.c" ***********************************/


/*********************/
/* Create and Delete */
/*********************/

static void UA_Client_init(UA_Client* client, UA_ClientConfig config) {
    client->state = UA_CLIENTSTATE_READY;
    UA_Connection_init(&client->connection);
    UA_SecureChannel_init(&client->channel);
    client->channel.connection = &client->connection;
    UA_String_init(&client->endpointUrl);
    client->requestId = 0;

    client->authenticationMethod = UA_CLIENTAUTHENTICATION_NONE;
    UA_String_init(&client->username);
    UA_String_init(&client->password);

    UA_NodeId_init(&client->authenticationToken);
    client->requestHandle = 0;

    client->config = config;
    client->scRenewAt = 0;

#ifdef UA_ENABLE_SUBSCRIPTIONS
    client->monitoredItemHandles = 0;
    LIST_INIT(&client->pendingNotificationsAcks);
    LIST_INIT(&client->subscriptions);
#endif
}

UA_Client * UA_Client_new(UA_ClientConfig config) {
    UA_Client *client = UA_calloc(1, sizeof(UA_Client));
    if(!client)
        return NULL;

    UA_Client_init(client, config);
    return client;
}

static void UA_Client_deleteMembers(UA_Client* client) {
    UA_Client_disconnect(client);
    UA_Connection_deleteMembers(&client->connection);
    UA_SecureChannel_deleteMembersCleanup(&client->channel);
    if(client->endpointUrl.data)
        UA_String_deleteMembers(&client->endpointUrl);
    UA_UserTokenPolicy_deleteMembers(&client->token);
    if(client->username.data)
        UA_String_deleteMembers(&client->username);
    if(client->password.data)
           UA_String_deleteMembers(&client->password);
#ifdef UA_ENABLE_SUBSCRIPTIONS
    UA_Client_NotificationsAckNumber *n, *tmp;
    LIST_FOREACH_SAFE(n, &client->pendingNotificationsAcks, listEntry, tmp) {
        LIST_REMOVE(n, listEntry);
        free(n);
    }
    UA_Client_Subscription *sub, *tmps;
    LIST_FOREACH_SAFE(sub, &client->subscriptions, listEntry, tmps) {
        LIST_REMOVE(sub, listEntry);
        UA_Client_MonitoredItem *mon, *tmpmon;
        LIST_FOREACH_SAFE(mon, &sub->MonitoredItems, listEntry, tmpmon) {
            UA_Client_Subscriptions_removeMonitoredItem(client, sub->SubscriptionID,
                                                        mon->MonitoredItemId);
        }
        free(sub);
    }
#endif
}

void UA_Client_reset(UA_Client* client){
    UA_Client_deleteMembers(client);
    UA_Client_init(client, client->config);
}

void UA_Client_delete(UA_Client* client){
    if(client->state != UA_CLIENTSTATE_READY)
        UA_Client_deleteMembers(client);
    UA_free(client);
}

UA_ClientState UA_EXPORT UA_Client_getState(UA_Client *client) {
    if (client == NULL)
        return UA_CLIENTSTATE_ERRORED;
    return client->state;
}

/*************************/
/* Manage the Connection */
/*************************/

static UA_StatusCode HelAckHandshake(UA_Client *client) {
    UA_TcpMessageHeader messageHeader;
    messageHeader.messageTypeAndChunkType = UA_CHUNKTYPE_FINAL + UA_MESSAGETYPE_HEL;

    UA_TcpHelloMessage hello;
    UA_String_copy(&client->endpointUrl, &hello.endpointUrl); /* must be less than 4096 bytes */

    UA_Connection *conn = &client->connection;
    hello.maxChunkCount = conn->localConf.maxChunkCount;
    hello.maxMessageSize = conn->localConf.maxMessageSize;
    hello.protocolVersion = conn->localConf.protocolVersion;
    hello.receiveBufferSize = conn->localConf.recvBufferSize;
    hello.sendBufferSize = conn->localConf.sendBufferSize;

    UA_ByteString message;
    UA_StatusCode retval;
    retval = client->connection.getSendBuffer(&client->connection, client->connection.remoteConf.recvBufferSize, &message);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;

    size_t offset = 8;
    retval |= UA_TcpHelloMessage_encodeBinary(&hello, &message, &offset);
    messageHeader.messageSize = (UA_UInt32)offset;
    offset = 0;
    retval |= UA_TcpMessageHeader_encodeBinary(&messageHeader, &message, &offset);
    UA_TcpHelloMessage_deleteMembers(&hello);
    if(retval != UA_STATUSCODE_GOOD) {
        client->connection.releaseSendBuffer(&client->connection, &message);
        return retval;
    }

    message.length = messageHeader.messageSize;
    retval = client->connection.send(&client->connection, &message);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(client->config.logger, UA_LOGCATEGORY_NETWORK, "Sending HEL failed");
        return retval;
    }
    UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_NETWORK, "Sent HEL message");

    UA_ByteString reply;
    UA_ByteString_init(&reply);
    UA_Boolean realloced = false;
    do {
        retval = client->connection.recv(&client->connection, &reply, client->config.timeout);
        retval |= UA_Connection_completeMessages(&client->connection, &reply, &realloced);
        if(retval != UA_STATUSCODE_GOOD) {
            UA_LOG_INFO(client->config.logger, UA_LOGCATEGORY_NETWORK, "Receiving ACK message failed");
            return retval;
        }
    } while(reply.length == 0);

    offset = 0;
    UA_TcpMessageHeader_decodeBinary(&reply, &offset, &messageHeader);
    UA_TcpAcknowledgeMessage ackMessage;
    retval = UA_TcpAcknowledgeMessage_decodeBinary(&reply, &offset, &ackMessage);
    if(!realloced)
        client->connection.releaseRecvBuffer(&client->connection, &reply);
    else
        UA_ByteString_deleteMembers(&reply);

    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_INFO(client->config.logger, UA_LOGCATEGORY_NETWORK, "Decoding ACK message failed");
        return retval;
    }
    UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_NETWORK, "Received ACK message");

    conn->remoteConf.maxChunkCount = ackMessage.maxChunkCount;
    conn->remoteConf.maxMessageSize = ackMessage.maxMessageSize;
    conn->remoteConf.protocolVersion = ackMessage.protocolVersion;
    conn->remoteConf.recvBufferSize = ackMessage.receiveBufferSize;
    conn->remoteConf.sendBufferSize = ackMessage.sendBufferSize;
    conn->state = UA_CONNECTION_ESTABLISHED;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode SecureChannelHandshake(UA_Client *client, UA_Boolean renew) {
    /* Check if sc is still valid */
    if(renew && client->scRenewAt - UA_DateTime_now() > 0)
        return UA_STATUSCODE_GOOD;

    UA_Connection *c = &client->connection;
    if(c->state != UA_CONNECTION_ESTABLISHED)
        return UA_STATUSCODE_BADSERVERNOTCONNECTED;

    UA_SecureConversationMessageHeader messageHeader;
    messageHeader.messageHeader.messageTypeAndChunkType = UA_MESSAGETYPE_OPN + UA_CHUNKTYPE_FINAL;
    if(renew){
        messageHeader.secureChannelId = client->channel.securityToken.channelId;
    }else{
        messageHeader.secureChannelId = 0;
    }

    UA_SequenceHeader seqHeader;
    seqHeader.sequenceNumber = ++client->channel.sequenceNumber;
    seqHeader.requestId = ++client->requestId;

    UA_AsymmetricAlgorithmSecurityHeader asymHeader;
    UA_AsymmetricAlgorithmSecurityHeader_init(&asymHeader);
    asymHeader.securityPolicyUri = UA_STRING_ALLOC("http://opcfoundation.org/UA/SecurityPolicy#None");

    /* id of opensecurechannelrequest */
    UA_NodeId requestType = UA_NODEID_NUMERIC(0, UA_NS0ID_OPENSECURECHANNELREQUEST + UA_ENCODINGOFFSET_BINARY);

    UA_OpenSecureChannelRequest opnSecRq;
    UA_OpenSecureChannelRequest_init(&opnSecRq);
    opnSecRq.requestHeader.timestamp = UA_DateTime_now();
    opnSecRq.requestHeader.authenticationToken = client->authenticationToken;
    opnSecRq.requestedLifetime = client->config.secureChannelLifeTime;
    if(renew) {
        opnSecRq.requestType = UA_SECURITYTOKENREQUESTTYPE_RENEW;
        UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_SECURECHANNEL, "Requesting to renew the SecureChannel");
    } else {
        opnSecRq.requestType = UA_SECURITYTOKENREQUESTTYPE_ISSUE;
        UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_SECURECHANNEL, "Requesting to open a SecureChannel");
    }

    UA_ByteString_copy(&client->channel.clientNonce, &opnSecRq.clientNonce);
    opnSecRq.securityMode = UA_MESSAGESECURITYMODE_NONE;

    UA_ByteString message;
    UA_StatusCode retval = c->getSendBuffer(c, c->remoteConf.recvBufferSize, &message);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_AsymmetricAlgorithmSecurityHeader_deleteMembers(&asymHeader);
        UA_OpenSecureChannelRequest_deleteMembers(&opnSecRq);
        return retval;
    }

    size_t offset = 12;
    retval = UA_AsymmetricAlgorithmSecurityHeader_encodeBinary(&asymHeader, &message, &offset);
    retval |= UA_SequenceHeader_encodeBinary(&seqHeader, &message, &offset);
    retval |= UA_NodeId_encodeBinary(&requestType, &message, &offset);
    retval |= UA_OpenSecureChannelRequest_encodeBinary(&opnSecRq, &message, &offset);
    messageHeader.messageHeader.messageSize = (UA_UInt32)offset;
    offset = 0;
    retval |= UA_SecureConversationMessageHeader_encodeBinary(&messageHeader, &message, &offset);

    UA_AsymmetricAlgorithmSecurityHeader_deleteMembers(&asymHeader);
    UA_OpenSecureChannelRequest_deleteMembers(&opnSecRq);
    if(retval != UA_STATUSCODE_GOOD) {
        client->connection.releaseSendBuffer(&client->connection, &message);
        return retval;
    }

    message.length = messageHeader.messageHeader.messageSize;
    retval = client->connection.send(&client->connection, &message);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;

    UA_ByteString reply;
    UA_ByteString_init(&reply);
    UA_Boolean realloced = false;
    do {
        retval = c->recv(c, &reply, client->config.timeout);
        retval |= UA_Connection_completeMessages(c, &reply, &realloced);
        if(retval != UA_STATUSCODE_GOOD) {
            UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_SECURECHANNEL,
                         "Receiving OpenSecureChannelResponse failed");
            return retval;
        }
    } while(reply.length == 0);

    offset = 0;
    UA_SecureConversationMessageHeader_decodeBinary(&reply, &offset, &messageHeader);
    UA_AsymmetricAlgorithmSecurityHeader_decodeBinary(&reply, &offset, &asymHeader);
    UA_SequenceHeader_decodeBinary(&reply, &offset, &seqHeader);
    UA_NodeId_decodeBinary(&reply, &offset, &requestType);
    UA_NodeId expectedRequest = UA_NODEID_NUMERIC(0, UA_NS0ID_OPENSECURECHANNELRESPONSE +
                                                  UA_ENCODINGOFFSET_BINARY);
    if(!UA_NodeId_equal(&requestType, &expectedRequest)) {
        UA_ByteString_deleteMembers(&reply);
        UA_AsymmetricAlgorithmSecurityHeader_deleteMembers(&asymHeader);
        UA_NodeId_deleteMembers(&requestType);
        UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_CLIENT,
                     "Reply answers the wrong request. Expected OpenSecureChannelResponse.");
        return UA_STATUSCODE_BADINTERNALERROR;
    }

    UA_OpenSecureChannelResponse response;
    UA_OpenSecureChannelResponse_init(&response);
    retval = UA_OpenSecureChannelResponse_decodeBinary(&reply, &offset, &response);
    if(!realloced)
        c->releaseRecvBuffer(c, &reply);
    else
        UA_ByteString_deleteMembers(&reply);
        
    if(retval != UA_STATUSCODE_GOOD) {
        UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_SECURECHANNEL, "Decoding OpenSecureChannelResponse failed");
        UA_AsymmetricAlgorithmSecurityHeader_deleteMembers(&asymHeader);
        UA_OpenSecureChannelResponse_init(&response);
        response.responseHeader.serviceResult = retval;
        return retval;
    }

    //response.securityToken.revisedLifetime is UInt32 we need to cast it to DateTime=Int64
    //we take 75% of lifetime to start renewing as described in standard
    client->scRenewAt = UA_DateTime_now() +
        (UA_DateTime)(response.securityToken.revisedLifetime * (UA_Double)UA_MSEC_TO_DATETIME * 0.75);
    retval = response.responseHeader.serviceResult;

    if(retval != UA_STATUSCODE_GOOD)
        UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_SECURECHANNEL, "SecureChannel could not be opened / renewed");
    else {
        UA_ChannelSecurityToken_deleteMembers(&client->channel.securityToken);
        UA_ChannelSecurityToken_copy(&response.securityToken, &client->channel.securityToken);
        /* if the handshake is repeated, replace the old nonce */
        UA_ByteString_deleteMembers(&client->channel.serverNonce);
        UA_ByteString_copy(&response.serverNonce, &client->channel.serverNonce);
        if(renew)
            UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_SECURECHANNEL, "SecureChannel renewed");
        else
            UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_SECURECHANNEL, "SecureChannel opened");
    }
    UA_OpenSecureChannelResponse_deleteMembers(&response);
    UA_AsymmetricAlgorithmSecurityHeader_deleteMembers(&asymHeader);
    return retval;
}

static UA_StatusCode ActivateSession(UA_Client *client) {
    UA_ActivateSessionRequest request;
    UA_ActivateSessionRequest_init(&request);

    request.requestHeader.requestHandle = 2; //TODO: is it a magic number?
    request.requestHeader.authenticationToken = client->authenticationToken;
    request.requestHeader.timestamp = UA_DateTime_now();
    request.requestHeader.timeoutHint = 600000;

    //manual ExtensionObject encoding of the identityToken
    if(client->authenticationMethod == UA_CLIENTAUTHENTICATION_NONE){
        UA_AnonymousIdentityToken* identityToken = UA_malloc(sizeof(UA_AnonymousIdentityToken));
        UA_AnonymousIdentityToken_init(identityToken);
        UA_String_copy(&client->token.policyId, &identityToken->policyId);
        request.userIdentityToken.encoding = UA_EXTENSIONOBJECT_DECODED;
        request.userIdentityToken.content.decoded.type = &UA_TYPES[UA_TYPES_ANONYMOUSIDENTITYTOKEN];
        request.userIdentityToken.content.decoded.data = identityToken;
    }else{
        UA_UserNameIdentityToken* identityToken = UA_malloc(sizeof(UA_UserNameIdentityToken));
        UA_UserNameIdentityToken_init(identityToken);
        UA_String_copy(&client->token.policyId, &identityToken->policyId);
        UA_String_copy(&client->username, &identityToken->userName);
        UA_String_copy(&client->password, &identityToken->password);
        request.userIdentityToken.encoding = UA_EXTENSIONOBJECT_DECODED;
        request.userIdentityToken.content.decoded.type = &UA_TYPES[UA_TYPES_USERNAMEIDENTITYTOKEN];
        request.userIdentityToken.content.decoded.data = identityToken;
    }

    UA_ActivateSessionResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_ACTIVATESESSIONREQUEST],
                        &response, &UA_TYPES[UA_TYPES_ACTIVATESESSIONRESPONSE]);

    UA_ActivateSessionRequest_deleteMembers(&request);
    UA_ActivateSessionResponse_deleteMembers(&response);
    return response.responseHeader.serviceResult; // not deleted
}

/**
 * Gets a list of endpoints
 * Memory is allocated for endpointDescription array
 */
static UA_StatusCode
GetEndpoints(UA_Client *client, size_t* endpointDescriptionsSize, UA_EndpointDescription** endpointDescriptions) {
    UA_GetEndpointsRequest request;
    UA_GetEndpointsRequest_init(&request);
    request.requestHeader.authenticationToken = client->authenticationToken;
    request.requestHeader.timestamp = UA_DateTime_now();
    request.requestHeader.timeoutHint = 10000;
    request.endpointUrl = client->endpointUrl; // assume the endpointurl outlives the service call
    
    UA_GetEndpointsResponse response;
    UA_GetEndpointsResponse_init(&response);
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_GETENDPOINTSREQUEST],
                        &response, &UA_TYPES[UA_TYPES_GETENDPOINTSRESPONSE]);

    if(response.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
        UA_LOG_ERROR(client->config.logger, UA_LOGCATEGORY_CLIENT, "GetEndpointRequest failed");
        UA_GetEndpointsResponse_deleteMembers(&response);
        return response.responseHeader.serviceResult;
    }

    *endpointDescriptionsSize = response.endpointsSize;
    *endpointDescriptions = UA_Array_new(response.endpointsSize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
    for(size_t i=0;i<response.endpointsSize;i++)
        UA_EndpointDescription_copy(&response.endpoints[i], &(*endpointDescriptions)[i]);
    UA_GetEndpointsResponse_deleteMembers(&response);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode EndpointsHandshake(UA_Client *client) {
    UA_EndpointDescription* endpointArray = NULL;
    size_t endpointArraySize = 0;
    UA_StatusCode retval = GetEndpoints(client, &endpointArraySize, &endpointArray);

    UA_Boolean endpointFound = false;
    UA_Boolean tokenFound = false;
    UA_String securityNone = UA_STRING("http://opcfoundation.org/UA/SecurityPolicy#None");
    UA_String binaryTransport = UA_STRING("http://opcfoundation.org/UA-Profile/Transport/uatcp-uasc-uabinary");

    //TODO: compare endpoint information with client->endpointUri
    for(size_t i = 0; i < endpointArraySize; i++) {
        UA_EndpointDescription* endpoint = &endpointArray[i];
        /* look out for binary transport endpoints */
        //NODE: Siemens returns empty ProfileUrl, we will accept it as binary
        if(endpoint->transportProfileUri.length!=0 && !UA_String_equal(&endpoint->transportProfileUri, &binaryTransport))
            continue;
        /* look out for an endpoint without security */
        if(!UA_String_equal(&endpoint->securityPolicyUri, &securityNone))
            continue;
        endpointFound = true;
        /* endpoint with no security found */
        /* look for a user token policy with an anonymous token */
        for(size_t j = 0; j < endpoint->userIdentityTokensSize; ++j) {
            UA_UserTokenPolicy* userToken = &endpoint->userIdentityTokens[j];
            //anonymous authentication
            if(client->authenticationMethod == UA_CLIENTAUTHENTICATION_NONE){
                if(userToken->tokenType != UA_USERTOKENTYPE_ANONYMOUS)
                    continue;
            }else{
            //username authentication
                if(userToken->tokenType != UA_USERTOKENTYPE_USERNAME)
                    continue;
            }
            tokenFound = true;
            UA_UserTokenPolicy_copy(userToken, &client->token);
            break;
        }
    }

    UA_Array_delete(endpointArray, endpointArraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);

    if(!endpointFound) {
        UA_LOG_ERROR(client->config.logger, UA_LOGCATEGORY_CLIENT, "No suitable endpoint found");
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    if(!tokenFound) {
        UA_LOG_ERROR(client->config.logger, UA_LOGCATEGORY_CLIENT, "No anonymous token found");
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    return retval;
}

static UA_StatusCode SessionHandshake(UA_Client *client) {
    UA_CreateSessionRequest request;
    UA_CreateSessionRequest_init(&request);

    // todo: is this needed for all requests?
    UA_NodeId_copy(&client->authenticationToken, &request.requestHeader.authenticationToken);
    request.requestHeader.timestamp = UA_DateTime_now();
    request.requestHeader.timeoutHint = 10000;
    UA_ByteString_copy(&client->channel.clientNonce, &request.clientNonce);
    request.requestedSessionTimeout = 1200000;
    request.maxResponseMessageSize = UA_INT32_MAX;

    UA_CreateSessionResponse response;
    UA_CreateSessionResponse_init(&response);
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_CREATESESSIONREQUEST],
                        &response, &UA_TYPES[UA_TYPES_CREATESESSIONRESPONSE]);

    UA_NodeId_copy(&response.authenticationToken, &client->authenticationToken);

    UA_CreateSessionRequest_deleteMembers(&request);
    UA_CreateSessionResponse_deleteMembers(&response);
    return response.responseHeader.serviceResult; // not deleted
}

static UA_StatusCode CloseSession(UA_Client *client) {
    UA_CloseSessionRequest request;
    UA_CloseSessionRequest_init(&request);

    request.requestHeader.timestamp = UA_DateTime_now();
    request.requestHeader.timeoutHint = 10000;
    request.deleteSubscriptions = true;
    UA_NodeId_copy(&client->authenticationToken, &request.requestHeader.authenticationToken);
    UA_CloseSessionResponse response;
    __UA_Client_Service(client, &request, &UA_TYPES[UA_TYPES_CLOSESESSIONREQUEST],
                        &response, &UA_TYPES[UA_TYPES_CLOSESESSIONRESPONSE]);

    UA_CloseSessionRequest_deleteMembers(&request);
    UA_CloseSessionResponse_deleteMembers(&response);
    return response.responseHeader.serviceResult; // not deleted
}

static UA_StatusCode CloseSecureChannel(UA_Client *client) {
    UA_SecureChannel *channel = &client->channel;
    UA_CloseSecureChannelRequest request;
    UA_CloseSecureChannelRequest_init(&request);
    request.requestHeader.requestHandle = 1; //TODO: magic number?
    request.requestHeader.timestamp = UA_DateTime_now();
    request.requestHeader.timeoutHint = 10000;
    request.requestHeader.authenticationToken = client->authenticationToken;

    UA_SecureConversationMessageHeader msgHeader;
    msgHeader.messageHeader.messageTypeAndChunkType = UA_MESSAGETYPE_CLO + UA_CHUNKTYPE_FINAL;
    msgHeader.secureChannelId = client->channel.securityToken.channelId;

    UA_SymmetricAlgorithmSecurityHeader symHeader;
    symHeader.tokenId = channel->securityToken.tokenId;
    
    UA_SequenceHeader seqHeader;
    seqHeader.sequenceNumber = ++channel->sequenceNumber;
    seqHeader.requestId = ++client->requestId;

    UA_NodeId typeId = UA_NODEID_NUMERIC(0, UA_NS0ID_CLOSESECURECHANNELREQUEST + UA_ENCODINGOFFSET_BINARY);

    UA_ByteString message;
    UA_Connection *c = &client->connection;
    UA_StatusCode retval = c->getSendBuffer(c, c->remoteConf.recvBufferSize, &message);
    if(retval != UA_STATUSCODE_GOOD)
        return retval;

    size_t offset = 12;
    retval |= UA_SymmetricAlgorithmSecurityHeader_encodeBinary(&symHeader, &message, &offset);
    retval |= UA_SequenceHeader_encodeBinary(&seqHeader, &message, &offset);
    retval |= UA_NodeId_encodeBinary(&typeId, &message, &offset);
    retval |= UA_encodeBinary(&request, &UA_TYPES[UA_TYPES_CLOSESECURECHANNELREQUEST],NULL,NULL, &message, &offset);

    msgHeader.messageHeader.messageSize = (UA_UInt32)offset;
    offset = 0;
    retval |= UA_SecureConversationMessageHeader_encodeBinary(&msgHeader, &message, &offset);

    if(retval != UA_STATUSCODE_GOOD) {
        client->connection.releaseSendBuffer(&client->connection, &message);
        return retval;
    }
        
    message.length = msgHeader.messageHeader.messageSize;
    retval = client->connection.send(&client->connection, &message);
    return retval;
}

UA_StatusCode
UA_Client_getEndpoints(UA_Client *client, const char *serverUrl,
                       size_t* endpointDescriptionsSize,
                       UA_EndpointDescription** endpointDescriptions) {
    if(client->state == UA_CLIENTSTATE_CONNECTED)
        return UA_STATUSCODE_GOOD;
    if(client->state == UA_CLIENTSTATE_ERRORED)
        UA_Client_reset(client);


    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    client->connection = client->config.connectionFunc(UA_ConnectionConfig_standard, serverUrl, client->config.logger);
    if(client->connection.state != UA_CONNECTION_OPENING) {
        retval = UA_STATUSCODE_BADCONNECTIONCLOSED;
        goto cleanup;
    }

    client->endpointUrl = UA_STRING_ALLOC(serverUrl);
    if(!client->endpointUrl.data) {
        retval = UA_STATUSCODE_BADOUTOFMEMORY;
        goto cleanup;
    }
    
    client->connection.localConf = client->config.localConnectionConfig;
    retval = HelAckHandshake(client);
    if(retval == UA_STATUSCODE_GOOD)
        retval = SecureChannelHandshake(client, false);
    if(retval == UA_STATUSCODE_GOOD)
        retval = GetEndpoints(client, endpointDescriptionsSize, endpointDescriptions);
    
    /* always cleanup */
    cleanup:
    UA_Client_reset(client);
    return retval;
}

UA_StatusCode
UA_Client_connect_username(UA_Client *client, const char *endpointUrl,
                           const char *username, const char *password){
    client->authenticationMethod=UA_CLIENTAUTHENTICATION_USERNAME;
    client->username = UA_STRING_ALLOC(username);
    client->password = UA_STRING_ALLOC(password);
    return UA_Client_connect(client, endpointUrl);
}


UA_StatusCode
UA_Client_connect(UA_Client *client, const char *endpointUrl) {
    if(client->state == UA_CLIENTSTATE_CONNECTED)
        return UA_STATUSCODE_GOOD;
    if(client->state == UA_CLIENTSTATE_ERRORED) {
        UA_Client_reset(client);
    }

    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    client->connection = client->config.connectionFunc(UA_ConnectionConfig_standard, endpointUrl, client->config.logger);
    if(client->connection.state != UA_CONNECTION_OPENING) {
        retval = UA_STATUSCODE_BADCONNECTIONCLOSED;
        goto cleanup;
    }

    client->endpointUrl = UA_STRING_ALLOC(endpointUrl);
    if(!client->endpointUrl.data) {
        retval = UA_STATUSCODE_BADOUTOFMEMORY;
        goto cleanup;
    }

    client->connection.localConf = client->config.localConnectionConfig;
    retval = HelAckHandshake(client);
    if(retval == UA_STATUSCODE_GOOD)
        retval = SecureChannelHandshake(client, false);
    if(retval == UA_STATUSCODE_GOOD)
        retval = EndpointsHandshake(client);
    if(retval == UA_STATUSCODE_GOOD)
        retval = SessionHandshake(client);
    if(retval == UA_STATUSCODE_GOOD)
        retval = ActivateSession(client);
    if(retval == UA_STATUSCODE_GOOD) {
        client->connection.state = UA_CONNECTION_ESTABLISHED;
        client->state = UA_CLIENTSTATE_CONNECTED;
    } else {
        goto cleanup;
    }
    return retval;

    cleanup:
    UA_Client_reset(client);
    return retval;
}

UA_StatusCode UA_Client_disconnect(UA_Client *client) {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    //is a session established?
    if(client->state == UA_CLIENTSTATE_CONNECTED && client->channel.connection->state == UA_CONNECTION_ESTABLISHED)
        retval = CloseSession(client);
    //is a secure channel established?
    if(retval == UA_STATUSCODE_GOOD && client->channel.connection->state == UA_CONNECTION_ESTABLISHED)
        retval = CloseSecureChannel(client);
    return retval;
}

UA_StatusCode UA_Client_manuallyRenewSecureChannel(UA_Client *client) {
    UA_StatusCode retval = SecureChannelHandshake(client, true);
    if(retval == UA_STATUSCODE_GOOD)
      client->state = UA_CLIENTSTATE_CONNECTED;
    return retval;
}

/****************/
/* Raw Services */
/****************/

void __UA_Client_Service(UA_Client *client, const void *r, const UA_DataType *requestType,
                         void *response, const UA_DataType *responseType) {
    /* Requests always begin witih a RequestHeader, therefore we can cast. */
    UA_RequestHeader *request = (void*)(uintptr_t)r;
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_init(response, responseType);
    UA_ResponseHeader *respHeader = (UA_ResponseHeader*)response;
    
    /* make sure we have a valid session */
    retval = UA_Client_manuallyRenewSecureChannel(client);
    if(retval != UA_STATUSCODE_GOOD) {
        respHeader->serviceResult = retval;
        client->state = UA_CLIENTSTATE_ERRORED;
        return;
    }

    /* handling request parameters */
    UA_NodeId_copy(&client->authenticationToken, &request->authenticationToken);
    request->timestamp = UA_DateTime_now();
    request->requestHandle = ++client->requestHandle;

    /* Send the request */
    UA_UInt32 requestId = ++client->requestId;
    UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_CLIENT,
                 "Sending a request of type %i", requestType->typeId.identifier.numeric);
    retval = UA_SecureChannel_sendBinaryMessage(&client->channel, requestId, request, requestType);
    if(retval != UA_STATUSCODE_GOOD) {
        if(retval == UA_STATUSCODE_BADENCODINGLIMITSEXCEEDED)
            respHeader->serviceResult = UA_STATUSCODE_BADREQUESTTOOLARGE;
        else
            respHeader->serviceResult = retval;
        client->state = UA_CLIENTSTATE_ERRORED;
        return;
    }

    /* Retrieve the response */
    // Todo: push this into the generic securechannel implementation for client and server
    UA_ByteString reply;
    UA_ByteString_init(&reply);
    UA_Boolean realloced = false;
    do {
        retval = client->connection.recv(&client->connection, &reply, client->config.timeout);
        retval |= UA_Connection_completeMessages(&client->connection, &reply, &realloced);
        if(retval != UA_STATUSCODE_GOOD) {
            respHeader->serviceResult = retval;
            client->state = UA_CLIENTSTATE_ERRORED;
            return;
        }
    } while(!reply.data);

    size_t offset = 0;
    UA_SecureConversationMessageHeader msgHeader;
    retval |= UA_SecureConversationMessageHeader_decodeBinary(&reply, &offset, &msgHeader);
    UA_SymmetricAlgorithmSecurityHeader symHeader;
    retval |= UA_SymmetricAlgorithmSecurityHeader_decodeBinary(&reply, &offset, &symHeader);
    UA_SequenceHeader seqHeader;
    retval |= UA_SequenceHeader_decodeBinary(&reply, &offset, &seqHeader);
    UA_NodeId responseId;
    retval |= UA_NodeId_decodeBinary(&reply, &offset, &responseId);
    UA_NodeId expectedNodeId = UA_NODEID_NUMERIC(0, responseType->typeId.identifier.numeric +
                                                 UA_ENCODINGOFFSET_BINARY);

    if(retval != UA_STATUSCODE_GOOD)
        goto finish;

    /* Todo: we need to demux responses since a publish responses may come at any time */
    if(!UA_NodeId_equal(&responseId, &expectedNodeId) || seqHeader.requestId != requestId) {
        if(responseId.identifier.numeric != UA_NS0ID_SERVICEFAULT + UA_ENCODINGOFFSET_BINARY) {
            UA_LOG_ERROR(client->config.logger, UA_LOGCATEGORY_CLIENT,
                         "Reply answers the wrong request. Expected ns=%i,i=%i. But retrieved ns=%i,i=%i",
                         expectedNodeId.namespaceIndex, expectedNodeId.identifier.numeric,
                         responseId.namespaceIndex, responseId.identifier.numeric);
            respHeader->serviceResult = UA_STATUSCODE_BADINTERNALERROR;
        } else
            retval = UA_decodeBinary(&reply, &offset, respHeader, &UA_TYPES[UA_TYPES_SERVICEFAULT]);
        goto finish;
    } 
    
    retval = UA_decodeBinary(&reply, &offset, response, responseType);
    if(retval == UA_STATUSCODE_BADENCODINGLIMITSEXCEEDED)
        retval = UA_STATUSCODE_BADRESPONSETOOLARGE;

 finish:
    UA_SymmetricAlgorithmSecurityHeader_deleteMembers(&symHeader);
    if(!realloced)
        client->connection.releaseRecvBuffer(&client->connection, &reply);
    else
        UA_ByteString_deleteMembers(&reply);

    if(retval != UA_STATUSCODE_GOOD){
        UA_LOG_INFO(client->config.logger, UA_LOGCATEGORY_CLIENT, "Error receiving the response");
        client->state = UA_CLIENTSTATE_FAULTED;
        respHeader->serviceResult = retval;
    } else {
      client->state = UA_CLIENTSTATE_CONNECTED;
    }
    UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_CLIENT,
                 "Received a response of type %i", responseId.identifier.numeric);
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/client/ua_client_highlevel.c" ***********************************/


UA_StatusCode
UA_Client_NamespaceGetIndex(UA_Client *client, UA_String *namespaceUri, UA_UInt16 *namespaceIndex) {
	UA_ReadRequest request;
	UA_ReadRequest_init(&request);
    UA_ReadValueId id;
	id.attributeId = UA_ATTRIBUTEID_VALUE;
	id.nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER_NAMESPACEARRAY);
	request.nodesToRead = &id;
	request.nodesToReadSize = 1;

	UA_ReadResponse response = UA_Client_Service_read(client, request);

	UA_StatusCode retval = UA_STATUSCODE_GOOD;
    if(response.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
        retval = response.responseHeader.serviceResult;
    else if(response.resultsSize != 1 || !response.results[0].hasValue)
        retval = UA_STATUSCODE_BADNODEATTRIBUTESINVALID;
    else if(response.results[0].value.type != &UA_TYPES[UA_TYPES_STRING])
        retval = UA_STATUSCODE_BADTYPEMISMATCH;

    if(retval != UA_STATUSCODE_GOOD) {
        UA_ReadResponse_deleteMembers(&response);
        return retval;
    }

    retval = UA_STATUSCODE_BADNOTFOUND;
    UA_String *ns = response.results[0].value.data;
    for(size_t i = 0; i < response.results[0].value.arrayLength; i++){
        if(UA_String_equal(namespaceUri, &ns[i])) {
            *namespaceIndex = (UA_UInt16)i;
            retval = UA_STATUSCODE_GOOD;
            break;
        }
    }

    UA_ReadResponse_deleteMembers(&response);
	return retval;
}

UA_StatusCode
UA_Client_forEachChildNodeCall(UA_Client *client, UA_NodeId parentNodeId, UA_NodeIteratorCallback callback, void *handle) {
  UA_StatusCode retval = UA_STATUSCODE_GOOD;
  
  UA_BrowseRequest bReq;
  UA_BrowseRequest_init(&bReq);
  bReq.requestedMaxReferencesPerNode = 0;
  bReq.nodesToBrowse = UA_BrowseDescription_new();
  bReq.nodesToBrowseSize = 1;
  UA_NodeId_copy(&parentNodeId, &bReq.nodesToBrowse[0].nodeId);
  bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; //return everything
  bReq.nodesToBrowse[0].browseDirection = UA_BROWSEDIRECTION_BOTH;
  
  UA_BrowseResponse bResp = UA_Client_Service_browse(client, bReq);
  
  if(bResp.responseHeader.serviceResult == UA_STATUSCODE_GOOD) {
    for (size_t i = 0; i < bResp.resultsSize; ++i) {
      for (size_t j = 0; j < bResp.results[i].referencesSize; ++j) {
        UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
        retval |= callback(ref->nodeId.nodeId, ! ref->isForward, ref->referenceTypeId, handle);
      }
    }
  }
  else
    retval = bResp.responseHeader.serviceResult;
  
  
  UA_BrowseRequest_deleteMembers(&bReq);
  UA_BrowseResponse_deleteMembers(&bResp);
  
  return retval;
}

/*******************/
/* Node Management */
/*******************/

UA_StatusCode UA_EXPORT
UA_Client_addReference(UA_Client *client, const UA_NodeId sourceNodeId, const UA_NodeId referenceTypeId,
                       UA_Boolean isForward, const UA_String targetServerUri,
                       const UA_ExpandedNodeId targetNodeId, UA_NodeClass targetNodeClass) {
    UA_AddReferencesItem item;
    UA_AddReferencesItem_init(&item);
    item.sourceNodeId = sourceNodeId;
    item.referenceTypeId = referenceTypeId;
    item.isForward = isForward;
    item.targetServerUri = targetServerUri;
    item.targetNodeId = targetNodeId;
    item.targetNodeClass = targetNodeClass;
    UA_AddReferencesRequest request;
    UA_AddReferencesRequest_init(&request);
    request.referencesToAdd = &item;
    request.referencesToAddSize = 1;
    UA_AddReferencesResponse response = UA_Client_Service_addReferences(client, request);
    UA_StatusCode retval = response.responseHeader.serviceResult;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_AddReferencesResponse_deleteMembers(&response);
        return retval;
    }
    if(response.resultsSize != 1) {
        UA_AddReferencesResponse_deleteMembers(&response);
        return UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    retval = response.results[0];
    UA_AddReferencesResponse_deleteMembers(&response);
    return retval;
}

UA_StatusCode UA_EXPORT
UA_Client_deleteReference(UA_Client *client, const UA_NodeId sourceNodeId, const UA_NodeId referenceTypeId,
                          UA_Boolean isForward, const UA_ExpandedNodeId targetNodeId,
                          UA_Boolean deleteBidirectional) {
    UA_DeleteReferencesItem item;
    UA_DeleteReferencesItem_init(&item);
    item.sourceNodeId = sourceNodeId;
    item.referenceTypeId = referenceTypeId;
    item.isForward = isForward;
    item.targetNodeId = targetNodeId;
    item.deleteBidirectional = deleteBidirectional;
    UA_DeleteReferencesRequest request;
    UA_DeleteReferencesRequest_init(&request);
    request.referencesToDelete = &item;
    request.referencesToDeleteSize = 1;
    UA_DeleteReferencesResponse response = UA_Client_Service_deleteReferences(client, request);
    UA_StatusCode retval = response.responseHeader.serviceResult;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_DeleteReferencesResponse_deleteMembers(&response);
        return retval;
    }
    if(response.resultsSize != 1) {
        UA_DeleteReferencesResponse_deleteMembers(&response);
        return UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    retval = response.results[0];
    UA_DeleteReferencesResponse_deleteMembers(&response);
    return retval;
}

UA_StatusCode
UA_Client_deleteNode(UA_Client *client, const UA_NodeId nodeId, UA_Boolean deleteTargetReferences) {
    UA_DeleteNodesItem item;
    UA_DeleteNodesItem_init(&item);
    item.nodeId = nodeId;
    item.deleteTargetReferences = deleteTargetReferences;
    UA_DeleteNodesRequest request;
    UA_DeleteNodesRequest_init(&request);
    request.nodesToDelete = &item;
    request.nodesToDeleteSize = 1;
    UA_DeleteNodesResponse response = UA_Client_Service_deleteNodes(client, request);
    UA_StatusCode retval = response.responseHeader.serviceResult;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_DeleteNodesResponse_deleteMembers(&response);
        return retval;
    }
    if(response.resultsSize != 1) {
        UA_DeleteNodesResponse_deleteMembers(&response);
        return UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    retval = response.results[0];
    UA_DeleteNodesResponse_deleteMembers(&response);
    return retval;
}

UA_StatusCode
__UA_Client_addNode(UA_Client *client, const UA_NodeClass nodeClass, const UA_NodeId requestedNewNodeId,
                    const UA_NodeId parentNodeId, const UA_NodeId referenceTypeId,
                    const UA_QualifiedName browseName, const UA_NodeId typeDefinition,
                    const UA_NodeAttributes *attr, const UA_DataType *attributeType, UA_NodeId *outNewNodeId) {
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    UA_AddNodesRequest request;
    UA_AddNodesRequest_init(&request);
    UA_AddNodesItem item;
    UA_AddNodesItem_init(&item);
    item.parentNodeId.nodeId = parentNodeId;
    item.referenceTypeId = referenceTypeId;
    item.requestedNewNodeId.nodeId = requestedNewNodeId;
    item.browseName = browseName;
    item.nodeClass = nodeClass;
    item.typeDefinition.nodeId = typeDefinition;
    item.nodeAttributes.encoding = UA_EXTENSIONOBJECT_DECODED_NODELETE;
    item.nodeAttributes.content.decoded.type = attributeType;
    item.nodeAttributes.content.decoded.data = (void*)(uintptr_t)attr; // hack. is not written into.
    request.nodesToAdd = &item;
    request.nodesToAddSize = 1;
    UA_AddNodesResponse response = UA_Client_Service_addNodes(client, request);
    if(response.responseHeader.serviceResult != UA_STATUSCODE_GOOD) {
        retval = response.responseHeader.serviceResult;
        UA_AddNodesResponse_deleteMembers(&response);
        return retval;
    }
    if(response.resultsSize != 1) {
        UA_AddNodesResponse_deleteMembers(&response);
        return UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    if(outNewNodeId && response.results[0].statusCode == UA_STATUSCODE_GOOD) {
        *outNewNodeId = response.results[0].addedNodeId;
        UA_NodeId_init(&response.results[0].addedNodeId);
    }
    retval = response.results[0].statusCode;
    UA_AddNodesResponse_deleteMembers(&response);
    return retval;
}

/********/
/* Call */
/********/

UA_StatusCode
UA_Client_call(UA_Client *client, const UA_NodeId objectId, const UA_NodeId methodId, size_t inputSize,
               const UA_Variant *input, size_t *outputSize, UA_Variant **output) {
    UA_CallRequest request;
    UA_CallRequest_init(&request);
    UA_CallMethodRequest item;
    UA_CallMethodRequest_init(&item);
    item.methodId = methodId;
    item.objectId = objectId;
    item.inputArguments = (void*)(uintptr_t)input; // cast const...
    item.inputArgumentsSize = inputSize;
    request.methodsToCall = &item;
    request.methodsToCallSize = 1;
    UA_CallResponse response = UA_Client_Service_call(client, request);
    UA_StatusCode retval = response.responseHeader.serviceResult;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_CallResponse_deleteMembers(&response);
        return retval;
    }
    if(response.resultsSize != 1) {
        UA_CallResponse_deleteMembers(&response);
        return UA_STATUSCODE_BADUNEXPECTEDERROR;
    }
    retval = response.results[0].statusCode;
    if(retval == UA_STATUSCODE_GOOD && response.resultsSize > 0) {
        if (output != NULL && outputSize != NULL) {
          *output = response.results[0].outputArguments;
          *outputSize = response.results[0].outputArgumentsSize;
        }
        response.results[0].outputArguments = NULL;
        response.results[0].outputArgumentsSize = 0;
    }
    UA_CallResponse_deleteMembers(&response);
    return retval;
}

/********************/
/* Write Attributes */
/********************/

UA_StatusCode 
__UA_Client_writeAttribute(UA_Client *client, const UA_NodeId *nodeId, UA_AttributeId attributeId,
                           const void *in, const UA_DataType *inDataType) {
    if(!in)
      return UA_STATUSCODE_BADTYPEMISMATCH;
    
    UA_WriteValue wValue;
    UA_WriteValue_init(&wValue);
    wValue.nodeId = *nodeId;
    wValue.attributeId = attributeId;
    if(attributeId == UA_ATTRIBUTEID_VALUE)
        wValue.value.value = *(const UA_Variant*)in;
    else
        UA_Variant_setScalar(&wValue.value.value, (void*)(uintptr_t)in, inDataType); /* hack. is never written into. */
    wValue.value.hasValue = true;
    UA_WriteRequest wReq;
    UA_WriteRequest_init(&wReq);
    wReq.nodesToWrite = &wValue;
    wReq.nodesToWriteSize = 1;
    
    UA_WriteResponse wResp = UA_Client_Service_write(client, wReq);
    UA_StatusCode retval = wResp.responseHeader.serviceResult;
    UA_WriteResponse_deleteMembers(&wResp);
    return retval;
}

UA_StatusCode
UA_Client_writeArrayDimensionsAttribute(UA_Client *client, const UA_NodeId nodeId,
                                        const UA_Int32 *newArrayDimensions, size_t newArrayDimensionsSize) {
    if(!newArrayDimensions)
      return UA_STATUSCODE_BADTYPEMISMATCH;
    
    UA_WriteValue wValue;
    UA_WriteValue_init(&wValue);
    wValue.nodeId = nodeId;
    wValue.attributeId = UA_ATTRIBUTEID_ARRAYDIMENSIONS;
    UA_Variant_setArray(&wValue.value.value, (void*)(uintptr_t)newArrayDimensions,
                        newArrayDimensionsSize, &UA_TYPES[UA_TYPES_INT32]);
    wValue.value.hasValue = true;
    UA_WriteRequest wReq;
    UA_WriteRequest_init(&wReq);
    wReq.nodesToWrite = &wValue;
    wReq.nodesToWriteSize = 1;
    
    UA_WriteResponse wResp = UA_Client_Service_write(client, wReq);
    UA_StatusCode retval = wResp.responseHeader.serviceResult;
    UA_WriteResponse_deleteMembers(&wResp);
    return retval;
}

/*******************/
/* Read Attributes */
/*******************/

UA_StatusCode 
__UA_Client_readAttribute(UA_Client *client, const UA_NodeId *nodeId, UA_AttributeId attributeId,
                          void *out, const UA_DataType *outDataType) {
    UA_ReadValueId item;
    UA_ReadValueId_init(&item);
    item.nodeId = *nodeId;
    item.attributeId = attributeId;
    UA_ReadRequest request;
    UA_ReadRequest_init(&request);
    request.nodesToRead = &item;
    request.nodesToReadSize = 1;
    UA_ReadResponse response = UA_Client_Service_read(client, request);
    UA_StatusCode retval = response.responseHeader.serviceResult;
    if(retval == UA_STATUSCODE_GOOD && response.resultsSize != 1)
        retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_ReadResponse_deleteMembers(&response);
        return retval;
    }

    UA_DataValue *res = response.results;
    if(res->hasStatus != UA_STATUSCODE_GOOD)
        retval = res->hasStatus;
    else if(!res->hasValue || !UA_Variant_isScalar(&res->value))
        retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_ReadResponse_deleteMembers(&response);
        return retval;
    }

    if(attributeId == UA_ATTRIBUTEID_VALUE) {
        memcpy(out, &res->value, sizeof(UA_Variant));
        UA_Variant_init(&res->value);
    } else if(UA_Variant_isScalar(&res->value) &&
              res->value.type == outDataType) {
        memcpy(out, res->value.data, res->value.type->memSize);
        UA_free(res->value.data);
        res->value.data = NULL;
    } else {
        retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
    }

    UA_ReadResponse_deleteMembers(&response);
    return retval;
}

UA_StatusCode
UA_Client_readArrayDimensionsAttribute(UA_Client *client, const UA_NodeId nodeId,
                                       UA_Int32 **outArrayDimensions, size_t *outArrayDimensionsSize) {
    UA_ReadValueId item;
    UA_ReadValueId_init(&item);
    item.nodeId = nodeId;
    item.attributeId = UA_ATTRIBUTEID_ARRAYDIMENSIONS;
    UA_ReadRequest request;
    UA_ReadRequest_init(&request);
    request.nodesToRead = &item;
    request.nodesToReadSize = 1;
    UA_ReadResponse response = UA_Client_Service_read(client, request);
    UA_StatusCode retval = response.responseHeader.serviceResult;
    if(retval == UA_STATUSCODE_GOOD && response.resultsSize != 1)
        retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
    if(retval != UA_STATUSCODE_GOOD)
        goto cleanup;

    UA_DataValue *res = response.results;
    if(res->hasStatus != UA_STATUSCODE_GOOD)
        retval = res->hasStatus;
    else if(!res->hasValue || UA_Variant_isScalar(&res->value))
        retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
    if(retval != UA_STATUSCODE_GOOD)
        goto cleanup;

    if(UA_Variant_isScalar(&res->value) ||
       res->value.type != &UA_TYPES[UA_TYPES_INT32]) {
        retval = UA_STATUSCODE_BADUNEXPECTEDERROR;
        goto cleanup;
    }

    *outArrayDimensions = res->value.data;
    *outArrayDimensionsSize = res->value.arrayLength;
    UA_free(res->value.data);
    res->value.data = NULL;
    res->value.arrayLength = 0;

 cleanup:
    UA_ReadResponse_deleteMembers(&response);
    return retval;

}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_nodestore.c" ***********************************/


#ifndef UA_ENABLE_MULTITHREADING /* conditional compilation */

#define UA_NODESTORE_MINSIZE 64

typedef struct UA_NodeStoreEntry {
    struct UA_NodeStoreEntry *orig; // the version this is a copy from (or NULL)
    UA_Node node;
} UA_NodeStoreEntry;

struct UA_NodeStore {
    UA_NodeStoreEntry **entries;
    UA_UInt32 size;
    UA_UInt32 count;
    UA_UInt32 sizePrimeIndex;
};


/* The size of the hash-map is always a prime number. They are chosen to be
   close to the next power of 2. So the size ca. doubles with each prime. */
static hash_t const primes[] = {
    7,         13,         31,         61,         127,         251,
    509,       1021,       2039,       4093,       8191,        16381,
    32749,     65521,      131071,     262139,     524287,      1048573,
    2097143,   4194301,    8388593,    16777213,   33554393,    67108859,
    134217689, 268435399,  536870909,  1073741789, 2147483647,  4294967291
};

static UA_UInt16 higher_prime_index(hash_t n) {
    UA_UInt16 low  = 0;
    UA_UInt16 high = (UA_UInt16)(sizeof(primes) / sizeof(hash_t));
    while(low != high) {
        UA_UInt16 mid = (UA_UInt16)(low + ((high - low) / 2));
        if(n > primes[mid])
            low = (UA_UInt16)(mid + 1);
        else
            high = mid;
    }
    return low;
}

static UA_NodeStoreEntry * instantiateEntry(UA_NodeClass nodeClass) {
    size_t size = sizeof(UA_NodeStoreEntry) - sizeof(UA_Node);
    switch(nodeClass) {
    case UA_NODECLASS_OBJECT:
        size += sizeof(UA_ObjectNode);
        break;
    case UA_NODECLASS_VARIABLE:
        size += sizeof(UA_VariableNode);
        break;
    case UA_NODECLASS_METHOD:
        size += sizeof(UA_MethodNode);
        break;
    case UA_NODECLASS_OBJECTTYPE:
        size += sizeof(UA_ObjectTypeNode);
        break;
    case UA_NODECLASS_VARIABLETYPE:
        size += sizeof(UA_VariableTypeNode);
        break;
    case UA_NODECLASS_REFERENCETYPE:
        size += sizeof(UA_ReferenceTypeNode);
        break;
    case UA_NODECLASS_DATATYPE:
        size += sizeof(UA_DataTypeNode);
        break;
    case UA_NODECLASS_VIEW:
        size += sizeof(UA_ViewNode);
        break;
    default:
        return NULL;
    }
    UA_NodeStoreEntry *entry = UA_calloc(1, size);
    if(!entry)
        return NULL;
    entry->node.nodeClass = nodeClass;
    return entry;
}

static void deleteEntry(UA_NodeStoreEntry *entry) {
    UA_Node_deleteMembersAnyNodeClass(&entry->node);
    UA_free(entry);
}

/* Returns true if an entry was found under the nodeid. Otherwise, returns
   false and sets slot to a pointer to the next free slot. */
static UA_Boolean
containsNodeId(const UA_NodeStore *ns, const UA_NodeId *nodeid, UA_NodeStoreEntry ***entry) {
    hash_t h = hash(nodeid);
    UA_UInt32 size = ns->size;
    hash_t idx = mod(h, size);
    UA_NodeStoreEntry *e = ns->entries[idx];

    if(!e) {
        *entry = &ns->entries[idx];
        return false;
    }

    if(UA_NodeId_equal(&e->node.nodeId, nodeid)) {
        *entry = &ns->entries[idx];
        return true;
    }

    hash_t hash2 = mod2(h, size);
    for(;;) {
        idx += hash2;
        if(idx >= size)
            idx -= size;
        e = ns->entries[idx];
        if(!e) {
            *entry = &ns->entries[idx];
            return false;
        }
        if(UA_NodeId_equal(&e->node.nodeId, nodeid)) {
            *entry = &ns->entries[idx];
            return true;
        }
    }

    /* NOTREACHED */
    return true;
}

/* The occupancy of the table after the call will be about 50% */
static UA_StatusCode expand(UA_NodeStore *ns) {
    UA_UInt32 osize = ns->size;
    UA_UInt32 count = ns->count;
    /* Resize only when table after removal of unused elements is either too full or too empty  */
    if(count * 2 < osize && (count * 8 > osize || osize <= UA_NODESTORE_MINSIZE))
        return UA_STATUSCODE_GOOD;

    UA_NodeStoreEntry **oentries = ns->entries;
    UA_UInt32 nindex = higher_prime_index(count * 2);
    UA_UInt32 nsize = primes[nindex];
    UA_NodeStoreEntry **nentries;
    if(!(nentries = UA_calloc(nsize, sizeof(UA_NodeStoreEntry*))))
        return UA_STATUSCODE_BADOUTOFMEMORY;

    ns->entries = nentries;
    ns->size = nsize;
    ns->sizePrimeIndex = nindex;

    /* recompute the position of every entry and insert the pointer */
    for(size_t i = 0, j = 0; i < osize && j < count; i++) {
        if(!oentries[i])
            continue;
        UA_NodeStoreEntry **e;
        containsNodeId(ns, &oentries[i]->node.nodeId, &e);  /* We know this returns an empty entry here */
        *e = oentries[i];
        j++;
    }

    UA_free(oentries);
    return UA_STATUSCODE_GOOD;
}

/**********************/
/* Exported functions */
/**********************/

UA_NodeStore * UA_NodeStore_new(void) {
    UA_NodeStore *ns;
    if(!(ns = UA_malloc(sizeof(UA_NodeStore))))
        return NULL;
    ns->sizePrimeIndex = higher_prime_index(UA_NODESTORE_MINSIZE);
    ns->size = primes[ns->sizePrimeIndex];
    ns->count = 0;
    if(!(ns->entries = UA_calloc(ns->size, sizeof(UA_NodeStoreEntry*)))) {
        UA_free(ns);
        return NULL;
    }
    return ns;
}

void UA_NodeStore_delete(UA_NodeStore *ns) {
    UA_UInt32 size = ns->size;
    UA_NodeStoreEntry **entries = ns->entries;
    for(UA_UInt32 i = 0; i < size; i++) {
        if(entries[i])
            deleteEntry(entries[i]);
    }
    UA_free(ns->entries);
    UA_free(ns);
}

UA_Node * UA_NodeStore_newNode(UA_NodeClass class) {
    UA_NodeStoreEntry *entry = instantiateEntry(class);
    if(!entry)
        return NULL;
    return (UA_Node*)&entry->node;
}

void UA_NodeStore_deleteNode(UA_Node *node) {
    deleteEntry(container_of(node, UA_NodeStoreEntry, node));
}

UA_StatusCode UA_NodeStore_insert(UA_NodeStore *ns, UA_Node *node) {
    if(ns->size * 3 <= ns->count * 4) {
        if(expand(ns) != UA_STATUSCODE_GOOD)
            return UA_STATUSCODE_BADINTERNALERROR;
    }

    UA_NodeId tempNodeid;
    tempNodeid = node->nodeId;
    tempNodeid.namespaceIndex = 0;
    UA_NodeStoreEntry **entry;
    if(UA_NodeId_isNull(&tempNodeid)) {
        if(node->nodeId.namespaceIndex == 0)
            node->nodeId.namespaceIndex = 1;
        /* find a free nodeid */
        UA_UInt32 identifier = ns->count+1; // start value
        UA_UInt32 size = ns->size;
        hash_t increase = mod2(identifier, size);
        while(true) {
            node->nodeId.identifier.numeric = identifier;
            if(!containsNodeId(ns, &node->nodeId, &entry))
                break;
            identifier += increase;
            if(identifier >= size)
                identifier -= size;
        }
    } else {
        if(containsNodeId(ns, &node->nodeId, &entry)) {
            deleteEntry(container_of(node, UA_NodeStoreEntry, node));
            return UA_STATUSCODE_BADNODEIDEXISTS;
        }
    }

    *entry = container_of(node, UA_NodeStoreEntry, node);
    ns->count++;
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
UA_NodeStore_replace(UA_NodeStore *ns, UA_Node *node) {
    UA_NodeStoreEntry **entry;
    if(!containsNodeId(ns, &node->nodeId, &entry))
        return UA_STATUSCODE_BADNODEIDUNKNOWN;
    UA_NodeStoreEntry *newEntry = container_of(node, UA_NodeStoreEntry, node);
    if(*entry != newEntry->orig) {
        deleteEntry(newEntry);
        return UA_STATUSCODE_BADINTERNALERROR; // the node was replaced since the copy was made
    }
    deleteEntry(*entry);
    *entry = newEntry;
    return UA_STATUSCODE_GOOD;
}

const UA_Node * UA_NodeStore_get(UA_NodeStore *ns, const UA_NodeId *nodeid) {
    UA_NodeStoreEntry **entry;
    if(!containsNodeId(ns, nodeid, &entry))
        return NULL;
    return (const UA_Node*)&(*entry)->node;
}

UA_Node * UA_NodeStore_getCopy(UA_NodeStore *ns, const UA_NodeId *nodeid) {
    UA_NodeStoreEntry **slot;
    if(!containsNodeId(ns, nodeid, &slot))
        return NULL;
    UA_NodeStoreEntry *entry = *slot;
    UA_NodeStoreEntry *new = instantiateEntry(entry->node.nodeClass);
    if(!new)
        return NULL;
    if(UA_Node_copyAnyNodeClass(&entry->node, &new->node) != UA_STATUSCODE_GOOD) {
        deleteEntry(new);
        return NULL;
    }
    new->orig = entry;
    return &new->node;
}

UA_StatusCode UA_NodeStore_remove(UA_NodeStore *ns, const UA_NodeId *nodeid) {
    UA_NodeStoreEntry **slot;
    if(!containsNodeId(ns, nodeid, &slot))
        return UA_STATUSCODE_BADNODEIDUNKNOWN;
    deleteEntry(*slot);
    *slot = NULL;
    ns->count--;
    /* Downsize the hashmap if it is very empty */
    if(ns->count * 8 < ns->size && ns->size > 32)
        expand(ns); // this can fail. we just continue with the bigger hashmap.
    return UA_STATUSCODE_GOOD;
}

void UA_NodeStore_iterate(UA_NodeStore *ns, UA_NodeStore_nodeVisitor visitor) {
    for(UA_UInt32 i = 0; i < ns->size; i++) {
        if(ns->entries[i])
            visitor((UA_Node*)&ns->entries[i]->node);
    }
}

#endif /* UA_ENABLE_MULTITHREADING */

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_nodestore_concurrent.c" ***********************************/


#ifdef UA_ENABLE_MULTITHREADING /* conditional compilation */

struct nodeEntry {
    struct cds_lfht_node htn; ///< Contains the next-ptr for urcu-hashmap
    struct rcu_head rcu_head; ///< For call-rcu
    struct nodeEntry *orig; //< the version this is a copy from (or NULL)
    UA_Node node; ///< Might be cast from any _bigger_ UA_Node* type. Allocate enough memory!
};


static struct nodeEntry * instantiateEntry(UA_NodeClass class) {
    size_t size = sizeof(struct nodeEntry) - sizeof(UA_Node);
    switch(class) {
    case UA_NODECLASS_OBJECT:
        size += sizeof(UA_ObjectNode);
        break;
    case UA_NODECLASS_VARIABLE:
        size += sizeof(UA_VariableNode);
        break;
    case UA_NODECLASS_METHOD:
        size += sizeof(UA_MethodNode);
        break;
    case UA_NODECLASS_OBJECTTYPE:
        size += sizeof(UA_ObjectTypeNode);
        break;
    case UA_NODECLASS_VARIABLETYPE:
        size += sizeof(UA_VariableTypeNode);
        break;
    case UA_NODECLASS_REFERENCETYPE:
        size += sizeof(UA_ReferenceTypeNode);
        break;
    case UA_NODECLASS_DATATYPE:
        size += sizeof(UA_DataTypeNode);
        break;
    case UA_NODECLASS_VIEW:
        size += sizeof(UA_ViewNode);
        break;
    default:
        return NULL;
    }
    struct nodeEntry *entry = UA_calloc(1, size);
    if(!entry)
        return NULL;
    entry->node.nodeClass = class;
    return entry;
}

static void deleteEntry(struct rcu_head *head) {
    struct nodeEntry *entry = container_of(head, struct nodeEntry, rcu_head);
    UA_Node_deleteMembersAnyNodeClass(&entry->node);
    UA_free(entry);
}

/* We are in a rcu_read lock. So the node will not be freed under our feet. */
static int compare(struct cds_lfht_node *htn, const void *orig) {
    const UA_NodeId *origid = (const UA_NodeId *)orig;
    /* The htn is first in the entry structure. */
    const UA_NodeId *newid  = &((struct nodeEntry *)htn)->node.nodeId;
    return UA_NodeId_equal(newid, origid);
}

UA_NodeStore * UA_NodeStore_new() {
    /* 64 is the minimum size for the hashtable. */
    return (UA_NodeStore*)cds_lfht_new(64, 64, 0, CDS_LFHT_AUTO_RESIZE, NULL);
}

/* do not call with read-side critical section held!! */
void UA_NodeStore_delete(UA_NodeStore *ns) {
    UA_ASSERT_RCU_LOCKED();
    struct cds_lfht *ht = (struct cds_lfht*)ns;
    struct cds_lfht_iter iter;
    cds_lfht_first(ht, &iter);
    while(iter.node) {
        if(!cds_lfht_del(ht, iter.node)) {
            /* points to the htn entry, which is first */
            struct nodeEntry *entry = (struct nodeEntry*) iter.node;
            call_rcu(&entry->rcu_head, deleteEntry);
        }
        cds_lfht_next(ht, &iter);
    }
    cds_lfht_destroy(ht, NULL);
    UA_free(ns);
}

UA_Node * UA_NodeStore_newNode(UA_NodeClass class) {
    struct nodeEntry *entry = instantiateEntry(class);
    if(!entry)
        return NULL;
    return (UA_Node*)&entry->node;
}

void UA_NodeStore_deleteNode(UA_Node *node) {
    struct nodeEntry *entry = container_of(node, struct nodeEntry, node);
    deleteEntry(&entry->rcu_head);
}

UA_StatusCode UA_NodeStore_insert(UA_NodeStore *ns, UA_Node *node) {
    UA_ASSERT_RCU_LOCKED();
    struct nodeEntry *entry = container_of(node, struct nodeEntry, node);
    struct cds_lfht *ht = (struct cds_lfht*)ns;
    cds_lfht_node_init(&entry->htn);
    struct cds_lfht_node *result;
    //namespace index is assumed to be valid
    UA_NodeId tempNodeid;
    tempNodeid = node->nodeId;
    tempNodeid.namespaceIndex = 0;
    if(!UA_NodeId_isNull(&tempNodeid)) {
        hash_t h = hash(&node->nodeId);
        result = cds_lfht_add_unique(ht, h, compare, &node->nodeId, &entry->htn);
        /* If the nodeid exists already */
        if(result != &entry->htn) {
            deleteEntry(&entry->rcu_head);
            return UA_STATUSCODE_BADNODEIDEXISTS;
        }
    } else {
        /* create a unique nodeid */
        node->nodeId.identifierType = UA_NODEIDTYPE_NUMERIC;
        if(node->nodeId.namespaceIndex == 0) // original request for ns=0 should yield ns=1
            node->nodeId.namespaceIndex = 1;

        unsigned long identifier;
        long before, after;
        cds_lfht_count_nodes(ht, &before, &identifier, &after); // current number of nodes stored
        identifier++;

        node->nodeId.identifier.numeric = (UA_UInt32)identifier;
        while(true) {
            hash_t h = hash(&node->nodeId);
            result = cds_lfht_add_unique(ht, h, compare, &node->nodeId, &entry->htn);
            if(result == &entry->htn)
                break;
            node->nodeId.identifier.numeric += (UA_UInt32)(identifier * 2654435761);
        }
    }
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode UA_NodeStore_replace(UA_NodeStore *ns, UA_Node *node) {
    UA_ASSERT_RCU_LOCKED();
    struct nodeEntry *entry = container_of(node, struct nodeEntry, node);
    struct cds_lfht *ht = (struct cds_lfht*)ns;

    /* Get the current version */
    hash_t h = hash(&node->nodeId);
    struct cds_lfht_iter iter;
    cds_lfht_lookup(ht, h, compare, &node->nodeId, &iter);
    if(!iter.node)
        return UA_STATUSCODE_BADNODEIDUNKNOWN;

    /* We try to replace an obsolete version of the node */
    struct nodeEntry *oldEntry = (struct nodeEntry*)iter.node;
    if(oldEntry != entry->orig)
        return UA_STATUSCODE_BADINTERNALERROR;
    
    cds_lfht_node_init(&entry->htn);
    if(cds_lfht_replace(ht, &iter, h, compare, &node->nodeId, &entry->htn) != 0) {
        /* Replacing failed. Maybe the node got replaced just before this thread tried to.*/
        deleteEntry(&entry->rcu_head);
        return UA_STATUSCODE_BADINTERNALERROR;
    }
        
    /* If an entry got replaced, mark it as dead. */
    call_rcu(&oldEntry->rcu_head, deleteEntry);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode UA_NodeStore_remove(UA_NodeStore *ns, const UA_NodeId *nodeid) {
    UA_ASSERT_RCU_LOCKED();
    struct cds_lfht *ht = (struct cds_lfht*)ns;
    hash_t h = hash(nodeid);
    struct cds_lfht_iter iter;
    cds_lfht_lookup(ht, h, compare, nodeid, &iter);
    if(!iter.node || cds_lfht_del(ht, iter.node) != 0)
        return UA_STATUSCODE_BADNODEIDUNKNOWN;
    struct nodeEntry *entry = (struct nodeEntry*)iter.node;
    call_rcu(&entry->rcu_head, deleteEntry);
    return UA_STATUSCODE_GOOD;
}

const UA_Node * UA_NodeStore_get(UA_NodeStore *ns, const UA_NodeId *nodeid) {
    UA_ASSERT_RCU_LOCKED();
    struct cds_lfht *ht = (struct cds_lfht*)ns;
    hash_t h = hash(nodeid);
    struct cds_lfht_iter iter;
    cds_lfht_lookup(ht, h, compare, nodeid, &iter);
    struct nodeEntry *found_entry = (struct nodeEntry*)iter.node;
    if(!found_entry)
        return NULL;
    return &found_entry->node;
}

UA_Node * UA_NodeStore_getCopy(UA_NodeStore *ns, const UA_NodeId *nodeid) {
    UA_ASSERT_RCU_LOCKED();
    struct cds_lfht *ht = (struct cds_lfht*)ns;
    hash_t h = hash(nodeid);
    struct cds_lfht_iter iter;
    cds_lfht_lookup(ht, h, compare, nodeid, &iter);
    struct nodeEntry *entry = (struct nodeEntry*)iter.node;
    if(!entry)
        return NULL;
    struct nodeEntry *new = instantiateEntry(entry->node.nodeClass);
    if(!new)
        return NULL;
    if(UA_Node_copyAnyNodeClass(&entry->node, &new->node) != UA_STATUSCODE_GOOD) {
        deleteEntry(&new->rcu_head);
        return NULL;
    }
    new->orig = entry;
    return &new->node;
}

void UA_NodeStore_iterate(UA_NodeStore *ns, UA_NodeStore_nodeVisitor visitor) {
    UA_ASSERT_RCU_LOCKED();
    struct cds_lfht *ht = (struct cds_lfht*)ns;
    struct cds_lfht_iter iter;
    cds_lfht_first(ht, &iter);
    while(iter.node != NULL) {
        struct nodeEntry *found_entry = (struct nodeEntry*)iter.node;
        visitor(&found_entry->node);
        cds_lfht_next(ht, &iter);
    }
}

#endif /* UA_ENABLE_MULTITHREADING */

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_services_call.c" ***********************************/


#ifdef UA_ENABLE_METHODCALLS /* conditional compilation */

static const UA_VariableNode *
getArgumentsVariableNode(UA_Server *server, const UA_MethodNode *ofMethod,
                         UA_String withBrowseName) {
    UA_NodeId hasProperty = UA_NODEID_NUMERIC(0, UA_NS0ID_HASPROPERTY);
    for(size_t i = 0; i < ofMethod->referencesSize; i++) {
        if(ofMethod->references[i].isInverse == false &&
            UA_NodeId_equal(&hasProperty, &ofMethod->references[i].referenceTypeId)) {
            const UA_Node *refTarget =
                UA_NodeStore_get(server->nodestore, &ofMethod->references[i].targetId.nodeId);
            if(!refTarget)
                continue;
            if(refTarget->nodeClass == UA_NODECLASS_VARIABLE &&
                refTarget->browseName.namespaceIndex == 0 &&
                UA_String_equal(&withBrowseName, &refTarget->browseName.name)) {
                return (const UA_VariableNode*) refTarget;
            }
        }
    }
    return NULL;
}

static UA_StatusCode
satisfySignature(const UA_Variant *var, const UA_Argument *arg) {
    if(!UA_NodeId_equal(&var->type->typeId, &arg->dataType) )
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    
    // Note: The namespace compiler will compile nodes with their actual array dimensions
    // Todo: Check if this is standard conform for scalars
    if(arg->arrayDimensionsSize > 0 && var->arrayDimensionsSize > 0)
        if(var->arrayDimensionsSize != arg->arrayDimensionsSize)
            return UA_STATUSCODE_BADINVALIDARGUMENT;
        
    UA_Int32 *varDims = var->arrayDimensions;
    size_t varDimsSize = var->arrayDimensionsSize;
    UA_Boolean scalar = UA_Variant_isScalar(var);

    /* The dimension 1 is implicit in the array length */
    UA_Int32 fakeDims;
    if(!scalar && !varDims) {
        fakeDims = (UA_Int32)var->arrayLength;
        varDims = &fakeDims;
        varDimsSize = 1;
    }

    /* ValueRank Semantics
     *  n >= 1: the value is an array with the specified number of dimens*ions.
     *  n = 0: the value is an array with one or more dimensions.
     *  n = -1: the value is a scalar.
     *  n = -2: the value can be a scalar or an array with any number of dimensions.
     *  n = -3:  the value can be a scalar or a one dimensional array. */
    switch(arg->valueRank) {
    case -3:
        if(varDimsSize > 1)
            return UA_STATUSCODE_BADINVALIDARGUMENT;
        break;
    case -2:
        break;
    case -1:
        if(!scalar)
            return UA_STATUSCODE_BADINVALIDARGUMENT;
        break;
    case 0:
        if(scalar || !varDims)
            return UA_STATUSCODE_BADINVALIDARGUMENT;
        break;
    default:
        break;
    }

    /* do the array dimensions match? */
    if(arg->arrayDimensionsSize != varDimsSize)
        return UA_STATUSCODE_BADINVALIDARGUMENT;
    for(size_t i = 0; i < varDimsSize; i++) {
        if((UA_Int32)arg->arrayDimensions[i] != varDims[i])
            return UA_STATUSCODE_BADINVALIDARGUMENT;
    }
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
argConformsToDefinition(const UA_VariableNode *argRequirements, size_t argsSize, const UA_Variant *args) {
    if(argRequirements->value.variant.value.type != &UA_TYPES[UA_TYPES_ARGUMENT])
        return UA_STATUSCODE_BADINTERNALERROR;
    UA_Argument *argReqs = (UA_Argument*)argRequirements->value.variant.value.data;
    size_t argReqsSize = argRequirements->value.variant.value.arrayLength;
    if(argRequirements->valueSource != UA_VALUESOURCE_VARIANT)
        return UA_STATUSCODE_BADINTERNALERROR;
    if(UA_Variant_isScalar(&argRequirements->value.variant.value))
        argReqsSize = 1;
    if(argReqsSize > argsSize)
        return UA_STATUSCODE_BADARGUMENTSMISSING;
    if(argReqsSize != argsSize)
        return UA_STATUSCODE_BADINVALIDARGUMENT;

    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    for(size_t i = 0; i < argReqsSize; i++)
        retval |= satisfySignature(&args[i], &argReqs[i]);
    return retval;
}

void
Service_Call_single(UA_Server *server, UA_Session *session, const UA_CallMethodRequest *request,
                    UA_CallMethodResult *result) {
    const UA_MethodNode *methodCalled =
        (const UA_MethodNode*)UA_NodeStore_get(server->nodestore, &request->methodId);
    if(!methodCalled) {
        result->statusCode = UA_STATUSCODE_BADMETHODINVALID;
        return;
    }
    
    const UA_ObjectNode *withObject =
        (const UA_ObjectNode*)UA_NodeStore_get(server->nodestore, &request->objectId);
    if(!withObject) {
        result->statusCode = UA_STATUSCODE_BADNODEIDINVALID;
        return;
    }
    
    if(methodCalled->nodeClass != UA_NODECLASS_METHOD) {
        result->statusCode = UA_STATUSCODE_BADNODECLASSINVALID;
        return;
    }
    
    if(withObject->nodeClass != UA_NODECLASS_OBJECT && withObject->nodeClass != UA_NODECLASS_OBJECTTYPE) {
        result->statusCode = UA_STATUSCODE_BADNODECLASSINVALID;
        return;
    }
    
    /* Verify method/object relations */
    // Object must have a hasComponent reference (or any inherited referenceType from sayd reference)
    // to be valid for a methodCall...
    result->statusCode = UA_STATUSCODE_BADMETHODINVALID;
    for(size_t i = 0; i < withObject->referencesSize; i++) {
        if(withObject->references[i].referenceTypeId.identifier.numeric == UA_NS0ID_HASCOMPONENT) {
            // FIXME: Not checking any subtypes of HasComponent at the moment
            if(UA_NodeId_equal(&withObject->references[i].targetId.nodeId, &methodCalled->nodeId)) {
                result->statusCode = UA_STATUSCODE_GOOD;
                break;
            }
        }
    }
    if(result->statusCode != UA_STATUSCODE_GOOD)
        return;
        
    /* Verify method executable */
    if(!methodCalled->executable || !methodCalled->userExecutable) {
        result->statusCode = UA_STATUSCODE_BADNOTWRITABLE; // There is no NOTEXECUTABLE?
        return;
    }

    /* Verify Input Argument count, types and sizes */
    const UA_VariableNode *inputArguments;
    inputArguments = getArgumentsVariableNode(server, methodCalled, UA_STRING("InputArguments"));
    if(inputArguments) {
        result->statusCode = argConformsToDefinition(inputArguments, request->inputArgumentsSize,
                                                     request->inputArguments);
        if(result->statusCode != UA_STATUSCODE_GOOD)
            return;
    } else if(request->inputArgumentsSize > 0) {
        result->statusCode = UA_STATUSCODE_BADINVALIDARGUMENT;
        return;
    }

    const UA_VariableNode *outputArguments =
        getArgumentsVariableNode(server, methodCalled, UA_STRING("OutputArguments"));
    if(!outputArguments) {
        // A MethodNode must have an OutputArguments variable (which may be empty)
        result->statusCode = UA_STATUSCODE_BADINTERNALERROR;
        return;
    }
    
    /* Call method if available */
    if(methodCalled->attachedMethod) {
        result->outputArguments = UA_Array_new(outputArguments->value.variant.value.arrayLength,
                                               &UA_TYPES[UA_TYPES_VARIANT]);
        result->outputArgumentsSize = outputArguments->value.variant.value.arrayLength;
        result->statusCode = methodCalled->attachedMethod(methodCalled->methodHandle, withObject->nodeId,
                                                          request->inputArgumentsSize, request->inputArguments,
                                                          result->outputArgumentsSize, result->outputArguments);
    }
    else
        result->statusCode = UA_STATUSCODE_BADNOTWRITABLE; // There is no NOTEXECUTABLE?
    
    /* TODO: Verify Output Argument count, types and sizes */
}

void Service_Call(UA_Server *server, UA_Session *session, const UA_CallRequest *request,
                  UA_CallResponse *response) {
    if(request->methodsToCallSize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }

    response->results = UA_Array_new(request->methodsToCallSize,
                                     &UA_TYPES[UA_TYPES_CALLMETHODRESULT]);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }
    response->resultsSize = request->methodsToCallSize;
    
    for(size_t i = 0; i < request->methodsToCallSize;i++)
        Service_Call_single(server, session, &request->methodsToCall[i], &response->results[i]);
}

#endif /* UA_ENABLE_METHODCALLS */

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_subscription.c" ***********************************/


#ifdef UA_ENABLE_SUBSCRIPTIONS /* conditional compilation */

/*****************/
/* MonitoredItem */
/*****************/

UA_MonitoredItem * UA_MonitoredItem_new() {
    UA_MonitoredItem *new = UA_malloc(sizeof(UA_MonitoredItem));
    new->subscription = NULL;
    new->currentQueueSize = 0;
    new->maxQueueSize = 0;
    new->monitoredItemType = UA_MONITOREDITEMTYPE_CHANGENOTIFY; /* currently hardcoded */
    new->timestampsToReturn = UA_TIMESTAMPSTORETURN_SOURCE;
    UA_String_init(&new->indexRange);
    TAILQ_INIT(&new->queue);
    UA_NodeId_init(&new->monitoredNodeId);
    new->lastSampledValue = UA_BYTESTRING_NULL;
    memset(&new->sampleJobGuid, 0, sizeof(UA_Guid));
    new->sampleJobIsRegistered = false;
    return new;
}

void MonitoredItem_delete(UA_Server *server, UA_MonitoredItem *monitoredItem) {
    MonitoredItem_unregisterSampleJob(server, monitoredItem);
    /* clear the queued samples */
    MonitoredItem_queuedValue *val, *val_tmp;
    TAILQ_FOREACH_SAFE(val, &monitoredItem->queue, listEntry, val_tmp) {
        TAILQ_REMOVE(&monitoredItem->queue, val, listEntry);
        UA_DataValue_deleteMembers(&val->value);
        UA_free(val);
    }
    monitoredItem->currentQueueSize = 0;
    LIST_REMOVE(monitoredItem, listEntry);
    UA_String_deleteMembers(&monitoredItem->indexRange);
    UA_ByteString_deleteMembers(&monitoredItem->lastSampledValue);
    UA_NodeId_deleteMembers(&monitoredItem->monitoredNodeId);
    UA_free(monitoredItem);
}

static void SampleCallback(UA_Server *server, UA_MonitoredItem *monitoredItem) {
    if(monitoredItem->monitoredItemType != UA_MONITOREDITEMTYPE_CHANGENOTIFY) {
        UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
                     "Cannot process a monitoreditem that is not a data change notification");
        return;
    }

    MonitoredItem_queuedValue *newvalue = UA_malloc(sizeof(MonitoredItem_queuedValue));
    if(!newvalue) {
        UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_SERVER,
                    "Skipped a sample due to lack of memory on monitoreditem %u", monitoredItem->itemId);
        return;
    }
    UA_DataValue_init(&newvalue->value);
    newvalue->clientHandle = monitoredItem->clientHandle;
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
                 "Sampling the value on monitoreditem %u", monitoredItem->itemId);

    /* Read the value */
    UA_ReadValueId rvid;
    UA_ReadValueId_init(&rvid);
    rvid.nodeId = monitoredItem->monitoredNodeId;
    rvid.attributeId = monitoredItem->attributeID;
    rvid.indexRange = monitoredItem->indexRange;
    UA_Subscription *sub = monitoredItem->subscription;
    Service_Read_single(server, sub->session, monitoredItem->timestampsToReturn, &rvid, &newvalue->value);

    /* encode to see if the data has changed */
    size_t binsize = UA_calcSizeBinary(&newvalue->value.value, &UA_TYPES[UA_TYPES_VARIANT]);
    UA_ByteString newValueAsByteString;
    UA_StatusCode retval = UA_ByteString_allocBuffer(&newValueAsByteString, binsize);
    if(retval != UA_STATUSCODE_GOOD) {
        UA_DataValue_deleteMembers(&newvalue->value);
        UA_free(newvalue);
        return;
    }
    size_t encodingOffset = 0;
    retval = UA_encodeBinary(&newvalue->value.value, &UA_TYPES[UA_TYPES_VARIANT],
                             NULL, NULL, &newValueAsByteString, &encodingOffset);

    /* error or the content has not changed */
    if(retval != UA_STATUSCODE_GOOD ||
       (monitoredItem->lastSampledValue.data &&
        UA_String_equal(&newValueAsByteString, &monitoredItem->lastSampledValue))) {
        UA_ByteString_deleteMembers(&newValueAsByteString);
        UA_DataValue_deleteMembers(&newvalue->value);
        UA_free(newvalue);
        UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
                     "Do not sample since the data value has not changed");
        return;
    }

    /* do we have space in the queue? */
    if(monitoredItem->currentQueueSize >= monitoredItem->maxQueueSize) {
        if(!monitoredItem->discardOldest) {
            // We cannot remove the oldest value and theres no queue space left. We're done here.
            UA_ByteString_deleteMembers(&newValueAsByteString);
            UA_DataValue_deleteMembers(&newvalue->value);
            UA_free(newvalue);
            return;
        }
        MonitoredItem_queuedValue *queueItem = TAILQ_LAST(&monitoredItem->queue, QueueOfQueueDataValues);
        TAILQ_REMOVE(&monitoredItem->queue, queueItem, listEntry);
        UA_DataValue_deleteMembers(&queueItem->value);
        UA_free(queueItem);
        monitoredItem->currentQueueSize--;
    }

    /* add the sample */
    UA_ByteString_deleteMembers(&monitoredItem->lastSampledValue);
    monitoredItem->lastSampledValue = newValueAsByteString;
    TAILQ_INSERT_TAIL(&monitoredItem->queue, newvalue, listEntry);
    monitoredItem->currentQueueSize++;
}

UA_StatusCode MonitoredItem_registerSampleJob(UA_Server *server, UA_MonitoredItem *mon) {
    //SampleCallback(server, mon);
    UA_Job job = {.type = UA_JOBTYPE_METHODCALL,
                  .job.methodCall = {.method = (UA_ServerCallback)SampleCallback, .data = mon} };
    UA_StatusCode retval = UA_Server_addRepeatedJob(server, job, (UA_UInt32)mon->samplingInterval,
                                                    &mon->sampleJobGuid);
    if(retval == UA_STATUSCODE_GOOD)
        mon->sampleJobIsRegistered = true;
    return retval;
}

UA_StatusCode MonitoredItem_unregisterSampleJob(UA_Server *server, UA_MonitoredItem *mon) {
    if(!mon->sampleJobIsRegistered)
        return UA_STATUSCODE_GOOD;
    mon->sampleJobIsRegistered = false;
    return UA_Server_removeRepeatedJob(server, mon->sampleJobGuid);
}

/****************/
/* Subscription */
/****************/

UA_Subscription * UA_Subscription_new(UA_Session *session, UA_UInt32 subscriptionID) {
    UA_Subscription *new = UA_malloc(sizeof(UA_Subscription));
    if(!new)
        return NULL;
    new->session = session;
    new->subscriptionID = subscriptionID;
    new->sequenceNumber = 0;
    new->maxKeepAliveCount = 0;
    new->publishingEnabled = false;
    memset(&new->publishJobGuid, 0, sizeof(UA_Guid));
    new->publishJobIsRegistered = false;
    new->currentKeepAliveCount = 0;
    new->currentLifetimeCount = 0;
    new->state = UA_SUBSCRIPTIONSTATE_LATE; /* The first publish response is sent immediately */
    LIST_INIT(&new->retransmissionQueue);
    LIST_INIT(&new->MonitoredItems);
    return new;
}

void UA_Subscription_deleteMembers(UA_Subscription *subscription, UA_Server *server) {
    Subscription_unregisterPublishJob(server, subscription);

    /* Delete monitored Items */
    UA_MonitoredItem *mon, *tmp_mon;
    LIST_FOREACH_SAFE(mon, &subscription->MonitoredItems, listEntry, tmp_mon) {
        LIST_REMOVE(mon, listEntry);
        MonitoredItem_delete(server, mon);
    }

    /* Delete Retransmission Queue */
    UA_NotificationMessageEntry *nme, *nme_tmp;
    LIST_FOREACH_SAFE(nme, &subscription->retransmissionQueue, listEntry, nme_tmp) {
        LIST_REMOVE(nme, listEntry);
        UA_NotificationMessage_deleteMembers(&nme->message);
        UA_free(nme);
    }
}

UA_MonitoredItem *
UA_Subscription_getMonitoredItem(UA_Subscription *sub, UA_UInt32 monitoredItemID) {
    UA_MonitoredItem *mon;
    LIST_FOREACH(mon, &sub->MonitoredItems, listEntry) {
        if(mon->itemId == monitoredItemID)
            break;
    }
    return mon;
}

UA_StatusCode
UA_Subscription_deleteMonitoredItem(UA_Server *server, UA_Subscription *sub,
                                    UA_UInt32 monitoredItemID) {
    UA_MonitoredItem *mon;
    LIST_FOREACH(mon, &sub->MonitoredItems, listEntry) {
        if(mon->itemId == monitoredItemID) {
            LIST_REMOVE(mon, listEntry);
            MonitoredItem_delete(server, mon);
            return UA_STATUSCODE_GOOD;
        }
    }
    return UA_STATUSCODE_BADMONITOREDITEMIDINVALID;
}

void UA_Subscription_publishCallback(UA_Server *server, UA_Subscription *sub) {
    /* Count the available notifications */
    size_t notifications = 0;
    UA_Boolean moreNotifications = false;
    if(sub->publishingEnabled) {
        UA_MonitoredItem *mon;
        LIST_FOREACH(mon, &sub->MonitoredItems, listEntry) {
            MonitoredItem_queuedValue *qv;
            TAILQ_FOREACH(qv, &mon->queue, listEntry) {
                if(notifications >= sub->notificationsPerPublish) {
                    moreNotifications = true;
                    break;
                }
                notifications++;
            }
        }
    }

    /* Return if nothing to do */
    if(notifications == 0) {
        sub->currentKeepAliveCount++;
        if(sub->currentKeepAliveCount < sub->maxKeepAliveCount)
            return;
    }

    /* Check if the securechannel is valid */
    UA_SecureChannel *channel = sub->session->channel;
    if(!channel)
        return;

    /* Dequeue a response */
    UA_PublishResponseEntry *pre = SIMPLEQ_FIRST(&sub->session->responseQueue);
    if(!pre) {
        UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
            "Cannot send a publish response on subscription %u " \
            "since the publish queue is empty on session %u",
            sub->subscriptionID, sub->session->authenticationToken.identifier.numeric);
        if(sub->state != UA_SUBSCRIPTIONSTATE_LATE) {
            sub->state = UA_SUBSCRIPTIONSTATE_LATE;
        } else {
            sub->currentLifetimeCount++;
            if(sub->currentLifetimeCount >= sub->lifeTimeCount) {
                UA_LOG_INFO(server->config.logger, UA_LOGCATEGORY_SERVER,
                    "End of lifetime for subscription %u on session %u",
                    sub->subscriptionID, sub->session->authenticationToken.identifier.numeric);
                UA_Session_deleteSubscription(server, sub->session, sub->subscriptionID);
            }
        }
        return;
    }
    SIMPLEQ_REMOVE_HEAD(&sub->session->responseQueue, listEntry);
    UA_PublishResponse *response = &pre->response;
    UA_UInt32 requestId = pre->requestId;

    /* We have a request. Reset state to normal. */
    sub->state = UA_SUBSCRIPTIONSTATE_NORMAL;
    sub->currentKeepAliveCount = 0;
    sub->currentLifetimeCount = 0;

    /* Prepare the response */
    response->responseHeader.timestamp = UA_DateTime_now();
    response->subscriptionId = sub->subscriptionID;
    response->moreNotifications = moreNotifications;
    UA_NotificationMessage *message = &response->notificationMessage;
    message->publishTime = response->responseHeader.timestamp;
    if(notifications == 0) {
        /* Send sequence number for the next notification */
        message->sequenceNumber = sub->sequenceNumber + 1;
    } else {
        /* Increase the sequence number */
        message->sequenceNumber = ++sub->sequenceNumber;

        /* Collect the notification messages */
        message->notificationData = UA_ExtensionObject_new();
        message->notificationDataSize = 1;
        UA_ExtensionObject *data = message->notificationData;
        UA_DataChangeNotification *dcn = UA_DataChangeNotification_new();
        dcn->monitoredItems = UA_Array_new(notifications, &UA_TYPES[UA_TYPES_MONITOREDITEMNOTIFICATION]);
        dcn->monitoredItemsSize = notifications;
        size_t l = 0;
        UA_MonitoredItem *mon;
        LIST_FOREACH(mon, &sub->MonitoredItems, listEntry) {
            MonitoredItem_queuedValue *qv, *qv_tmp;
            TAILQ_FOREACH_SAFE(qv, &mon->queue, listEntry, qv_tmp) {
                if(notifications <= l)
                    break;
                UA_MonitoredItemNotification *min = &dcn->monitoredItems[l];
                min->clientHandle = qv->clientHandle;
                min->value = qv->value;
                TAILQ_REMOVE(&mon->queue, qv, listEntry);
                UA_free(qv);
                mon->currentQueueSize--;
                l++;
            }
        }
        data->encoding = UA_EXTENSIONOBJECT_DECODED;
        data->content.decoded.data = dcn;
        data->content.decoded.type = &UA_TYPES[UA_TYPES_DATACHANGENOTIFICATION];

        /* Put the notification message into the retransmission queue */
        UA_NotificationMessageEntry *retransmission = malloc(sizeof(UA_NotificationMessageEntry));
        retransmission->message = response->notificationMessage;
        LIST_INSERT_HEAD(&sub->retransmissionQueue, retransmission, listEntry);
    }

    /* Get the available sequence numbers from the retransmission queue */
    size_t available = 0, i = 0;
    UA_NotificationMessageEntry *nme;
    LIST_FOREACH(nme, &sub->retransmissionQueue, listEntry)
        available++;
    //cppcheck-suppress knownConditionTrueFalse
    if(available > 0) {
        response->availableSequenceNumbers = UA_alloca(available * sizeof(UA_UInt32));
        response->availableSequenceNumbersSize = available;
    }
    LIST_FOREACH(nme, &sub->retransmissionQueue, listEntry) {
        response->availableSequenceNumbers[i] = nme->message.sequenceNumber;
        i++;
    }

    /* Send the response */
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
                 "Sending out a publish response on subscription %u on securechannel %u " \
                 "with %u notifications", sub->subscriptionID,
                 sub->session->authenticationToken.identifier.numeric, (UA_UInt32)notifications);
    UA_SecureChannel_sendBinaryMessage(sub->session->channel, requestId, response,
                                       &UA_TYPES[UA_TYPES_PUBLISHRESPONSE]);

    /* Remove the queued request */
    UA_NotificationMessage_init(&response->notificationMessage); /* message was copied to the queue */
    response->availableSequenceNumbers = NULL; /* stack-allocated */
    response->availableSequenceNumbersSize = 0;
    UA_PublishResponse_deleteMembers(&pre->response);
    UA_free(pre);

    /* Repeat if there are more notifications to send */
    if(moreNotifications)
        UA_Subscription_publishCallback(server, sub);
}

UA_StatusCode Subscription_registerPublishJob(UA_Server *server, UA_Subscription *sub) {
    UA_Job job = (UA_Job) {.type = UA_JOBTYPE_METHODCALL,
                           .job.methodCall = {.method = (UA_ServerCallback)UA_Subscription_publishCallback,
                                              .data = sub} };
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
                 "Adding a subscription with %i millisec interval", (int)sub->publishingInterval);
    UA_StatusCode retval = UA_Server_addRepeatedJob(server, job,
                                                    (UA_UInt32)sub->publishingInterval,
                                                    &sub->publishJobGuid);
    if(retval == UA_STATUSCODE_GOOD)
        sub->publishJobIsRegistered = true;
    else
        UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
                     "Could not register a subscription publication job " \
                     "with status code 0x%08x\n", retval);
    return retval;
}

UA_StatusCode Subscription_unregisterPublishJob(UA_Server *server, UA_Subscription *sub) {
    if(!sub->publishJobIsRegistered)
        return UA_STATUSCODE_GOOD;
    sub->publishJobIsRegistered = false;
    UA_StatusCode retval = UA_Server_removeRepeatedJob(server, sub->publishJobGuid);
    if(retval != UA_STATUSCODE_GOOD)
        UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
                     "Could not remove a subscription publication job " \
                     "with status code 0x%08x\n", retval);
    return retval;
}

#endif /* UA_ENABLE_SUBSCRIPTIONS */

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/server/ua_services_subscription.c" ***********************************/


#ifdef UA_ENABLE_SUBSCRIPTIONS /* conditional compilation */

#define UA_BOUNDEDVALUE_SETWBOUNDS(BOUNDS, SRC, DST) { \
        if(SRC > BOUNDS.max) DST = BOUNDS.max;         \
        else if(SRC < BOUNDS.min) DST = BOUNDS.min;    \
        else DST = SRC;                                \
    }

static void
setSubscriptionSettings(UA_Server *server, UA_Subscription *subscription,
                        UA_Double requestedPublishingInterval,
                        UA_UInt32 requestedLifetimeCount,
                        UA_UInt32 requestedMaxKeepAliveCount,
                        UA_UInt32 maxNotificationsPerPublish, UA_Byte priority) {
    Subscription_unregisterPublishJob(server, subscription);
    subscription->publishingInterval = requestedPublishingInterval;
    UA_BOUNDEDVALUE_SETWBOUNDS(server->config.publishingIntervalLimits,
                               requestedPublishingInterval, subscription->publishingInterval);
    /* check for nan*/
    if(requestedPublishingInterval != requestedPublishingInterval)
        subscription->publishingInterval = server->config.publishingIntervalLimits.min;
    UA_BOUNDEDVALUE_SETWBOUNDS(server->config.keepAliveCountLimits,
                               requestedMaxKeepAliveCount, subscription->maxKeepAliveCount);
    UA_BOUNDEDVALUE_SETWBOUNDS(server->config.lifeTimeCountLimits,
                               requestedLifetimeCount, subscription->lifeTimeCount);
    if(subscription->lifeTimeCount < 3 * subscription->maxKeepAliveCount)
        subscription->lifeTimeCount = 3 * subscription->maxKeepAliveCount;
    subscription->notificationsPerPublish = maxNotificationsPerPublish;
    if(maxNotificationsPerPublish == 0 ||
       maxNotificationsPerPublish > server->config.maxNotificationsPerPublish)
        subscription->notificationsPerPublish = server->config.maxNotificationsPerPublish;
    subscription->priority = priority;
    Subscription_registerPublishJob(server, subscription);
}

void Service_CreateSubscription(UA_Server *server, UA_Session *session,
                                const UA_CreateSubscriptionRequest *request,
                                UA_CreateSubscriptionResponse *response) {
    response->subscriptionId = UA_Session_getUniqueSubscriptionID(session);
    UA_Subscription *newSubscription = UA_Subscription_new(session, response->subscriptionId);
    if(!newSubscription) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }

    UA_Session_addSubscription(session, newSubscription);
    newSubscription->publishingEnabled = request->publishingEnabled;
    setSubscriptionSettings(server, newSubscription, request->requestedPublishingInterval,
                            request->requestedLifetimeCount, request->requestedMaxKeepAliveCount,
                            request->maxNotificationsPerPublish, request->priority);
    /* immediately send the first response */
    newSubscription->currentKeepAliveCount = newSubscription->maxKeepAliveCount;
    response->revisedPublishingInterval = newSubscription->publishingInterval;
    response->revisedLifetimeCount = newSubscription->lifeTimeCount;
    response->revisedMaxKeepAliveCount = newSubscription->maxKeepAliveCount;
}

void Service_ModifySubscription(UA_Server *server, UA_Session *session,
                                const UA_ModifySubscriptionRequest *request,
                                UA_ModifySubscriptionResponse *response) {
    UA_Subscription *sub = UA_Session_getSubscriptionByID(session, request->subscriptionId);
    if(!sub) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
        return;
    }

    setSubscriptionSettings(server, sub, request->requestedPublishingInterval,
                            request->requestedLifetimeCount, request->requestedMaxKeepAliveCount,
                            request->maxNotificationsPerPublish, request->priority);
    sub->currentLifetimeCount = 0; /* Reset the subscription lifetime */
    response->revisedPublishingInterval = sub->publishingInterval;
    response->revisedLifetimeCount = sub->lifeTimeCount;
    response->revisedMaxKeepAliveCount = sub->maxKeepAliveCount;
    return;
}

void Service_SetPublishingMode(UA_Server *server, UA_Session *session,
                               const UA_SetPublishingModeRequest *request,
                               UA_SetPublishingModeResponse *response) {

    if(request->subscriptionIdsSize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }

    size_t size = request->subscriptionIdsSize;
    response->results = UA_Array_new(size, &UA_TYPES[UA_TYPES_STATUSCODE]);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }

    response->resultsSize = size;
    for(size_t i = 0; i < size; i++) {
        UA_Subscription *sub = UA_Session_getSubscriptionByID(session, request->subscriptionIds[i]);
        if(!sub) {
            response->results[i] = UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
            continue;
        }
        sub->publishingEnabled = request->publishingEnabled;
        sub->currentLifetimeCount = 0; /* Reset the subscription lifetime */
    }
}

static void
setMonitoredItemSettings(UA_Server *server, UA_MonitoredItem *mon,
                         UA_MonitoringMode monitoringMode, UA_UInt32 clientHandle,
                         UA_Double samplingInterval, UA_UInt32 queueSize,
                         UA_Boolean discardOldest) {
    MonitoredItem_unregisterSampleJob(server, mon);
    mon->monitoringMode = monitoringMode;
    mon->clientHandle = clientHandle;
    mon->samplingInterval = samplingInterval;
    UA_BOUNDEDVALUE_SETWBOUNDS(server->config.samplingIntervalLimits,
        samplingInterval, mon->samplingInterval);
    /* Check for nan */
    if(samplingInterval != samplingInterval)
        mon->samplingInterval = server->config.samplingIntervalLimits.min;
    UA_BOUNDEDVALUE_SETWBOUNDS(server->config.queueSizeLimits,
                               queueSize, mon->maxQueueSize);
    mon->discardOldest = discardOldest;
    MonitoredItem_registerSampleJob(server, mon);
}

static const UA_String binaryEncoding = {sizeof("DefaultBinary")-1, (UA_Byte*)"DefaultBinary"};
static void
Service_CreateMonitoredItems_single(UA_Server *server, UA_Session *session, UA_Subscription *sub,
                                    const UA_TimestampsToReturn timestampsToReturn,
                                    const UA_MonitoredItemCreateRequest *request,
                                    UA_MonitoredItemCreateResult *result) {
    /* Check if the target exists */
    const UA_Node *target = UA_NodeStore_get(server->nodestore, &request->itemToMonitor.nodeId);
    if(!target) {
        result->statusCode = UA_STATUSCODE_BADNODEIDINVALID;
        return;
    }
    // TODO: Check if the target node type has the requested attribute

    /* Check if the encoding is supported */
    if(request->itemToMonitor.dataEncoding.name.length > 0 &&
       !UA_String_equal(&binaryEncoding, &request->itemToMonitor.dataEncoding.name)) {
        result->statusCode = UA_STATUSCODE_BADDATAENCODINGUNSUPPORTED;
        return;
    }

    /* Create the monitoreditem */
    UA_MonitoredItem *newMon = UA_MonitoredItem_new();
    if(!newMon) {
        result->statusCode = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }
    UA_StatusCode retval = UA_NodeId_copy(&target->nodeId, &newMon->monitoredNodeId);
    if(retval != UA_STATUSCODE_GOOD) {
        result->statusCode = retval;
        MonitoredItem_delete(server, newMon);
        return;
    }
    newMon->subscription = sub;
    newMon->attributeID = request->itemToMonitor.attributeId;
    newMon->itemId = UA_Session_getUniqueSubscriptionID(session);
    newMon->timestampsToReturn = timestampsToReturn;
    setMonitoredItemSettings(server, newMon, request->monitoringMode,
                             request->requestedParameters.clientHandle,
                             request->requestedParameters.samplingInterval,
                             request->requestedParameters.queueSize,
                             request->requestedParameters.discardOldest);
    LIST_INSERT_HEAD(&sub->MonitoredItems, newMon, listEntry);

    /* Prepare the response */
    UA_String_copy(&request->itemToMonitor.indexRange, &newMon->indexRange);
    result->revisedSamplingInterval = newMon->samplingInterval;
    result->revisedQueueSize = newMon->maxQueueSize;
    result->monitoredItemId = newMon->itemId;
}

void
Service_CreateMonitoredItems(UA_Server *server, UA_Session *session,
                             const UA_CreateMonitoredItemsRequest *request,
                             UA_CreateMonitoredItemsResponse *response) {
    UA_Subscription *sub = UA_Session_getSubscriptionByID(session, request->subscriptionId);
    if(!sub) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
        return;
    }

    /* Reset the subscription lifetime */
    sub->currentLifetimeCount = 0;
    if(request->itemsToCreateSize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }

    response->results = UA_Array_new(request->itemsToCreateSize,
                                     &UA_TYPES[UA_TYPES_MONITOREDITEMCREATERESULT]);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }
    response->resultsSize = request->itemsToCreateSize;

    for(size_t i = 0; i < request->itemsToCreateSize; i++)
        Service_CreateMonitoredItems_single(server, session, sub, request->timestampsToReturn,
                                            &request->itemsToCreate[i], &response->results[i]);
}

static void
Service_ModifyMonitoredItems_single(UA_Server *server, UA_Session *session, UA_Subscription *sub,
                                    const UA_MonitoredItemModifyRequest *request,
                                    UA_MonitoredItemModifyResult *result) {
    UA_MonitoredItem *mon = UA_Subscription_getMonitoredItem(sub, request->monitoredItemId);
    if(!mon) {
        result->statusCode = UA_STATUSCODE_BADMONITOREDITEMIDINVALID;
        return;
    }

    setMonitoredItemSettings(server, mon, mon->monitoringMode,
                             request->requestedParameters.clientHandle,
                             request->requestedParameters.samplingInterval,
                             request->requestedParameters.queueSize,
                             request->requestedParameters.discardOldest);
    result->revisedSamplingInterval = mon->samplingInterval;
    result->revisedQueueSize = mon->maxQueueSize;
}

void Service_ModifyMonitoredItems(UA_Server *server, UA_Session *session,
                                  const UA_ModifyMonitoredItemsRequest *request,
                                  UA_ModifyMonitoredItemsResponse *response) {
    UA_Subscription *sub = UA_Session_getSubscriptionByID(session, request->subscriptionId);
    if(!sub) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
        return;
    }

    /* Reset the subscription lifetime */
    sub->currentLifetimeCount = 0;
    if(request->itemsToModifySize <= 0) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADNOTHINGTODO;
        return;
    }

    response->results = UA_Array_new(request->itemsToModifySize,
                                     &UA_TYPES[UA_TYPES_MONITOREDITEMMODIFYRESULT]);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }
    response->resultsSize = request->itemsToModifySize;

    for(size_t i = 0; i < request->itemsToModifySize; i++)
        Service_ModifyMonitoredItems_single(server, session, sub, &request->itemsToModify[i],
                                            &response->results[i]);

}

void
Service_Publish(UA_Server *server, UA_Session *session,
                const UA_PublishRequest *request, UA_UInt32 requestId) {
    /* Return an error if the session has no subscription */
    if(LIST_EMPTY(&session->serverSubscriptions)) {
        UA_PublishResponse response;
        UA_PublishResponse_init(&response);
        response.responseHeader.requestHandle = request->requestHeader.requestHandle;
        response.responseHeader.serviceResult = UA_STATUSCODE_BADNOSUBSCRIPTION;
        UA_SecureChannel_sendBinaryMessage(session->channel, requestId, &response,
                                           &UA_TYPES[UA_TYPES_PUBLISHRESPONSE]);
        return;
    }

    // todo error handling for malloc
    UA_PublishResponseEntry *entry = UA_malloc(sizeof(UA_PublishResponseEntry));
    entry->requestId = requestId;
    UA_PublishResponse *response = &entry->response;
    UA_PublishResponse_init(response);
    response->responseHeader.requestHandle = request->requestHeader.requestHandle;

    /* Delete Acknowledged Subscription Messages */
    response->results = UA_malloc(request->subscriptionAcknowledgementsSize * sizeof(UA_StatusCode));
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }
    response->resultsSize = request->subscriptionAcknowledgementsSize;
    for(size_t i = 0; i < request->subscriptionAcknowledgementsSize; i++) {
        UA_SubscriptionAcknowledgement *ack = &request->subscriptionAcknowledgements[i];
        UA_Subscription *sub = UA_Session_getSubscriptionByID(session, ack->subscriptionId);
        if(!sub) {
            response->results[i] = UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
            UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
                         "Cannot process acknowledgements subscription %u", ack->subscriptionId);
            continue;
        }

        response->results[i] = UA_STATUSCODE_BADSEQUENCENUMBERUNKNOWN;
        UA_NotificationMessageEntry *pre, *pre_tmp;
        LIST_FOREACH_SAFE(pre, &sub->retransmissionQueue, listEntry, pre_tmp) {
            if(pre->message.sequenceNumber == ack->sequenceNumber) {
                LIST_REMOVE(pre, listEntry);
                response->results[i] = UA_STATUSCODE_GOOD;
                UA_NotificationMessage_deleteMembers(&pre->message);
                UA_free(pre);
                break;
            }
        }
    }

    /* Queue the publish response */
    SIMPLEQ_INSERT_TAIL(&session->responseQueue, entry, listEntry);
    UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
                 "Queued a publication message on session %u",
                 session->authenticationToken.identifier.numeric);

    /* Answer immediately to a late subscription */
    UA_Subscription *immediate;
    LIST_FOREACH(immediate, &session->serverSubscriptions, listEntry) {
        if(immediate->state == UA_SUBSCRIPTIONSTATE_LATE) {
            UA_LOG_DEBUG(server->config.logger, UA_LOGCATEGORY_SERVER,
                         "Response on a late subscription on session %u",
                         session->authenticationToken.identifier.numeric);
            UA_Subscription_publishCallback(server, immediate);
            return;
        }
    }
}

void Service_DeleteSubscriptions(UA_Server *server, UA_Session *session,
                                 const UA_DeleteSubscriptionsRequest *request,
                                 UA_DeleteSubscriptionsResponse *response) {
    response->results = UA_malloc(sizeof(UA_StatusCode) * request->subscriptionIdsSize);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }
    response->resultsSize = request->subscriptionIdsSize;

    for(size_t i = 0; i < request->subscriptionIdsSize; i++)
        response->results[i] = UA_Session_deleteSubscription(server, session, request->subscriptionIds[i]);
}

void Service_DeleteMonitoredItems(UA_Server *server, UA_Session *session,
                                  const UA_DeleteMonitoredItemsRequest *request,
                                  UA_DeleteMonitoredItemsResponse *response) {
    UA_Subscription *sub = UA_Session_getSubscriptionByID(session, request->subscriptionId);
    if(!sub) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
        return;
    }

    /* Reset the subscription lifetime */
    sub->currentLifetimeCount = 0;
    response->results = UA_malloc(sizeof(UA_StatusCode) * request->monitoredItemIdsSize);
    if(!response->results) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADOUTOFMEMORY;
        return;
    }
    response->resultsSize = request->monitoredItemIdsSize;

    for(size_t i = 0; i < request->monitoredItemIdsSize; i++)
        response->results[i] = UA_Subscription_deleteMonitoredItem(server, sub, request->monitoredItemIds[i]);
}

void Service_Republish(UA_Server *server, UA_Session *session, const UA_RepublishRequest *request,
                       UA_RepublishResponse *response) {
    /* get the subscription */
    UA_Subscription *sub = UA_Session_getSubscriptionByID(session, request->subscriptionId);
    if (!sub) {
        response->responseHeader.serviceResult = UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
        return;
    }

    /* Reset the subscription lifetime */
    sub->currentLifetimeCount = 0;

    /* Find the notification in the retransmission queue  */
    UA_NotificationMessageEntry *entry;
    LIST_FOREACH(entry, &sub->retransmissionQueue, listEntry) {
        if(entry->message.sequenceNumber == request->retransmitSequenceNumber)
            break;
    }
    if(entry)
        response->responseHeader.serviceResult =
            UA_NotificationMessage_copy(&entry->message, &response->notificationMessage);
    else
      response->responseHeader.serviceResult = UA_STATUSCODE_BADMESSAGENOTAVAILABLE;
}

#endif /* UA_ENABLE_SUBSCRIPTIONS */

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/src/client/ua_client_highlevel_subscriptions.c" ***********************************/


#ifdef UA_ENABLE_SUBSCRIPTIONS /* conditional compilation */

const UA_SubscriptionSettings UA_SubscriptionSettings_standard = {
    .requestedPublishingInterval = 500.0,
    .requestedLifetimeCount = 10000,
    .requestedMaxKeepAliveCount = 1,
    .maxNotificationsPerPublish = 10,
    .publishingEnabled = true,
    .priority = 0
};

UA_StatusCode UA_Client_Subscriptions_new(UA_Client *client, UA_SubscriptionSettings settings,
                                          UA_UInt32 *newSubscriptionId) {
    UA_CreateSubscriptionRequest request;
    UA_CreateSubscriptionRequest_init(&request);
    request.requestedPublishingInterval = settings.requestedPublishingInterval;
    request.requestedLifetimeCount = settings.requestedLifetimeCount;
    request.requestedMaxKeepAliveCount = settings.requestedMaxKeepAliveCount;
    request.maxNotificationsPerPublish = settings.maxNotificationsPerPublish;
    request.publishingEnabled = settings.publishingEnabled;
    request.priority = settings.priority;
    
    UA_CreateSubscriptionResponse response = UA_Client_Service_createSubscription(client, request);
    UA_StatusCode retval = response.responseHeader.serviceResult;
    if(retval == UA_STATUSCODE_GOOD) {
        UA_Client_Subscription *newSub = UA_malloc(sizeof(UA_Client_Subscription));
        LIST_INIT(&newSub->MonitoredItems);
        newSub->LifeTime = response.revisedLifetimeCount;
        newSub->KeepAliveCount = response.revisedMaxKeepAliveCount;
        newSub->PublishingInterval = response.revisedPublishingInterval;
        newSub->SubscriptionID = response.subscriptionId;
        newSub->NotificationsPerPublish = request.maxNotificationsPerPublish;
        newSub->Priority = request.priority;
        if(newSubscriptionId)
            *newSubscriptionId = newSub->SubscriptionID;
        LIST_INSERT_HEAD(&client->subscriptions, newSub, listEntry);
    }
    
    UA_CreateSubscriptionResponse_deleteMembers(&response);
    return retval;
}

UA_StatusCode UA_Client_Subscriptions_remove(UA_Client *client, UA_UInt32 subscriptionId) {
    UA_Client_Subscription *sub;
    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    
    LIST_FOREACH(sub, &client->subscriptions, listEntry) {
        if(sub->SubscriptionID == subscriptionId)
            break;
    }
    
    // Problem? We do not have this subscription registeres. Maybe the server should
    // be consulted at this point?
    if(!sub)
        return UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
    
    UA_DeleteSubscriptionsRequest request;
    UA_DeleteSubscriptionsRequest_init(&request);
    request.subscriptionIdsSize = 1;
    request.subscriptionIds = (UA_UInt32 *) UA_malloc(sizeof(UA_UInt32));
    *request.subscriptionIds = sub->SubscriptionID;
    
    UA_Client_MonitoredItem *mon, *tmpmon;
    LIST_FOREACH_SAFE(mon, &sub->MonitoredItems, listEntry, tmpmon) {
        retval |= UA_Client_Subscriptions_removeMonitoredItem(client, sub->SubscriptionID,
                                                              mon->MonitoredItemId);
    }
    if(retval != UA_STATUSCODE_GOOD) {
        UA_DeleteSubscriptionsRequest_deleteMembers(&request);
        return retval;
    }
    
    UA_DeleteSubscriptionsResponse response = UA_Client_Service_deleteSubscriptions(client, request);
    if(response.resultsSize > 0)
        retval = response.results[0];
    else
        retval = response.responseHeader.serviceResult;
    
    if(retval == UA_STATUSCODE_GOOD) {
        LIST_REMOVE(sub, listEntry);
        UA_free(sub);
    }
    UA_DeleteSubscriptionsRequest_deleteMembers(&request);
    UA_DeleteSubscriptionsResponse_deleteMembers(&response);
    return retval;
}

UA_StatusCode
UA_Client_Subscriptions_addMonitoredItem(UA_Client *client, UA_UInt32 subscriptionId,
                                         UA_NodeId nodeId, UA_UInt32 attributeID,
                                         UA_MonitoredItemHandlingFunction handlingFunction,
                                         void *handlingContext, UA_UInt32 *newMonitoredItemId) {
    UA_Client_Subscription *sub;
    LIST_FOREACH(sub, &client->subscriptions, listEntry) {
        if(sub->SubscriptionID == subscriptionId)
            break;
    }
    if(!sub)
        return UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
    
    /* Send the request */
    UA_CreateMonitoredItemsRequest request;
    UA_CreateMonitoredItemsRequest_init(&request);
    request.subscriptionId = subscriptionId;
    UA_MonitoredItemCreateRequest item;
    UA_MonitoredItemCreateRequest_init(&item);
    item.itemToMonitor.nodeId = nodeId;
    item.itemToMonitor.attributeId = attributeID;
    item.monitoringMode = UA_MONITORINGMODE_REPORTING;
    item.requestedParameters.clientHandle = ++(client->monitoredItemHandles);
    item.requestedParameters.samplingInterval = sub->PublishingInterval;
    item.requestedParameters.discardOldest = true;
    item.requestedParameters.queueSize = 1;
    request.itemsToCreate = &item;
    request.itemsToCreateSize = 1;
    UA_CreateMonitoredItemsResponse response = UA_Client_Service_createMonitoredItems(client, request);
    
    // slight misuse of retval here to check if the deletion was successfull.
    UA_StatusCode retval;
    if(response.resultsSize == 0)
        retval = response.responseHeader.serviceResult;
    else
        retval = response.results[0].statusCode;
    if(retval != UA_STATUSCODE_GOOD) {
        UA_CreateMonitoredItemsResponse_deleteMembers(&response);
        return retval;
    }

    /* Create the handler */
    UA_Client_MonitoredItem *newMon = UA_malloc(sizeof(UA_Client_MonitoredItem));
    newMon->MonitoringMode = UA_MONITORINGMODE_REPORTING;
    UA_NodeId_copy(&nodeId, &newMon->monitoredNodeId); 
    newMon->AttributeID = attributeID;
    newMon->ClientHandle = client->monitoredItemHandles;
    newMon->SamplingInterval = sub->PublishingInterval;
    newMon->QueueSize = 1;
    newMon->DiscardOldest = true;
    newMon->handler = handlingFunction;
    newMon->handlerContext = handlingContext;
    newMon->MonitoredItemId = response.results[0].monitoredItemId;
    LIST_INSERT_HEAD(&sub->MonitoredItems, newMon, listEntry);
    *newMonitoredItemId = newMon->MonitoredItemId;

    UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_CLIENT,
                 "Created a monitored item with client handle %u", client->monitoredItemHandles);
    
    UA_CreateMonitoredItemsResponse_deleteMembers(&response);
    return UA_STATUSCODE_GOOD;
}

UA_StatusCode
UA_Client_Subscriptions_removeMonitoredItem(UA_Client *client, UA_UInt32 subscriptionId,
                                            UA_UInt32 monitoredItemId) {
    UA_Client_Subscription *sub;
    LIST_FOREACH(sub, &client->subscriptions, listEntry) {
        if(sub->SubscriptionID == subscriptionId)
            break;
    }
    if(!sub)
        return UA_STATUSCODE_BADSUBSCRIPTIONIDINVALID;
    
    UA_Client_MonitoredItem *mon;
    LIST_FOREACH(mon, &sub->MonitoredItems, listEntry) {
        if(mon->MonitoredItemId == monitoredItemId)
            break;
    }
    if(!mon)
        return UA_STATUSCODE_BADMONITOREDITEMIDINVALID;
    
    UA_DeleteMonitoredItemsRequest request;
    UA_DeleteMonitoredItemsRequest_init(&request);
    request.subscriptionId = sub->SubscriptionID;
    request.monitoredItemIdsSize = 1;
    request.monitoredItemIds = (UA_UInt32 *) UA_malloc(sizeof(UA_UInt32));
    request.monitoredItemIds[0] = mon->MonitoredItemId;
    
    UA_DeleteMonitoredItemsResponse response = UA_Client_Service_deleteMonitoredItems(client, request);

    UA_StatusCode retval = UA_STATUSCODE_GOOD;
    if(response.resultsSize > 1)
        retval = response.results[0];
    else
        retval = response.responseHeader.serviceResult;
    
    if(retval == UA_STATUSCODE_GOOD) {
        LIST_REMOVE(mon, listEntry);
        UA_NodeId_deleteMembers(&mon->monitoredNodeId);
        UA_free(mon);
    }
    
    UA_DeleteMonitoredItemsRequest_deleteMembers(&request);
    UA_DeleteMonitoredItemsResponse_deleteMembers(&response);
    return retval;
}

static void
UA_Client_processPublishResponse(UA_Client *client, UA_PublishResponse *response) {
    if(response->responseHeader.serviceResult != UA_STATUSCODE_GOOD)
        return;

    /* Find the subscription */
    UA_Client_Subscription *sub;
    LIST_FOREACH(sub, &client->subscriptions, listEntry) {
        if(sub->SubscriptionID == response->subscriptionId)
            break;
    }
    if(!sub)
        return;

    UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_CLIENT,
                 "Processing a publish response on subscription %u with %u notifications",
                 sub->SubscriptionID, response->notificationMessage.notificationDataSize);

    /* Check if the server has acknowledged any of our ACKS */
    // TODO: The acks should be attached to the subscription
    UA_Client_NotificationsAckNumber *ack, *tmpAck;
    size_t i = 0;
    LIST_FOREACH_SAFE(ack, &client->pendingNotificationsAcks, listEntry, tmpAck) {
        if(response->results[i] == UA_STATUSCODE_GOOD ||
           response->results[i] == UA_STATUSCODE_BADSEQUENCENUMBERUNKNOWN) {
            LIST_REMOVE(ack, listEntry);
            UA_free(ack);
        }
        i++;
    }
    
    /* Process the notification messages */
    UA_NotificationMessage *msg = &response->notificationMessage;
    for(size_t k = 0; k < msg->notificationDataSize; k++) {
        if(msg->notificationData[k].encoding != UA_EXTENSIONOBJECT_DECODED)
            continue;
        
        /* Currently only dataChangeNotifications are supported */
        if(msg->notificationData[k].content.decoded.type != &UA_TYPES[UA_TYPES_DATACHANGENOTIFICATION])
            continue;
        
        UA_DataChangeNotification *dataChangeNotification = msg->notificationData[k].content.decoded.data;
        for(size_t j = 0; j < dataChangeNotification->monitoredItemsSize; j++) {
            UA_MonitoredItemNotification *mitemNot = &dataChangeNotification->monitoredItems[j];
            UA_Client_MonitoredItem *mon;
            LIST_FOREACH(mon, &sub->MonitoredItems, listEntry) {
                if(mon->ClientHandle == mitemNot->clientHandle) {
                    mon->handler(mon->MonitoredItemId, &mitemNot->value, mon->handlerContext);
                    break;
                }
            }
            if(!mon)
                UA_LOG_DEBUG(client->config.logger, UA_LOGCATEGORY_CLIENT,
                             "Could not process a notification with clienthandle %u on subscription %u",
                             mitemNot->clientHandle, sub->SubscriptionID);
        }
    }
    
    /* Add to the list of pending acks */
    tmpAck = UA_malloc(sizeof(UA_Client_NotificationsAckNumber));
    tmpAck->subAck.sequenceNumber = msg->sequenceNumber;
    tmpAck->subAck.subscriptionId = sub->SubscriptionID;
    LIST_INSERT_HEAD(&client->pendingNotificationsAcks, tmpAck, listEntry);
}

UA_StatusCode UA_Client_Subscriptions_manuallySendPublishRequest(UA_Client *client) {
    if (client->state == UA_CLIENTSTATE_ERRORED)
        return UA_STATUSCODE_BADSERVERNOTCONNECTED;

    UA_Boolean moreNotifications = true;
    while(moreNotifications == true) {
        UA_PublishRequest request;
        UA_PublishRequest_init(&request);
        request.subscriptionAcknowledgementsSize = 0;

        UA_Client_NotificationsAckNumber *ack;
        LIST_FOREACH(ack, &client->pendingNotificationsAcks, listEntry)
            request.subscriptionAcknowledgementsSize++;
        if(request.subscriptionAcknowledgementsSize > 0) {
            request.subscriptionAcknowledgements =
                UA_malloc(sizeof(UA_SubscriptionAcknowledgement) * request.subscriptionAcknowledgementsSize);
            if(!request.subscriptionAcknowledgements)
                return UA_STATUSCODE_GOOD;
        }
        
        int index = 0 ;
        LIST_FOREACH(ack, &client->pendingNotificationsAcks, listEntry) {
            request.subscriptionAcknowledgements[index].sequenceNumber = ack->subAck.sequenceNumber;
            request.subscriptionAcknowledgements[index].subscriptionId = ack->subAck.subscriptionId;
            index++;
        }
        
        UA_PublishResponse response = UA_Client_Service_publish(client, request);
        UA_Client_processPublishResponse(client, &response);
        moreNotifications = response.moreNotifications;
        
        UA_PublishResponse_deleteMembers(&response);
        UA_PublishRequest_deleteMembers(&request);
    }
    return UA_STATUSCODE_GOOD;
}

#endif /* UA_ENABLE_SUBSCRIPTIONS */

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/plugins/networklayer_tcp.c" ***********************************/

 /*
 * This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information.
 */


#include <stdlib.h> // malloc, free
#include <stdio.h> // snprintf
#include <string.h> // memset
#include <errno.h>

#ifdef _WIN32
# include <malloc.h>
# include <winsock2.h>
# include <ws2tcpip.h>
# define CLOSESOCKET(S) closesocket(S)
# define ssize_t long
#else
# include <fcntl.h>
# include <sys/select.h>
# include <netinet/in.h>
# ifndef __CYGWIN__
#  include <netinet/tcp.h>
# endif
# include <sys/ioctl.h>
# include <netdb.h> //gethostbyname for the client
# include <unistd.h> // read, write, close
# include <arpa/inet.h>
# ifdef __QNX__
#  include <sys/socket.h>
# endif
# define CLOSESOCKET(S) close(S)
#endif

/* workaround a glibc bug where an integer conversion is required */
#if !defined(_WIN32)
# if defined(__GNU_LIBRARY__) && (__GNU_LIBRARY__ >= 6) && (__GLIBC__ >= 2) && (__GLIBC_MINOR__ >= 16)
#  define UA_fd_set(fd, fds) FD_SET(fd, fds)
#  define UA_fd_isset(fd, fds) FD_ISSET(fd, fds)
# else
#  define UA_fd_set(fd, fds) FD_SET((unsigned int)fd, fds)
#  define UA_fd_isset(fd, fds) FD_ISSET((unsigned int)fd, fds)
# endif
#else
# define UA_fd_set(fd, fds) FD_SET((unsigned int)fd, fds)
# define UA_fd_isset(fd, fds) FD_ISSET((unsigned int)fd, fds)
#endif

#ifdef UA_ENABLE_MULTITHREADING
# include <urcu/uatomic.h>
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

/****************************/
/* Generic Socket Functions */
/****************************/

static void
socket_close(UA_Connection *connection) {
    connection->state = UA_CONNECTION_CLOSED;
    shutdown(connection->sockfd,2);
    CLOSESOCKET(connection->sockfd);
}

static UA_StatusCode
socket_write(UA_Connection *connection, UA_ByteString *buf) {
    size_t nWritten = 0;
    do {
        ssize_t n = 0;
        do {
#ifdef _WIN32
            n = send((SOCKET)connection->sockfd, (const char*)buf->data, buf->length, 0);
            const int last_error = WSAGetLastError();
            if(n < 0 && last_error != WSAEINTR && last_error != WSAEWOULDBLOCK) {
                connection->close(connection);
                socket_close(connection);
                UA_ByteString_deleteMembers(buf);
                return UA_STATUSCODE_BADCONNECTIONCLOSED;
            }
#else
            n = send(connection->sockfd, (const char*)buf->data, buf->length, MSG_NOSIGNAL);
            if(n == -1L && errno != EINTR && errno != EAGAIN) {
                connection->close(connection);
                socket_close(connection);
                UA_ByteString_deleteMembers(buf);
                return UA_STATUSCODE_BADCONNECTIONCLOSED;
            }
#endif
        } while(n == -1L);
        nWritten += (size_t)n;
    } while(nWritten < buf->length);
    UA_ByteString_deleteMembers(buf);
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
socket_recv(UA_Connection *connection, UA_ByteString *response, UA_UInt32 timeout) {
    response->data = malloc(connection->localConf.recvBufferSize);
    if(!response->data) {
        response->length = 0;
        return UA_STATUSCODE_BADOUTOFMEMORY; /* not enough memory retry */
    }

    if(timeout > 0) {
        /* currently, only the client uses timeouts */
#ifndef _WIN32
        UA_UInt32 timeout_usec = timeout * 1000;
    #ifdef __APPLE__
        struct timeval tmptv = {(long int)(timeout_usec / 1000000), timeout_usec % 1000000};
	#else
        struct timeval tmptv = {(long int)(timeout_usec / 1000000), (long int)(timeout_usec % 1000000)};
	#endif
        int ret = setsockopt(connection->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tmptv, sizeof(struct timeval));
#else
        DWORD timeout_dw = timeout;
        int ret = setsockopt(connection->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_dw, sizeof(DWORD));
#endif
        if(0 != ret) {
            UA_ByteString_deleteMembers(response);
            socket_close(connection);
            return UA_STATUSCODE_BADCONNECTIONCLOSED;
        }
    }

#ifdef __CYGWIN__
    /* WORKAROUND for https://cygwin.com/ml/cygwin/2013-07/msg00107.html */
    ssize_t ret;

    if (timeout > 0) {
        fd_set fdset;
        UA_UInt32 timeout_usec = timeout * 1000;
    #ifdef __APPLE__
        struct timeval tmptv = {(long int)(timeout_usec / 1000000), timeout_usec % 1000000};
	#else
        struct timeval tmptv = {(long int)(timeout_usec / 1000000), (long int)(timeout_usec % 1000000)};
	#endif
        UA_Int32 retval;

        FD_ZERO(&fdset);
        UA_fd_set(connection->sockfd, &fdset);
        retval = select(connection->sockfd+1, &fdset, NULL, NULL, &tmptv);
        if(retval && UA_fd_isset(connection->sockfd, &fdset)) {
            ret = recv(connection->sockfd, (char*)response->data, connection->localConf.recvBufferSize, 0);
        } else {
            ret = 0;
        }
    } else {
        ret = recv(connection->sockfd, (char*)response->data, connection->localConf.recvBufferSize, 0);
    }
#else
    ssize_t ret = recv(connection->sockfd, (char*)response->data, connection->localConf.recvBufferSize, 0);
#endif

    if(ret == 0) {
        /* server has closed the connection */
        UA_ByteString_deleteMembers(response);
        socket_close(connection);
        return UA_STATUSCODE_BADCONNECTIONCLOSED;
    } else if(ret < 0) {
        UA_ByteString_deleteMembers(response);
#ifdef _WIN32
        const int last_error = WSAGetLastError();
        #define TEST_RETRY (last_error == WSAEINTR || (timeout > 0) ? 0 : (last_error == WSAEWOULDBLOCK))
#else
        #define TEST_RETRY (errno == EINTR || (timeout > 0) ? 0 : (errno == EAGAIN || errno == EWOULDBLOCK))
#endif
        if (TEST_RETRY)
            return UA_STATUSCODE_GOOD; /* retry */
        else {
            socket_close(connection);
            return UA_STATUSCODE_BADCONNECTIONCLOSED;
        }
    }
    response->length = (size_t)ret;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode socket_set_nonblocking(UA_Int32 sockfd) {
#ifdef _WIN32
    u_long iMode = 1;
    if(ioctlsocket(sockfd, FIONBIO, &iMode) != NO_ERROR)
        return UA_STATUSCODE_BADINTERNALERROR;
#else
    int opts = fcntl(sockfd, F_GETFL);
    if(opts < 0 || fcntl(sockfd, F_SETFL, opts|O_NONBLOCK) < 0)
        return UA_STATUSCODE_BADINTERNALERROR;
#endif
    return UA_STATUSCODE_GOOD;
}

static void FreeConnectionCallback(UA_Server *server, void *ptr) {
    UA_Connection_deleteMembers((UA_Connection*)ptr);
    free(ptr);
 }

/***************************/
/* Server NetworkLayer TCP */
/***************************/

/**
 * For the multithreaded mode, assume a single thread that periodically "gets work" from the network
 * layer. In addition, several worker threads are asynchronously calling into the callbacks of the
 * UA_Connection that holds a single connection.
 *
 * Creating a connection: When "GetWork" encounters a new connection, it creates a UA_Connection
 * with the socket information. This is added to the mappings array that links sockets to
 * UA_Connection structs.
 *
 * Reading data: In "GetWork", we listen on the sockets in the mappings array. If data arrives (or
 * the connection closes), a WorkItem is created that carries the work and a pointer to the
 * connection.
 *
 * Closing a connection: Closing can happen in two ways. Either it is triggered by the server in an
 * asynchronous callback. Or the connection is close by the client and this is detected in
 * "GetWork". The server needs to do some internal cleanups (close attached securechannels, etc.).
 * So even when a closed connection is detected in "GetWork", we trigger the server to close the
 * connection (with a WorkItem) and continue from the callback.
 *
 * - Server calls close-callback: We close the socket, set the connection-state to closed and add
 *   the connection to a linked list from which it is deleted later. The connection cannot be freed
 *   right away since other threads might still be using it.
 *
 * - GetWork: We remove the connection from the mappings array. In the non-multithreaded case, the
 *   connection is freed. For multithreading, we return a workitem that is delayed, i.e. that is
 *   called only after all workitems created before are finished in all threads. This workitems
 *   contains a callback that goes through the linked list of connections to be freed.
 *
 */

#define MAXBACKLOG 100

typedef struct {
    UA_ConnectionConfig conf;
    UA_UInt16 port;
    UA_Logger logger; // Set during start
    
    /* open sockets and connections */
    UA_Int32 serversockfd;
    size_t mappingsSize;
    struct ConnectionMapping {
        UA_Connection *connection;
        UA_Int32 sockfd;
    } *mappings;
} ServerNetworkLayerTCP;

static UA_StatusCode
ServerNetworkLayerGetSendBuffer(UA_Connection *connection, size_t length, UA_ByteString *buf) {
    if(length > connection->remoteConf.recvBufferSize)
        return UA_STATUSCODE_BADCOMMUNICATIONERROR;
    return UA_ByteString_allocBuffer(buf, length);
}

static void
ServerNetworkLayerReleaseSendBuffer(UA_Connection *connection, UA_ByteString *buf) {
    UA_ByteString_deleteMembers(buf);
}

static void
ServerNetworkLayerReleaseRecvBuffer(UA_Connection *connection, UA_ByteString *buf) {
    UA_ByteString_deleteMembers(buf);
}

/* after every select, we need to reset the sockets we want to listen on */
static UA_Int32
setFDSet(ServerNetworkLayerTCP *layer, fd_set *fdset) {
    FD_ZERO(fdset);
    UA_fd_set(layer->serversockfd, fdset);
    UA_Int32 highestfd = layer->serversockfd;
    for(size_t i = 0; i < layer->mappingsSize; i++) {
        UA_fd_set(layer->mappings[i].sockfd, fdset);
        if(layer->mappings[i].sockfd > highestfd)
            highestfd = layer->mappings[i].sockfd;
    }
    return highestfd;
}

/* callback triggered from the server */
static void
ServerNetworkLayerTCP_closeConnection(UA_Connection *connection) {
#ifdef UA_ENABLE_MULTITHREADING
    if(uatomic_xchg(&connection->state, UA_CONNECTION_CLOSED) == UA_CONNECTION_CLOSED)
        return;
#else
    if(connection->state == UA_CONNECTION_CLOSED)
        return;
    connection->state = UA_CONNECTION_CLOSED;
#endif
    //cppcheck-suppress unreadVariable
    ServerNetworkLayerTCP *layer = connection->handle;
    UA_LOG_INFO(layer->logger, UA_LOGCATEGORY_NETWORK, "Closing the Connection %i",
                connection->sockfd);
    /* only "shutdown" here. this triggers the select, where the socket is
       "closed" in the mainloop */
    shutdown(connection->sockfd, 2);
}

/* call only from the single networking thread */
static UA_StatusCode
ServerNetworkLayerTCP_add(ServerNetworkLayerTCP *layer, UA_Int32 newsockfd) {
    UA_Connection *c = malloc(sizeof(UA_Connection));
    if(!c)
        return UA_STATUSCODE_BADINTERNALERROR;

    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    getpeername(newsockfd, (struct sockaddr*)&addr, &addrlen);
    UA_LOG_INFO(layer->logger, UA_LOGCATEGORY_NETWORK, "New Connection %i over TCP from %s:%d",
                newsockfd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
    UA_Connection_init(c);
    c->sockfd = newsockfd;
    c->handle = layer;
    c->localConf = layer->conf;
    c->send = socket_write;
    c->close = ServerNetworkLayerTCP_closeConnection;
    c->getSendBuffer = ServerNetworkLayerGetSendBuffer;
    c->releaseSendBuffer = ServerNetworkLayerReleaseSendBuffer;
    c->releaseRecvBuffer = ServerNetworkLayerReleaseRecvBuffer;
    c->state = UA_CONNECTION_OPENING;
    struct ConnectionMapping *nm;
    nm = realloc(layer->mappings, sizeof(struct ConnectionMapping)*(layer->mappingsSize+1));
    if(!nm) {
        UA_LOG_ERROR(layer->logger, UA_LOGCATEGORY_NETWORK, "No memory for a new Connection");
        free(c);
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    layer->mappings = nm;
    layer->mappings[layer->mappingsSize] = (struct ConnectionMapping){c, newsockfd};
    layer->mappingsSize++;
    return UA_STATUSCODE_GOOD;
}

static UA_StatusCode
ServerNetworkLayerTCP_start(UA_ServerNetworkLayer *nl, UA_Logger logger) {
    ServerNetworkLayerTCP *layer = nl->handle;
    layer->logger = logger;

    /* get the discovery url from the hostname */
    UA_String du = UA_STRING_NULL;
    char hostname[256];
    if(gethostname(hostname, 255) == 0) {
        char discoveryUrl[256];
#ifndef _MSC_VER
        du.length = (size_t)snprintf(discoveryUrl, 255, "opc.tcp://%s:%d", hostname, layer->port);
#else
        du.length = (size_t)_snprintf_s(discoveryUrl, 255, _TRUNCATE, "opc.tcp://%s:%d", hostname, layer->port);
#endif
        du.data = (UA_Byte*)discoveryUrl;
    }
    UA_String_copy(&du, &nl->discoveryUrl);
    
    /* open the server socket */
#ifdef _WIN32
    if((layer->serversockfd = socket(PF_INET, SOCK_STREAM,0)) == (UA_Int32)INVALID_SOCKET) {
        UA_LOG_WARNING(layer->logger, UA_LOGCATEGORY_NETWORK, "Error opening socket, code: %d",
                       WSAGetLastError());
        return UA_STATUSCODE_BADINTERNALERROR;
    }
#else
    if((layer->serversockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        UA_LOG_WARNING(layer->logger, UA_LOGCATEGORY_NETWORK, "Error opening socket");
        return UA_STATUSCODE_BADINTERNALERROR;
    }
#endif
    const struct sockaddr_in serv_addr =
        {.sin_family = AF_INET, .sin_addr.s_addr = INADDR_ANY,
         .sin_port = htons(layer->port), .sin_zero = {0}};
    int optval = 1;
    if(setsockopt(layer->serversockfd, SOL_SOCKET,
                  SO_REUSEADDR, (const char *)&optval, sizeof(optval)) == -1) {
        UA_LOG_WARNING(layer->logger, UA_LOGCATEGORY_NETWORK,
                       "Error during setting of socket options");
        CLOSESOCKET(layer->serversockfd);
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    if(bind(layer->serversockfd, (const struct sockaddr *)&serv_addr,
            sizeof(serv_addr)) < 0) {
        UA_LOG_WARNING(layer->logger, UA_LOGCATEGORY_NETWORK, "Error during socket binding");
        CLOSESOCKET(layer->serversockfd);
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    socket_set_nonblocking(layer->serversockfd);
    listen(layer->serversockfd, MAXBACKLOG);
    UA_LOG_INFO(layer->logger, UA_LOGCATEGORY_NETWORK, "TCP network layer listening on %.*s",
                nl->discoveryUrl.length, nl->discoveryUrl.data);
    return UA_STATUSCODE_GOOD;
}

static size_t
ServerNetworkLayerTCP_getJobs(UA_ServerNetworkLayer *nl, UA_Job **jobs, UA_UInt16 timeout) {
    ServerNetworkLayerTCP *layer = nl->handle;
    fd_set fdset, errset;
    UA_Int32 highestfd = setFDSet(layer, &fdset);
    setFDSet(layer, &errset);
    struct timeval tmptv = {0, timeout * 1000};
    UA_Int32 resultsize;
    resultsize = select(highestfd+1, &fdset, NULL, &errset, &tmptv);
    if(resultsize < 0) {
        *jobs = NULL;
        return 0;
    }

    /* accept new connections (can only be a single one) */
    if(UA_fd_isset(layer->serversockfd, &fdset)) {
        resultsize--;
        struct sockaddr_in cli_addr;
        socklen_t cli_len = sizeof(cli_addr);
        int newsockfd = accept(layer->serversockfd, (struct sockaddr *) &cli_addr, &cli_len);
        int i = 1;
        setsockopt(newsockfd, IPPROTO_TCP, TCP_NODELAY, (void *)&i, sizeof(i));
        if(newsockfd >= 0) {
            socket_set_nonblocking(newsockfd);
            ServerNetworkLayerTCP_add(layer, newsockfd);
        }
    }

    /* alloc enough space for a cleanup-connection and free-connection job per resulted socket */
    if(resultsize == 0)
        return 0;
    UA_Job *js = malloc(sizeof(UA_Job) * (size_t)resultsize * 2);
    if(!js)
        return 0;

    /* read from established sockets */
    size_t j = 0;
    UA_ByteString buf = UA_BYTESTRING_NULL;
    for(size_t i = 0; i < layer->mappingsSize && j < (size_t)resultsize; i++) {
        if(!UA_fd_isset(layer->mappings[i].sockfd, &errset) && !UA_fd_isset(layer->mappings[i].sockfd, &fdset)) {
          continue;
        }
        UA_StatusCode retval = socket_recv(layer->mappings[i].connection, &buf, 0);
        if(retval == UA_STATUSCODE_GOOD) {
            js[j].job.binaryMessage.connection = layer->mappings[i].connection;
            js[j].job.binaryMessage.message = buf;
            js[j].type = UA_JOBTYPE_BINARYMESSAGE_NETWORKLAYER;
            j++;
        } else if (retval == UA_STATUSCODE_BADCONNECTIONCLOSED) {
            UA_Connection *c = layer->mappings[i].connection;
            /* the socket was closed from remote */
            js[j].type = UA_JOBTYPE_DETACHCONNECTION;
            js[j].job.closeConnection = layer->mappings[i].connection;
            layer->mappings[i] = layer->mappings[layer->mappingsSize-1];
            layer->mappingsSize--;
            j++;
            js[j].type = UA_JOBTYPE_METHODCALL_DELAYED;
            js[j].job.methodCall.method = FreeConnectionCallback;
            js[j].job.methodCall.data = c;
            j++;
        }
    }

    if(j == 0) {
    	free(js);
    	js = NULL;
    }

    *jobs = js;
    return j;
}

static size_t
ServerNetworkLayerTCP_stop(UA_ServerNetworkLayer *nl, UA_Job **jobs) {
    ServerNetworkLayerTCP *layer = nl->handle;
    UA_LOG_INFO(layer->logger, UA_LOGCATEGORY_NETWORK,
                "Shutting down the TCP network layer with %d open connection(s)", layer->mappingsSize);
    shutdown(layer->serversockfd,2);
    CLOSESOCKET(layer->serversockfd);
    UA_Job *items = malloc(sizeof(UA_Job) * layer->mappingsSize * 2);
    if(!items)
        return 0;
    for(size_t i = 0; i < layer->mappingsSize; i++) {
        socket_close(layer->mappings[i].connection);
        items[i*2].type = UA_JOBTYPE_DETACHCONNECTION;
        items[i*2].job.closeConnection = layer->mappings[i].connection;
        items[(i*2)+1].type = UA_JOBTYPE_METHODCALL_DELAYED;
        items[(i*2)+1].job.methodCall.method = FreeConnectionCallback;
        items[(i*2)+1].job.methodCall.data = layer->mappings[i].connection;
    }
#ifdef _WIN32
    WSACleanup();
#endif
    *jobs = items;
    return layer->mappingsSize*2;
}

/* run only when the server is stopped */
static void ServerNetworkLayerTCP_deleteMembers(UA_ServerNetworkLayer *nl) {
    ServerNetworkLayerTCP *layer = nl->handle;
    free(layer->mappings);
    free(layer);
    UA_String_deleteMembers(&nl->discoveryUrl);
}

UA_ServerNetworkLayer
UA_ServerNetworkLayerTCP(UA_ConnectionConfig conf, UA_UInt16 port) {
#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);
#endif

    UA_ServerNetworkLayer nl;
    memset(&nl, 0, sizeof(UA_ServerNetworkLayer));
    ServerNetworkLayerTCP *layer = calloc(1,sizeof(ServerNetworkLayerTCP));
    if(!layer)
        return nl;
    
    layer->conf = conf;
    layer->port = port;

    nl.handle = layer;
    nl.start = ServerNetworkLayerTCP_start;
    nl.getJobs = ServerNetworkLayerTCP_getJobs;
    nl.stop = ServerNetworkLayerTCP_stop;
    nl.deleteMembers = ServerNetworkLayerTCP_deleteMembers;
    return nl;
}

/***************************/
/* Client NetworkLayer TCP */
/***************************/

static UA_StatusCode
ClientNetworkLayerGetBuffer(UA_Connection *connection, size_t length, UA_ByteString *buf) {
    if(length > connection->remoteConf.recvBufferSize)
        return UA_STATUSCODE_BADCOMMUNICATIONERROR;
    if(connection->state == UA_CONNECTION_CLOSED)
        return UA_STATUSCODE_BADCONNECTIONCLOSED;
    return UA_ByteString_allocBuffer(buf, connection->remoteConf.recvBufferSize);
}

static void
ClientNetworkLayerReleaseBuffer(UA_Connection *connection, UA_ByteString *buf) {
    UA_ByteString_deleteMembers(buf);
}

static void
ClientNetworkLayerClose(UA_Connection *connection) {
#ifdef UA_ENABLE_MULTITHREADING
    if(uatomic_xchg(&connection->state, UA_CONNECTION_CLOSED) == UA_CONNECTION_CLOSED)
        return;
#else
    if(connection->state == UA_CONNECTION_CLOSED)
        return;
    connection->state = UA_CONNECTION_CLOSED;
#endif
    socket_close(connection);
}

/* we have no networklayer. instead, attach the reusable buffer to the handle */
UA_Connection
UA_ClientConnectionTCP(UA_ConnectionConfig localConf, const char *endpointUrl, UA_Logger logger) {
    UA_Connection connection;
    UA_Connection_init(&connection);
    connection.localConf = localConf;

    //socket_set_nonblocking(connection.sockfd);
    connection.send = socket_write;
    connection.recv = socket_recv;
    connection.close = ClientNetworkLayerClose;
    connection.getSendBuffer = ClientNetworkLayerGetBuffer;
    connection.releaseSendBuffer = ClientNetworkLayerReleaseBuffer;
    connection.releaseRecvBuffer = ClientNetworkLayerReleaseBuffer;

    size_t urlLength = strlen(endpointUrl);
    if(urlLength < 11 || urlLength >= 512) {
        UA_LOG_WARNING((*logger), UA_LOGCATEGORY_NETWORK, "Server url size invalid");
        return connection;
    }
    if(strncmp(endpointUrl, "opc.tcp://", 10) != 0) {
        UA_LOG_WARNING((*logger), UA_LOGCATEGORY_NETWORK, "Server url does not begin with opc.tcp://");
        return connection;
    }

    UA_UInt16 portpos = 9;
    UA_UInt16 port;
    for(port = 0; portpos < urlLength-1; portpos++) {
        if(endpointUrl[portpos] == ':') {
            char *endPtr = NULL;
            unsigned long int tempulong = strtoul(&endpointUrl[portpos+1], &endPtr, 10);
            if (ERANGE != errno && tempulong < UINT16_MAX && endPtr != &endpointUrl[portpos+1])
                port = (UA_UInt16)tempulong;
            break;
        }
    }
    if(port == 0) {
        UA_LOG_WARNING((*logger), UA_LOGCATEGORY_NETWORK, "Port invalid");
        return connection;
    }

    char hostname[512];
    for(int i=10; i < portpos; i++)
        hostname[i-10] = endpointUrl[i];
    hostname[portpos-10] = 0;
#ifdef _WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);
    if((connection.sockfd = socket(PF_INET, SOCK_STREAM,0)) == (UA_Int32)INVALID_SOCKET) {
#else
    if((connection.sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
#endif
        UA_LOG_WARNING((*logger), UA_LOGCATEGORY_NETWORK, "Could not create socket");
        return connection;
    }
    struct hostent *server = gethostbyname(hostname);
    if(!server) {
        UA_LOG_WARNING((*logger), UA_LOGCATEGORY_NETWORK, "DNS lookup of %s failed", hostname);
        return connection;
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    memcpy((char *)&server_addr.sin_addr.s_addr, (char *)server->h_addr_list[0], (size_t)server->h_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    connection.state = UA_CONNECTION_OPENING;
    if(connect(connection.sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        ClientNetworkLayerClose(&connection);
        UA_LOG_WARNING((*logger), UA_LOGCATEGORY_NETWORK, "Connection failed");
        return connection;
    }

#ifdef SO_NOSIGPIPE
    int val = 1;
    if(setsockopt(connection.sockfd, SOL_SOCKET, SO_NOSIGPIPE, (void*)&val, sizeof(val)) < 0) {
        UA_LOG_WARNING((*logger), UA_LOGCATEGORY_NETWORK, "Couldn't set SO_NOSIGPIPE");
        return connection;
    }
#endif

    return connection;
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/plugins/logger_stdout.c" ***********************************/

/*
 * This work is licensed under a Creative Commons CCZero 1.0 Universal License.
 * See http://creativecommons.org/publicdomain/zero/1.0/ for more information.
 */

#include <stdio.h>
#include <stdarg.h>

const char *LogLevelNames[6] = {"trace", "debug", "info", "warning", "error", "fatal"};
const char *LogCategoryNames[6] = {"network", "channel", "session", "server", "client", "userland"};

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4 || defined(__clang__))
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#endif

void Logger_Stdout(UA_LogLevel level, UA_LogCategory category, const char *msg, ...) {
	UA_String t = UA_DateTime_toString(UA_DateTime_now());
    printf("[%.23s] %s/%s\t", t.data, LogLevelNames[level], LogCategoryNames[category]);
	UA_ByteString_deleteMembers(&t);
    va_list ap;
    va_start(ap, msg);
    vprintf(msg, ap);
    va_end(ap);
    printf("\n");
}

#if ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || __GNUC__ > 4 || defined(__clang__))
#pragma GCC diagnostic pop
#endif

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/plugins/ua_config_standard.c" ***********************************/


#define MANUFACTURER_NAME "open62541.org"
#define PRODUCT_NAME "open62541 OPC UA Server"
#define PRODUCT_URI "urn:unconfigured:open62541"
#define APPLICATION_NAME "open62541-based OPC UA Application"
#define APPLICATION_URI "urn:unconfigured:application"

#define UA_STRING_STATIC(s) {sizeof(s)-1, (UA_Byte*)s}
#define UA_STRING_STATIC_NULL {0, NULL}

UA_UsernamePasswordLogin usernamePasswords[2] = {
    { UA_STRING_STATIC("user1"), UA_STRING_STATIC("password") },
    { UA_STRING_STATIC("user2"), UA_STRING_STATIC("password1") } };

const UA_ServerConfig UA_ServerConfig_standard = {
    .nThreads = 1,
    .logger = Logger_Stdout,

    .buildInfo = {
        .productUri = UA_STRING_STATIC(PRODUCT_URI),
        .manufacturerName = UA_STRING_STATIC(MANUFACTURER_NAME),
        .productName = UA_STRING_STATIC(PRODUCT_NAME),
        .softwareVersion = UA_STRING_STATIC("0"),
        .buildNumber = UA_STRING_STATIC("0"),
        .buildDate = 0 },
    .applicationDescription = {
        .applicationUri = UA_STRING_STATIC(APPLICATION_URI),
        .productUri = UA_STRING_STATIC(PRODUCT_URI),
        .applicationName = { .locale = UA_STRING_STATIC(""),
                             .text = UA_STRING_STATIC(APPLICATION_NAME) },
        .applicationType = UA_APPLICATIONTYPE_SERVER,
        .gatewayServerUri = UA_STRING_STATIC_NULL,
        .discoveryProfileUri = UA_STRING_STATIC_NULL,
        .discoveryUrlsSize = 0,
        .discoveryUrls = NULL },
    .serverCertificate = UA_STRING_STATIC_NULL,

    .networkLayersSize = 0,
    .networkLayers = NULL,

    .enableAnonymousLogin = true,
    .enableUsernamePasswordLogin = true,
    .usernamePasswordLogins = usernamePasswords,
    .usernamePasswordLoginsSize = 2,

    .publishingIntervalLimits = { .min = 100.0, .max = 3600.0 * 1000.0 },
    .lifeTimeCountLimits = { .max = 15000, .min = 3 },
    .keepAliveCountLimits = { .max = 100, .min = 1 },
    .maxNotificationsPerPublish = 1000,
    .samplingIntervalLimits = { .min = 50.0, .max = 24.0 * 3600.0 * 1000.0 },
    .queueSizeLimits = { .max = 100, .min = 1 }
};

const UA_EXPORT UA_ClientConfig UA_ClientConfig_standard = {
    .timeout = 5000,
    .secureChannelLifeTime = 600000,
    .logger = Logger_Stdout,
    .localConnectionConfig = {
        .protocolVersion = 0,
        .sendBufferSize = 65536,
        .recvBufferSize  = 65536,
        .maxMessageSize = 65536,
        .maxChunkCount = 1 },
    .connectionFunc = UA_ClientConnectionTCP
};

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/deps/libc_time.c" ***********************************/

/*
 * Originally released by the musl project (http://www.musl-libc.org/) under the
 * MIT license. Taken from the file /src/time/__secs_to_tm.c
 */


/* 2000-03-01 (mod 400 year, immediately after feb29 */
#define LEAPOCH (946684800LL + 86400*(31+29))

#define DAYS_PER_400Y (365*400 + 97)
#define DAYS_PER_100Y (365*100 + 24)
#define DAYS_PER_4Y   (365*4   + 1)

int __secs_to_tm(long long t, struct tm *tm)
{
    long long days, secs, years;
    int remdays, remsecs, remyears;
    int qc_cycles, c_cycles, q_cycles;
    int months;
    int wday, yday, leap;
    static const char days_in_month[] = {31,30,31,30,31,31,30,31,30,31,31,29};

    /* Reject time_t values whose year would overflow int */
    if (t < INT_MIN * 31622400LL || t > INT_MAX * 31622400LL)
        return -1;

    secs = t - LEAPOCH;
    days = secs / 86400LL;
    remsecs = (int)(secs % 86400);
    if (remsecs < 0) {
        remsecs += 86400;
        days--;
    }

    wday = (int)((3+days)%7);
    if (wday < 0) wday += 7;

    qc_cycles = (int)(days / DAYS_PER_400Y);
    remdays = (int)(days % DAYS_PER_400Y);
    if (remdays < 0) {
        remdays += DAYS_PER_400Y;
        qc_cycles--;
    }

    c_cycles = remdays / DAYS_PER_100Y;
    if (c_cycles == 4) c_cycles--;
    remdays -= c_cycles * DAYS_PER_100Y;

    q_cycles = remdays / DAYS_PER_4Y;
    if (q_cycles == 25) q_cycles--;
    remdays -= q_cycles * DAYS_PER_4Y;

    remyears = remdays / 365;
    if (remyears == 4) remyears--;
    remdays -= remyears * 365;

    leap = !remyears && (q_cycles || !c_cycles);
    yday = remdays + 31 + 28 + leap;
    if (yday >= 365+leap) yday -= 365+leap;

    years = remyears + 4*q_cycles + 100*c_cycles + 400LL*qc_cycles;

    for (months=0; days_in_month[months] <= remdays; months++)
        remdays -= days_in_month[months];

    if (years+100 > INT_MAX || years+100 < INT_MIN)
        return -1;

    tm->tm_year = (int)(years + 100);
    tm->tm_mon = months + 2;
    if (tm->tm_mon >= 12) {
        tm->tm_mon -=12;
        tm->tm_year++;
    }
    tm->tm_mday = remdays + 1;
    tm->tm_wday = wday;
    tm->tm_yday = yday;

    tm->tm_hour = remsecs / 3600;
    tm->tm_min = remsecs / 60 % 60;
    tm->tm_sec = remsecs % 60;

    return 0;
}

/*********************************** amalgamated original file "/home/travis/build/open62541/open62541/deps/pcg_basic.c" ***********************************/

/*
 * PCG Random Number Generation for C.
 *
 * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *       http://www.pcg-random.org
 */

/*
 * This code is derived from the full C implementation, which is in turn
 * derived from the canonical C++ PCG implementation. The C++ version
 * has many additional features and is preferable if you can use C++ in
 * your project.
 */


// state for global RNGs

static pcg32_random_t pcg32_global = PCG32_INITIALIZER;

// pcg32_srandom(initial_state, initseq)
// pcg32_srandom_r(rng, initial_state, initseq):
//     Seed the rng.  Specified in two parts, state initializer and a
//     sequence selection constant (a.k.a. stream id)

void pcg32_srandom_r(pcg32_random_t* rng, uint64_t initial_state, uint64_t initseq)
{
    rng->state = 0U;
    rng->inc = (initseq << 1u) | 1u;
    pcg32_random_r(rng);
    rng->state += initial_state;
    pcg32_random_r(rng);
}

void pcg32_srandom(uint64_t seed, uint64_t seq)
{
    pcg32_srandom_r(&pcg32_global, seed, seq);
}

// pcg32_random()
// pcg32_random_r(rng)
//     Generate a uniformly distributed 32-bit random number

uint32_t pcg32_random_r(pcg32_random_t* rng)
{
    uint64_t oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
    uint32_t rot = (uint32_t)(oldstate >> 59u);
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

uint32_t pcg32_random()
{
    return pcg32_random_r(&pcg32_global);
}


// pcg32_boundedrand(bound):
// pcg32_boundedrand_r(rng, bound):
//     Generate a uniformly distributed number, r, where 0 <= r < bound

uint32_t pcg32_boundedrand_r(pcg32_random_t* rng, uint32_t bound)
{
    // To avoid bias, we need to make the range of the RNG a multiple of
    // bound, which we do by dropping output less than a threshold.
    // A naive scheme to calculate the threshold would be to do
    //
    //     uint32_t threshold = 0x100000000ull % bound;
    //
    // but 64-bit div/mod is slower than 32-bit div/mod (especially on
    // 32-bit platforms).  In essence, we do
    //
    //     uint32_t threshold = (0x100000000ull-bound) % bound;
    //
    // because this version will calculate the same modulus, but the LHS
    // value is less than 2^32.

    uint32_t threshold = -bound % bound;

    // Uniformity guarantees that this loop will terminate.  In practice, it
    // should usually terminate quickly; on average (assuming all bounds are
    // equally likely), 82.25% of the time, we can expect it to require just
    // one iteration.  In the worst case, someone passes a bound of 2^31 + 1
    // (i.e., 2147483649), which invalidates almost 50% of the range.  In 
    // practice, bounds are typically small and only a tiny amount of the range
    // is eliminated.
    for (;;) {
        uint32_t r = pcg32_random_r(rng);
        if (r >= threshold)
            return r % bound;
    }
}


uint32_t pcg32_boundedrand(uint32_t bound)
{
    return pcg32_boundedrand_r(&pcg32_global, bound);
}

