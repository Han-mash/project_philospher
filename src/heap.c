#include "codexion.h"

static void	shift_up(t_heap *heap, int index)
{
	int	parent;

	parent = (index - 1) / 2;
	while (index > 0 && heap_before(&heap->data[index], &heap->data[parent]))
	{
		heap_swap(&heap->data[index], &heap->data[parent]);
		index = parent;
		parent = (index - 1) / 2;
	}
}

static int	smallest_of(t_heap *heap, int index)
{
	int	left;
	int	right;
	int	smallest;

	left = 2 * index + 1;
	right = 2 * index + 2;
	smallest = index;
	if (left < heap->size
		&& heap_before(&heap->data[left], &heap->data[smallest]))
		smallest = left;
	if (right < heap->size
		&& heap_before(&heap->data[right], &heap->data[smallest]))
		smallest = right;
	return (smallest);
}

static void	shift_down(t_heap *heap, int index)
{
	int	smallest;

	smallest = smallest_of(heap, index);
	while (smallest != index)
	{
		heap_swap(&heap->data[index], &heap->data[smallest]);
		index = smallest;
		smallest = smallest_of(heap, index);
	}
}

int	heap_push(t_heap *heap, t_request *request)
{
	if (heap->size >= heap->capacity)
		return (1);
	heap->data[heap->size] = *request;
	heap->size++;
	shift_up(heap, heap->size - 1);
	return (0);
}

int	heap_pop(t_heap *heap)
{
	if (heap->size == 0)
		return (1);
	heap->size--;
	heap->data[0] = heap->data[heap->size];
	shift_down(heap, 0);
	return (0);
}