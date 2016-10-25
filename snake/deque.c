/*
 * deque.c: Implementation of deque in C
 *
 * St: 2016-09-28 Wed 11:27 PM
 * Up: 2016-09-29 Thu 01:24 AM
 *
 * Author: SPS
 *
 * This file is copyright 2016 SPS.
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL SPS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include<stdio.h>
#include<stdlib.h>
#include<assert.h>

#include"mylib.h"

#define SKIP


/*
 * Check if deque is empty.
 *
 * @dek: Pointer to the deque structure
 */
int deque_is_empty(struct deque *dek)
{
	int retval;

	if (dek->head == NULL){
	
		retval = 1;
	}
	else
		retval = 0;

	return retval;
}

/*
 * Create a deque
 *
 * @cpy: Copy function
 * @cmp: Compare function
 * @dval: Funciton to destroy `val' of a node
 */
struct deque *deque_create(void *(*cpy)(void *), int (*cmp)(void *, void *),
		           void (*dval)(void *),
			   void (*printl)(struct deque_node *))
{
	struct deque *dek;

	dek = malloc(sizeof(struct deque));
	assert(dek);

	dek->head = NULL;
	dek->tail = NULL;
	dek->cpy = cpy;
	dek->cmp = cmp;
	dek->dval = dval;
	dek->printl = printl;
	dek->nmemb = 0;

	return dek;
}

/*
 * Create a dek_node
 *
 * @val: value of the node
 */
static struct deque_node *deque_node_create(struct deque *dek, void *val)
{
	struct deque_node *dekn;

	dekn = malloc(sizeof(struct deque_node));
	assert(dekn);

	dekn->val = dek->cpy(val);

	dekn->next = NULL;
	dekn->prev = NULL;

	return dekn;
}

/*
 * Insert a new value to deque
 *
 * @l: deque
 * @val: new value to be inserted to the deque
 */
int deque_insert(struct deque *dek, int dir, void *val)
{
	int retval;
	struct deque_node *dekn;

	/* Assume insert will succeed */
	retval = 1;

	dekn = deque_node_create(dek, val);
	if (dir == DEQUE_HEAD) {
		dekn->next = dek->head;
		if (dek->head != NULL)
			dek->head->prev = dekn;
		else
			dek->tail = dekn;
		dek->head = dekn;
	} else if (dir == DEQUE_TAIL) {
		dekn->prev = dek->tail;
		if (dek->tail != NULL)
			dek->tail->next = dekn;
		else
			dek->head = dekn;
		dek->tail = dekn;
	} else {
		/* Indicate insert failed */
		retval = 0;
	}

	dek->nmemb++;

	return retval;
}

/*
 * Pop a node from deque.
 *
 * A node can be popped from one of the two directions. Node can be
 * popped from either head side, or tail side. 
 *
 * @dek: Pointer to the deque structure
 * @dir: Direction for pop (HEAD or TAIL)
 */
void *deque_pop(struct deque *dek, int dir)
{
	struct deque_node *del_node;
	void *retval;

	/* Return if deque is empty */
	if (deque_is_empty(dek))
		return NULL;

	del_node = NULL;

	if (dir == DEQUE_HEAD) {
		del_node = dek->head;
		retval = dek->cpy(dek->head->val);
		if (dek->head->next != NULL)
			dek->head->next->prev = NULL;
		else
			dek->tail = NULL;
		dek->head = dek->head->next;
	} else if (dir == DEQUE_TAIL) {
		del_node = dek->tail;
		retval = dek->cpy(dek->tail->val);
		if (dek->tail->prev != NULL)
			dek->tail->prev->next = NULL;
		else
			dek->head = NULL;
		dek->tail = dek->tail->prev;
	} else {
		/* Return NULL to indicate pop failed */
		retval = NULL;
	}

	if (del_node != NULL) {
		dek->dval(del_node->val);
		free(del_node);
		dek->nmemb--;
	}

	return retval;
}

/*
 * Search for a val in deque
 *
 * @l: Pointer to deque
 * @val: Pointer to value to search
 */
int deque_search(struct deque *dek, void *val)
{
	int retval;
	struct deque_node *cur;

	retval = 0;

	cur = dek->head;
	while(cur != NULL) {
		if (dek->cmp(cur->val, val) == 0) {
			retval = 1;
			break;
		}
		cur = cur->next;
	}

	return retval;
}


/*
 * Delete a node from deque. Has no effect if
 * there is no node whose value is `val'.
 *
 * @l: Pointer to deque
 * @val: Value of the node to be deleted
 */
void deque_delete(struct deque *dek, void *val)
{
	struct deque_node *cur;
	struct deque_node **prev;

	/* Do nothing if nothing to delete */
	if (val == NULL)
		return;
	
	prev = &dek->head;
	while (*prev != NULL) {
		cur = *prev;
		if (dek->cmp(cur->val, val) == 0) {
			*prev = cur->next;
			if (cur->next != NULL)
				cur->next->prev = cur->prev;
			else
				dek->tail = cur->prev;
			dek->dval(cur->val);
			free(cur);
			dek->nmemb--;
			break;
		}
		prev = &cur->next;
	}
}

/*
 * Destroy deque
 *
 * @l: Pointer to deque
 */
void deque_destroy(struct deque *dek)
{
	struct deque_node *dekn;
	struct deque_node *next;

	dekn = dek->head;
	while (dekn != NULL) {
		next = dekn->next;
		dek->dval(dekn->val);
		free(dekn);
		dekn = next;
	}

	/* TODO: Is this needed? */
	dek->head = NULL;
	dek->tail = NULL;

	free(dek);
}

/*
 * Print deque
 *
 * @l: Pointer to deque
 */
void deque_print(struct deque *dek)
{
	struct deque_node *cur;

	/* If no print function, just return  */
	if (dek->printl == NULL)
		return;
	
	/* Print each node */
	cur = dek->head;
	while (cur != NULL) {
		dek->printl(cur);
		cur = cur->next;
	}
	putchar('\n');
}

/*
 * Initialize iterator for the deque.
 * Returns the pointer to first item.
 *
 * @l: Pointer to the deque structure
 */
void* deque_begin(struct deque *d)
{
	void *first;

	first = d->head;

	return first;
}

/*
 * Get the next value from deque iteration.
 * ll_first should be called before this function is called.
 *
 * @itrp: Pointer to iterator
 */
void* deque_next(struct deque *d, void **itrp)
{
	void *retval;
	struct deque_node *dekn;

	dekn = (struct deque_node *) *itrp;

	/* Make a copy of current node val to return */
	retval = d->cpy(dekn->val);

	/* Update iterator */
	*itrp = dekn->next;

	return retval;
}

/*
 * Check if iteration is completed
 *
 * @itr: Iterator
 */
int deque_done(void *itr)
{
	int retval;

	if (itr == NULL)
		retval = 1;
	else
		retval = 0;

	return retval;
}
