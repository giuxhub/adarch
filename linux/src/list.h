#ifndef __INC_LIST_H_
#define __INC_LIST_H_

#include <stddef.h>

typedef struct list_t
{
	struct list_t *next, *prev;
} list_t;

#define list_init(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

static inline void __list_add(list_t *_new, list_t *prev, list_t *next)
{
	next->prev = _new;
	_new->next = next;
	_new->prev = prev;
	prev->next = _new;
}

static inline void list_add(list_t *_new, list_t *head)
{
	__list_add(_new, head, head->next);
}

static inline void list_add_tail(list_t *_new, list_t *head)
{
	__list_add(_new, head->prev, head);
}

static inline void __list_del(list_t *prev, list_t *next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(list_t *entry)
{
	__list_del(entry->prev, entry->next);
	entry->next = (list_t *) 0;
	entry->prev = (list_t *) 0;
}

static inline void list_del_init(list_t *entry)
{
	__list_del(entry->prev, entry->next);
	list_init(entry);
}

static inline void list_move(list_t *list, list_t *head)
{
	__list_del(list->prev, list->next);
	list_add(list, head);
}

static inline void list_move_tail(list_t *list, list_t *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}

static inline int list_empty(list_t *head)
{
	return head->next == head;
}

static inline void __list_splice(list_t *list, list_t *head)
{
	list_t *first = list->next;
	list_t *last = list->prev;
	list_t *at = head->next;

	first->prev = head;
	head->next = first;

	last->next = at;
	at->prev = last;
}

static inline void list_splice(list_t *list, list_t *head)
{
	if (!list_empty(list))
		__list_splice(list, head);
}

static inline void list_splice_init(list_t *list, list_t *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head);
		list_init(list);
	}
}
/**
 * See ftp://rtfm.mit.edu/pub/usenet-by-group/news.answers/C-faq/faq
 * @param ptr: the &struct list_head pointer
 * @param type: the type of the struct this is embedded in
 * @param member: the name of the list_struct within the struct
 */
#define list_entry(ptr, type, member) \
	((type *)((char *)(ptr)-(size_t)(&((type *)0)->member)))

/**
 * @param pos: the &struct list_head to use as a loop counter
 * @param n: another &struct list_head to use as temporary storage
 * @param head: the head for your list
 */
#define list_for_each(pos, n, head) \
for ((pos) = (head)->next, (n) = (pos)->next; (pos) != (head); \
	(pos) = (n), (n) = (pos)->next)

/**
 * @param pos: the type * to use as a loop counter
 * @param n: another type * to use as temporary storage
 * @param head: the head for your list
 * @param member: the name of the list_struct within the struct
 */
#define list_for_each_entry(pos, n, head, member) \
for (pos = list_entry((head)->next, typeof(*pos), member), \
	n = list_entry(pos->member.next, typeof(*pos), member); \
	&(pos->member) != (head); \
	pos = n, n = list_entry(n->member.next, typeof(*n), member))

/**
 * @param pos: the type * to use as a loop counter
 * @param n: another type * to use as temporary storage
 * @param head: the type * head that contains your list
 * @param member: the name of the list_struct within the struct
 * @param func: the name of the function to free each entry
 */
#define list_destroy(pos, n, head, member, func) \
do { list_for_each_entry(pos, n, &(head->list), member) { \
	list_del(&(curr->list)); func(pos); } free(head); } while (0)

#endif
