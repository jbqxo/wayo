#include "nvim/eventmgr.h"

static struct nvim_ev_map_node *alloc_new_node(evmgr_t *evm)
{
	return mem_stack_alloc(evm->alloc, sizeof(struct nvim_ev_map_node));
}

static struct nvim_ev_map_node *map_search(evmgr_t *evm, const char *key)
{
	struct nvim_ev_map_node *node = evm->map.head;

	while (node) {
		char *nkey = node->key;
		if (!strcmp(nkey, key)) {
			return node;
		}
		node = node->next;
	}

	return NULL;
}

static void map_add(evmgr_t *evm, char *key, ev_handler_fn handler)
{
	struct nvim_ev_map_node *new_node = alloc_new_node(evm);
	assert(new_node);

	new_node->next = evm->map.head;
	new_node->key = key;
	new_node->handler = handler;

	evm->map.head = new_node;
}

void evmgr_subscribe(evmgr_t *evm, char *event, ev_handler_fn handler)
{
	map_add(evm, event, handler);
}

static struct event_context *alloc_new_ctx(evmgr_t *evm) {
	struct mem_stack *arena = mem_pool_alloc(evm->evctx_arena_pool);
	assert(arena);
	mem_stack_init(arena,
			(void*)((uintptr_t)arena + sizeof(struct mem_stack)),
			evm->arena_sz - sizeof(struct mem_stack));

	struct event_context *ctx = mem_stack_alloc(arena, sizeof(*ctx));
	assert(ctx);

	ctx->arena = arena;
	return ctx;
}

void evmgr_handle_notification_msg(evmgr_t *evm, mpack_node_t root)
{
	const char *method = msg->notif.method;
	struct nvim_ev_map_node *node = map_search(evm, method);
	if (!node || !node->handler) {
		return;
	}

	struct event_context *ctx = alloc_new_ctx(evm);
}

void evmgr_init(evmgr_t *evm, struct mem_stack *alloc,
		struct mem_pool *evctx_arena_pool,
		size_t arena_sz)
{
	evm->map.head = NULL;
	evm->map.count = 0;
	evm->alloc = alloc;
	evm->evctx_arena_pool = evctx_arena_pool;
	evm->arena_sz = arena_sz;
}
