#ifndef LIST_H
#define LIST_H

#define List_PushFront(list, item) \
	item->prev = NULL; \
	item->next = (list)->first; \
	if ( (list)->first ) \
		(list)->first->prev = item; \
	(list)->first = item; \
	if ( !(list)->last ) \
		(list)->last = item; \

#define List_PushBack(list, item) \
	item->next = NULL; \
	item->prev = (list)->last; \
	if ( (list)->last ) \
		(list)->last->next = item; \
	(list)->last = item; \
	if ( !(list)->first ) \
		(list)->first = item; \

#define List_Remove(list, itemat) \
	if ( itemat->prev ) \
		itemat->prev->next = itemat->next; \
	if ( itemat->next ) \
		itemat->next->prev = itemat->prev; \
	if ( (list)->first == itemat ) \
		(list)->first = itemat->next; \
	if ( (list)->last == itemat ) \
		(list)->last = itemat->prev; \

#define List_InsertBefore(list, itemat, item) \
	item->prev = itemat->prev; \
	item->next = itemat; \
	if ( itemat->prev ) \
		itemat->prev->next = item; \
	itemat->prev = item; \
	if ( (list)->first == itemat ) \
		(list)->first = item; \

#define List_InsertAfter(list, itemat, item) \
	item->prev = itemat; \
	item->next = itemat->next; \
	if ( itemat->next ) \
		itemat->next->prev = item; \
	itemat->next = item; \
	if ( (list)->last == itemat ) \
		(list)->last = item; \

#endif
