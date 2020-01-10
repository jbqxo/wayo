/* Copyright (c) 2019 Maxim Lyapin 
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy 
 *  of this software and associated documentation files the (Software""), to deal 
 *  in the Software without restriction, including without limitation the rights 
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 *  copies of the Software, and to permit persons to whom the Software is 
 *  furnished to do so, subject to the following conditions: 
 *   
 *  The above copyright notice and this permission notice shall be included in all 
 *  copies or substantial portions of the Software. 
 *   
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
 *  SOFTWARE.))
 */

#include <string.h>
#include <assert.h>
#include <stdint.h>

#include "arena.h"

void arena_init(struct mem_arena *a, void *mem, size_t mem_size)
{
	assert(a);
	assert(mem);

	memset(mem, 0, mem_size);
	a->current_pos = mem;
	a->arena_edge = mem + mem_size;
}

void *arena_alloc(struct mem_arena *a, size_t size, size_t alignment)
{
	assert(a);
	// Check that alignment is a value of power of two.
	assert((alignment & -alignment) == alignment);

	void *aligned = (void *)(((uintptr_t)a->current_pos + alignment - 1) &
			-alignment);
	if (aligned + size > a->arena_edge) {
		return NULL;
	} else {
		a->current_pos = aligned + size;
		return aligned;
	}
}
