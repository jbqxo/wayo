#include <unistd.h>

#include <uv.h>
#include <mpack.h>

#include "caller.h"
#include "api.h"
#include "util.h"

static msgid_t alloc_msgid(struct caller *c) {
	if (c->msgid == MSGID_MAX) {
		c->msgid = 0;
	}
	return c->msgid++;
}


void caller_init(struct caller *c, uv_loop_t *loop, struct mem_stack *global) {
	c->msgid = 0;
	c->loop = loop;

	int err = 0;
	uv_tty_t *tty_out = mem_stack_alloc(global, sizeof(*tty_out));
	err = uv_tty_init(loop, tty_out, STDOUT_FILENO, /* unused */ 0);
	assert(!err);

	c->out = (uv_stream_t*)tty_out;
}

// Global functions
void nvim_command(
		struct caller *caller,
		struct msg *ctx,
		const char *cmd,
		cmd_cb cb
		)
{
	uv_buf_t *request_buffer = xmalloc(sizeof(*request_buffer));

	mpack_writer_t writer;
	mpack_writer_init_growable(&writer, &request_buffer->base,
				   &request_buffer->len);

	mpack_start_array(&writer, 4);
	mpack_write_int(&writer, NVIM_RPC_REQUEST);
	mpack_write_u32(&writer, alloc_msgid(caller));
	mpack_write_cstr(&writer, "nvim_command");
	mpack_write_cstr(&writer, cmd);
	mpack_finish_array(&writer);

	if (mpack_writer_destroy(&writer) != mpack_ok) {
		fprintf(stderr, "An error occurred encoding the data\n");
		return;
	}

	uv_write_t wreq;
	wreq.data = request_buffer;
	/* uv_write(&wreq, api->out, request_buffer, 1, free_write_buffer); */
}
