#pragma once

#include "nvim/api.h"
#include "allocators/allocators.h"

typedef void (*ev_handler_fn)(struct msg *ctx);

struct nvim_ev_map_node {
	char *key;
	ev_handler_fn handler;
	struct nvim_ev_map_node *next;
};

struct nvim_ev_map {
	struct nvim_ev_map_node *head;
	size_t count;
};

typedef struct ev_manager {
	struct nvim_ev_map map;
	struct mem_stack *alloc;
	struct mem_pool *evctx_arena_pool;
	size_t arena_sz;
} evmgr_t;

void evmgr_init(evmgr_t *evm, struct mem_stack *alloc,
		struct mem_pool *evctx_arena_pool,
		size_t arena_sz);
void evmgr_handle_notification_msg(evmgr_t *evm, mpack_node_t root);
void evmgr_error_event(evmgr_t *evm, ev_handler_fn handler);
void evmgr_subscribe(evmgr_t *evm, char *event, ev_handler_fn handler);

