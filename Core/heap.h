#ifndef HEAP_H
#define HEAP_H

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

#define HEAP_PARENT(x)	(x >> 1)
#define HEAP_LEFT(x)	(x << 1)
#define HEAP_RIGHT(x)	((x << 1) + 1)

typedef bool (*heap_compare_fn)(void* pv0, void* pv1);

typedef struct heap_s
{
	int				m_iv;			// The next free v
	void**			m_avs;			// The array of types
	heap_compare_fn	m_heap_compare;
}
heap_t;

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

bool Heap_IsEmpty(heap_t* heap);
int Heap_GetSize(heap_t* heap);

void Heap_Push(heap_t* heap, void* pv);
void* Heap_Top(heap_t* heap);
void* Heap_Pop(heap_t* heap);

void Heap_Init(heap_t* heap, int size, heap_compare_fn heap_compare);
void Heap_Term(heap_t* heap);

void Heap_Update(heap_t* heap, void* pv);
void Heap_Heapify(heap_t* heap, int iv);

#endif
