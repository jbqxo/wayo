/* Copyright (c) 2019 Maxim Lyapin 
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy 
 *  of this software and associated documentation files (the "Software"), to deal 
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
 *  SOFTWARE.
 */
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "free_list.h"
#include "util.h"

struct header {
	// The size of the whole memory block including header.
	size_t block_size;
};

struct node {
	size_t block_size;

        /* Red-black tree's fields.                                
         * The tree is used for a fast lookup of best-fit blocks. */
                                                                   
        uintptr_t parent_addr : (sizeof(uintptr_t) * CHAR_BIT - 1);
        unsigned color : 1;                                        
        struct node *left;                                         
        struct node *right;                                        
};

#define MARK_RED(node_)                                                        \
	do {                                                                   \
		(node_)->color = 1;                                            \
	} while (false)

#define MARK_BLACK(node_)                                                      \
	do {                                                                   \
		(node_)->color = 0;                                            \
	} while (false)

#define IS_RED(node_) ((node_)->color)
#define IS_BLACK(node_) (!(IS_RED(node_)))

#define GET_PARENT_ADDR(node_) ((struct node *)((node_)->parent_addr << 1))
#define SET_PARENT_ADDR(node_, value)                                          \
	do {                                                                   \
		(node_)->parent_addr = (uintptr_t)value >> 1;                  \
	} while (false)
#define COPY_COLOR(src_, dest_)                                                \
	do {                                                                   \
		(src_)->color = (dest_)->color;                                \
	} while (false)

static void rotate_left(struct free_list *fl, struct node *n)
{
	assert(n);
	assert(n->right);

	struct node *newroot = n->right;
	struct node *parent = GET_PARENT_ADDR(n);
	SET_PARENT_ADDR(newroot, parent);
	if (!parent) {
		fl->root_node = newroot;
	} else if (parent->left == n) {
		parent->left = newroot;
	} else {
		parent->right = newroot;
	} 
	n->right = newroot->left;
	if (newroot->left) {
		SET_PARENT_ADDR(newroot->left, n);
	}

	newroot->left = n;
	SET_PARENT_ADDR(n, newroot);
}

static void rotate_right(struct free_list *fl, struct node *n)
{
	assert(n);
	assert(n->left);

	struct node *newroot = n->left;
	struct node *parent = GET_PARENT_ADDR(n);
	SET_PARENT_ADDR(newroot, parent);
	if (parent->left == n) {
		parent->left = newroot;
	} else if (parent->right == n) {
		parent->right = newroot;
	} else {
		fl->root_node = newroot;
	}

	n->left = newroot->right;
	if (newroot->right) {
		SET_PARENT_ADDR(newroot->right, n);
	}

	newroot->right = n;
	SET_PARENT_ADDR(n, newroot);
}

static void insert_node_fixup(struct free_list *fl, struct node *n)
{
	assert(fl);
	assert(n);

	struct node *parent = GET_PARENT_ADDR(n);
	struct node *grandp;
	if (!parent) {
		// If the tree doesn't contain any nodes, we want to skip the loop.
		goto after_loop;
	}
	grandp = GET_PARENT_ADDR(parent);

	while (IS_RED(parent)) {
		if (grandp->left == parent) {
			struct node *uncle = grandp->right;
			if (uncle && IS_RED(uncle)) {
				MARK_BLACK(parent);
				MARK_BLACK(uncle);
				MARK_RED(grandp);
				n = grandp;
				parent = GET_PARENT_ADDR(n);
				grandp = GET_PARENT_ADDR(parent);
			} else {
				if (n == parent->right) {
					// Left-Right case
					n = parent;
					parent = grandp;
					grandp = GET_PARENT_ADDR(grandp);
					rotate_left(fl, n);
				}
				// Left-left case; Split 4-node at grandp.
				MARK_BLACK(parent);
				// Uncle is already black. See the condition above.
				MARK_RED(grandp);
				rotate_right(fl, grandp);
			}
		} else {
			// Else-arm is the same, just mirrored.
			struct node *uncle = grandp->left;
			if (uncle && IS_RED(uncle)) {
				MARK_BLACK(parent);
				MARK_BLACK(uncle);
				MARK_RED(grandp);
				n = grandp;
				parent = GET_PARENT_ADDR(n);
				grandp = GET_PARENT_ADDR(parent);
			} else {
				if (n == parent->left) {
					// Right-Left case
					n = parent;
					parent = grandp;
					grandp = GET_PARENT_ADDR(grandp);
					rotate_right(fl, n);
				}
				// Right-right case; Split 4-node at grandp.
				MARK_BLACK(parent);
				// Uncle is already black. See the condition above.
				MARK_RED(grandp);
				rotate_left(fl, grandp);
			}
		}
	}
after_loop:
	MARK_BLACK((struct node *)fl->root_node);
}

static void insert_node(struct free_list *fl, void *block, size_t size)
{
	assert(fl);
	assert(block);
	assert(size >= 0);

	struct node *newnode = (struct node *)block;
	memset(newnode, 0, size);
	newnode->block_size = size;
	MARK_RED(newnode);

	struct node *parent = NULL;
	struct node *it = (struct node *)fl->root_node;
	while (it) {
		parent = it;
		if (size < it->block_size) {
			it = it->left;
		} else {
			it = it->right;
		}
	}
	SET_PARENT_ADDR(newnode, parent);

	if (!parent) {
		fl->root_node = newnode;
	} else if (size < parent->block_size) {
		parent->left = newnode;
	} else {
		parent->right = newnode;
	}

	insert_node_fixup(fl, newnode);
}

static struct node *find_node(struct node *n, size_t size)
{
	if (!n) {
		return NULL;
	}

