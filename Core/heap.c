#include "savage_common.h"
#include "heap.h"
#include "mem.h"

bool Heap_IsEmpty(heap_t* heap)
{
	return heap->m_iv == 0;
}

int Heap_GetSize(heap_t* heap) 
{ 
	return heap->m_iv; 
}

void Heap_Init(heap_t* heap, int size, heap_compare_fn heap_compare)
{
	heap->m_iv = 0;
	heap->m_avs = Tag_Malloc(sizeof(void*)*size, MEM_HEAP);
	heap->m_heap_compare = heap_compare;
}

void Heap_Term(heap_t* heap)
{
	Tag_Free(heap->m_avs);
}

void Heap_Heapify(heap_t* heap, int iv)
{
	int iLeft = HEAP_LEFT(iv);
	int iRight = HEAP_RIGHT(iv);
	int iLargest;
	if ( iLeft <= heap->m_iv && heap->m_heap_compare(heap->m_avs[iLeft], heap->m_avs[iv]) )
	{
		iLargest = iLeft;
	}
	else
	{
		iLargest = iv;
	}
	if ( iRight <= heap->m_iv && heap->m_heap_compare(heap->m_avs[iRight], heap->m_avs[iLargest]) )
	{
		iLargest = iRight;
	}
	if ( iLargest != iv )
	{
		void* pTemp;
		pTemp = heap->m_avs[iLargest];
		heap->m_avs[iLargest] = heap->m_avs[iv];
		heap->m_avs[iv] = pTemp;
		Heap_Heapify(heap, iLargest);
	}
}

void Heap_Push(heap_t* heap, void* pv)
{
	int iv = ++heap->m_iv;
	while ( iv > 1 && heap->m_heap_compare(pv, heap->m_avs[HEAP_PARENT(iv)]) )
	{
		heap->m_avs[iv] = heap->m_avs[HEAP_PARENT(iv)];
		iv = HEAP_PARENT(iv);
	}
	heap->m_avs[iv] = pv;
}

void* Heap_Top(heap_t* heap)
{
	return heap->m_avs[1];
}

void* Heap_Pop(heap_t* heap)
{
	void* pv = heap->m_avs[1];
	heap->m_avs[1] = heap->m_avs[heap->m_iv];
	heap->m_iv--;
	Heap_Heapify(heap, 1);
	return pv;
}

void Heap_Update(heap_t* heap, void* pv)
{
	int iv;
	for ( iv = 1 ; iv < heap->m_iv ; iv++ )
	{
		if ( heap->m_avs[iv] == pv )
		{
			heap->m_avs[iv] = heap->m_avs[heap->m_iv];
			heap->m_iv--;
			Heap_Heapify(heap, iv);
		}
	}
	
	Heap_Push(heap, pv);
}
