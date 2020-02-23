#pragma once

#include <stdint.h>

#include <mpack.h>

#include "allocators/allocators.h"

// Request typically contains: type, msgid, method, params
// Response contains: type, msgid, error, result
// Notification contains: type, method, params
#define RPC_MAX_NODE_COUNT 4

// Protocol Specs: https://github.com/msgpack-rpc/msgpack-rpc/blob/e6a28c4b71638b61ea11469917b030df45ef8081/spec.md
enum msg_type {
	NVIM_RPC_UNKNOWN = -0x1,
	NVIM_RPC_REQUEST = 0x0,
	NVIM_RPC_RESPONSE = 0x1,
	NVIM_RPC_NOTIFICATION = 0x2,
};

struct msg_request {
	uint32_t msgid;
	char *method;
	mpack_node_t params;
};

struct msg_response {
	uint32_t msgid;
	mpack_node_t error;
	mpack_node_t result;
};

struct msg_notification {
	char *method;
	mpack_node_t params;
};

struct event_context {
	struct msg_notification initial_msg;
	struct mem_stack *arena;
};

bool api_parse_to_resp(mpack_node_t root, struct msg_response *resp);
bool api_parse_to_notif(mpack_node_t root, struct msg_notification *notif,
			char *method_buffer, size_t method_buff_sz);
size_t api_notif_method_len(mpack_node_t root);
enum msg_type api_msg_get_type(mpack_node_t msg_root);
