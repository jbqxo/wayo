#include <uv.h>
#include <mpack/mpack.h>

#include "funcs.h"
#include "api.h"

#include "storage.h"

static void free_write_buffer(uv_write_t *req, int status) {
  if (status < 0) {
    // TODO(Maxim Lyapin): Log failed write
  }
  free(req->data);
}

// Global functions
void nvim_command(struct nvim_api *api, const char *restrict cmd) {
  // TODO(Maxim Lyapin): Replace with memory pool.
  uv_buf_t *request_buffer = malloc(sizeof(*request_buffer));

  mpack_writer_t writer;
  mpack_writer_init_growable(&writer, &request_buffer->base, &request_buffer->len);

  mpack_start_array(&writer, 4);
  mpack_write_int(&writer, NVIM_RPC_REQUEST);
  mpack_write_u32(&writer, nvim_next_msgid(api));
  mpack_write_cstr(&writer, "nvim_command");
  mpack_write_cstr(&writer, cmd);
  mpack_finish_array(&writer);

  if (mpack_writer_destroy(&writer) != mpack_ok) {
    fprintf(stderr, "An error occurred encoding the data\n");
    return;
  }

  uv_write_t wreq;
  wreq.data = request_buffer;
  uv_write(&wreq, api->out, request_buffer, 1, free_write_buffer);
  // TODO(Maxim Lyapin): Free request_buffer.base
}