	if (size == n->block_size) {
		return n;
	} else if (size > n->block_size) {
		return find_node(n->right, size);
	} else {
		struct node *match = find_node(n->left, size);
		if (!match) {
			// If there is no suitable node in the left sub-tree, return the current node.
			match = n;
		}
		return match;
	}
}

static struct node *subtree_min(struct node *n)
{
	assert(n);
	while (n->left) {
		n = n->left;
	}
	return n;
}

static void transplant_node(struct free_list *fl, struct node *to,
			    struct node *from)
{
	assert(fl);
	assert(to);

	struct node *parent = GET_PARENT_ADDR(to);
	if (!parent) {
		fl->root_node = from;
	} else if (parent->right == to) {
		parent->right = from;
	} else {
		parent->left = from;
	}

	if (from) {
		SET_PARENT_ADDR(from, parent);
	}
}

static void delete_node_fixup(struct free_list *fl, struct node *successor)
{
	assert(fl);
	assert(successor);

	struct node *root = fl->root_node;
	struct node *parent = GET_PARENT_ADDR(successor);
	while (IS_BLACK(successor) && successor != root) {
		if (successor == parent->left) {
			struct node *sibling = parent->right;
			if (IS_RED(sibling)) {
				MARK_BLACK(sibling);
				MARK_RED(parent);
				rotate_left(fl, parent);
				sibling = parent->right;
			}
			if (IS_BLACK(sibling->left) &&
			    IS_BLACK(sibling->right)) {
				MARK_RED(sibling);
				successor = parent;
				parent = GET_PARENT_ADDR(successor);
			} else {
				if (IS_BLACK(sibling->right)) {
					MARK_BLACK(sibling->left);
					MARK_RED(sibling);
					rotate_right(fl, sibling);
					sibling = parent->right;
				}
				COPY_COLOR(parent, sibling);
				MARK_BLACK(parent);
				MARK_BLACK(sibling->right);
				rotate_left(fl, parent);
				// At this point, we've fixed violations and can terminate the loop.
				break;
			}
		} else {
			// Else-arm is the same, just mirrored.
			struct node *sibling = parent->left;
			if (IS_RED(sibling)) {
				MARK_BLACK(sibling);
				MARK_RED(parent);
				rotate_right(fl, parent);
				sibling = parent->left;
			}
			if (IS_BLACK(sibling->left) &&
			    IS_BLACK(sibling->right)) {
				MARK_RED(sibling);
				successor = parent;
				parent = GET_PARENT_ADDR(successor);
			} else {
				if (IS_BLACK(sibling->left)) {
					MARK_BLACK(sibling->right);
					MARK_RED(sibling);
					rotate_left(fl, sibling);
					sibling = parent->left;
				}
				COPY_COLOR(parent, sibling);
				MARK_BLACK(parent);
				MARK_BLACK(sibling->left);
				rotate_left(fl, parent);
				// At this point, we've fixed violations and can terminate the loop.
				break;
			}
		}
	}
	MARK_BLACK(successor);
}

static void delete_node(struct free_list *fl, struct node *n)
{
	assert(fl);
	assert(n);

	struct node *successor = NULL;

	// If the node which will be eventually removed or moved within the tree is black,
	// it's removal could cause (certainly will) violations of the RBT properties.
	bool mv_node_red = IS_RED(n);

	if (!n->left) {
		successor = n->right;
		transplant_node(fl, n, successor);
	} else if (!n->right) {
		successor = n->left;
		transplant_node(fl, n, successor);
	} else {
		struct node *mv_node = subtree_min(n->right);
		mv_node_red = IS_RED(mv_node);
		successor = mv_node->right;

		if (GET_PARENT_ADDR(mv_node) != n) {
			transplant_node(fl, mv_node, mv_node->right);
			mv_node->right = n->right;
			SET_PARENT_ADDR(mv_node->right, mv_node);
		}
		transplant_node(fl, n, mv_node);
		mv_node->left = n->left;
		SET_PARENT_ADDR(mv_node->left, mv_node);
		COPY_COLOR(n, mv_node);
	}

	if (successor && !mv_node_red) {
		delete_node_fixup(fl, successor);
	}
}

void free_list_init(struct free_list *fl, void *mem, size_t size)
{
	assert(fl);
	assert(mem);
	assert(size > sizeof(struct node));

	fl->root_node = NULL;
	fl->mem_block = mem;

	insert_node(fl, mem, size);

	return;
}

int_rc free_list_alloc(struct free_list *fl, size_t s, void **result)
{
	assert(fl);
	assert(s > 0);

	size_t req_size = s + sizeof(struct header);
	if (req_size < sizeof(struct node)) {
		req_size = sizeof(struct node);
	}

	struct node *n = find_node(fl->root_node, req_size);

	if (!n) {
		*result = NULL;
		return -ENOSPC;
	}

	delete_node(fl, n);

	// If the node is too large, and the remained space is large enough to hold a new node, insert it.
	if (req_size < n->block_size &&
	    (n->block_size - req_size) >= sizeof(struct node)) {
		size_t remained = n->block_size - req_size;
		insert_node(fl, (void *)n + req_size, remained);
	}

	((struct header *)n)->block_size = req_size;

	// Zero all the memory in the block except for the header.
	memset((void *)n + sizeof(struct header), 0,
	       n->block_size - sizeof(struct header));
	*result = (void *)n + sizeof(struct header);

	return RC_OK;
}

void free_list_free(struct free_list *fl, void *block)
{
	assert(fl);
	if (!block) {
		return;
	}
	// The allocator stores headers before each allocated block.
	struct header *h = block - sizeof(struct header);
	insert_node(fl, h, h->block_size);
}
