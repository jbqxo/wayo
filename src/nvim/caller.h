#pragma once

#include <mpack.h>

#include "api.h"
#include "listener.h"

#define msgid_t uint32_t
#define MSGID_MAX UINT32_MAX

struct caller {
	msgid_t msgid;
	uv_loop_t *loop;
	uv_stream_t *out;
};

typedef void (*cmd_cb)(struct msg *ctx);

void caller_init(struct caller *c, uv_loop_t *loop, struct mem_stack *global);
void caller_handle(struct caller *c, struct msg *ctx);

// Global functions
void nvim_command(
		struct caller *caller,
		struct msg *ctx,
		const char *cmd,
		cmd_cb cb);
