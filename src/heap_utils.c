#include "codexion.h"

int	heap_init(t_heap *heap, int capacity)
{
	heap->size = 0;
	heap->capacity = 0;
	heap->data = malloc(sizeof(t_request) * capacity);
	if (!heap->data)
		return (1);
	memset(heap->data, 0, sizeof(t_request) * capacity);
    
	heap->capacity = capacity;
	return (0);
}


void	heap_destroy(t_heap *heap)
{
	free(heap->data);
	heap->data = NULL;
	heap->size = 0;
	heap->capacity = 0;
}

int	heap_before(t_request *a, t_request *b)
{
	if (a->key != b->key)
		return (a->key < b->key);
	return (a->coder_id < b->coder_id);
}

void	heap_swap(t_request *a, t_request *b)
{
	t_request	temp;

	temp = *a;
	*a = *b;
	*b = temp;
}

int	heap_peek(t_heap *heap, t_request *out)
{
	if (heap->size == 0)
		return (1);
	*out = heap->data[0];
	return (0);
}
